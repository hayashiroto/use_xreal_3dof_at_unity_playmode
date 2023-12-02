#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "cglm/cglm.h"
#include "hidapi/hidapi.h"

#define TICK_LEN (1.0f / 3906000.0f)
#define AIR_VID 0x3318
#define AIR_PID 0x0424

// カメラの回転を表すクォータニオン
static versor rotation = GLM_QUAT_IDENTITY_INIT;

// ミューテックス（スレッドセーフな変更のためのロック）
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// HIDデバイスから取得するデータの構造体
typedef struct {
    uint32_t tick;
    int16_t ang_vel[3];
} air_sample;

// HIDデバイスを開く関数
static hid_device* open_device() {
    struct hid_device_info* devs = hid_enumerate(AIR_VID, AIR_PID);
    struct hid_device_info* cur_dev = devs;
    hid_device* device = NULL;

    // インターフェース番号が3のデバイスを検索
    while (devs) {
        if (cur_dev->interface_number == 3) {
            device = hid_open_path(cur_dev->path);
            break;
        }
        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);
    return device;
}

// 受信データから角速度を取り出す関数
static void process_ang_vel(const int16_t ang_vel[3], vec3 out_vec) {
    // 角速度のスケールとバイアスの補正（経験的な補正）
    out_vec[0] = (float)(ang_vel[0] + 20) * -0.001f;
    out_vec[1] = (float)(ang_vel[2] - 20) * 0.001f + 0.012f;
    out_vec[2] = (float)(ang_vel[1] + 20) * 0.001f - 0.012f;
}

// HIDデバイスからのレポートを解析する関数
static int parse_report(const unsigned char* buffer, int size, air_sample* out_sample) {
    if (size != 64) {
        printf("Invalid packet size");
        return -1;
    }

    buffer += 5;
    out_sample->tick = *(buffer++) | (*(buffer++) << 8) | (*(buffer++) << 16) | (*(buffer++) << 24);
    buffer += 10;
    out_sample->ang_vel[0] = *(buffer++) | (*(buffer++) << 8);
    buffer++;
    out_sample->ang_vel[1] = *(buffer++) | (*(buffer++) << 8);
    buffer++;
    out_sample->ang_vel[2] = *(buffer++) | (*(buffer++) << 8);

    return 0;
}

// カメラの回転を更新する関数
static void update_rotation(float dt, vec3 ang_vel) {
    // ミューテックスのロック
    pthread_mutex_lock(&mutex);

    // 角速度の大きさが一定以上の場合のみ回転処理を行う
    float ang_vel_length = glm_vec3_norm(ang_vel);

    if (ang_vel_length > 0.0001f) {
        // 角速度ベクトルを正規化して回転軸として使用
        vec3 rot_axis = { ang_vel[0] / ang_vel_length, ang_vel[1] / ang_vel_length, ang_vel[2] / ang_vel_length };

        // 回転角度は角速度の大きさと経過時間に比例
        float rot_angle = ang_vel_length * dt;

        // 回転軸と回転角度から回転を表すクォータニオンを作成
        versor delta_rotation;
        glm_quatv(delta_rotation, rot_angle, rot_axis);

        // ここの出力をunity側がキャプチャする
        printf("%f %f %f %f\n", delta_rotation[0], delta_rotation[1], delta_rotation[2], delta_rotation[3]);

        // 既存の回転に新しい回転を合成
        glm_quat_mul(rotation, delta_rotation, rotation);
    }

    // クォータニオンの正規化（必要ならば）
    glm_quat_normalize(rotation);

    // ミューテックスのアンロック
    pthread_mutex_unlock(&mutex);
}

int main(void) {
    // HIDデバイスを開く
    hid_device* device = open_device();
    if (!device) {
        printf("Unable to open device\n");
        return 1;
    }

    // HIDデバイスに特定のデータを書き込む（マジックナンバー）
    uint8_t magic_payload[] = { 0xaa, 0xc5, 0xd1, 0x21, 0x42, 0x04, 0x00, 0x19, 0x01 };
    int res = hid_write(device, magic_payload, sizeof(magic_payload));
    if (res < 0) {
        printf("Unable to write to device\n");
        return 1;
    }

    do {
        unsigned char buffer[64] = {};
        uint32_t last_sample_tick = 0;
        air_sample sample = {};
        vec3 ang_vel = {};

        // HIDデバイスからデータを読み取る
        int res = hid_read(device, (void*)&buffer, sizeof(buffer));
        if (res < 0) {
            printf("Unable to get feature report\n");
            break;
        }

        // 受信データを解析して角速度を取得
        parse_report(buffer, sizeof(buffer), &sample);

        // 取得した角速度データを処理
        process_ang_vel(sample.ang_vel, ang_vel);

        // タイムスタンプの差から経過時間を計算
        uint32_t tick_delta = 3906;
        if (last_sample_tick > 0)
            tick_delta = sample.tick - last_sample_tick;

        float dt = tick_delta * TICK_LEN;
        last_sample_tick = sample.tick;

        // カメラの回転を更新
        update_rotation(dt, ang_vel);

    } while (res);

    // HIDデバイスを閉じる
    hid_close(device);
    res = hid_exit();

    return 0;
}
