using System.Diagnostics;
using UnityEngine;
using System.IO;
using System;
using System.Threading;

// プロセス間通信でCのバイナリを取得して、unityのカメラに反映する
public class ar_test_ipc: MonoBehaviour
{
    GameObject mainCamObj;

    public float pitchAngle;
    public float yawAngle;
    public float rollAngle;


    private Process cProcess;
    private StreamReader outputReader;
    private Thread readThread;

    private Quaternion firstRotation;

    void Start()
    {
        mainCamObj = Camera.main.gameObject;
        firstRotation = mainCamObj.transform.rotation;

        // プロセスを作成して外部プログラムを実行
        cProcess = new Process();
        cProcess.StartInfo.FileName = Path.Combine(Application.dataPath, "get_xreal_rotation/inspector");
        cProcess.StartInfo.UseShellExecute = false;
        cProcess.StartInfo.RedirectStandardOutput = true;
        cProcess.StartInfo.CreateNoWindow = true;
        cProcess.Start();

        // 外部プログラムの出力を非同期に読み取るスレッドを開始
        outputReader = cProcess.StandardOutput;
        readThread = new Thread(new ThreadStart(ReadOutput));
        readThread.Start();
    }

    void Update()
    {
        UpdateCameraRotation(pitchAngle, yawAngle, rollAngle);
    }

    // カメラを初期位置にリセット
    public void ResetRotate()
    {
        mainCamObj.transform.rotation = firstRotation;
    }

    // プログラムが終了した時にスレッドを終了する
    public void DestroyC()
    {
        UnityEngine.Debug.Log("プロセスキルしてプレイモード終了");

        if (cProcess != null && !cProcess.HasExited)
        {
            cProcess.Kill();
        }

        if (readThread != null && readThread.IsAlive)
        {
            readThread.Abort();
        }

        //ゲームプレイ終了
#if UNITY_EDITOR
        UnityEditor.EditorApplication.isPlaying = false;
#else
        Application.Quit();
#endif

    }

    // 外部プログラムの出力を読み取り、クォータニオンの保存処理を行う
    private void ReadOutput()
    {
        while (true)
        {
            string output = outputReader.ReadLine();

            // プロセスが終了した場合はループを抜ける
            if (output == null)
            {
                UnityEngine.Debug.Log("プロセス終了");
                break;
            }

            float[] floatArray = ParseFloatArray(output);

            // UnityのQuaternionに変換してMain Cameraの角度を変更
            // メインスレッドじゃないと、ゲームオブジェクトを取得できない
            // UpdateCameraRotation(angles[0], angles[1], angles[2]);
            pitchAngle = floatArray[0];
            yawAngle = floatArray[1];
            rollAngle = floatArray[2];
        }
    }

    // Quaternion.Eulerを使用してMain Cameraの角度を変更
    private void UpdateCameraRotation(float pitchAngle, float yawAngle, float rollAngle)
    {
        float pitch = pitchAngle * 750.0f; // 適宜調整
        float yaw = yawAngle * 750.0f; // 適宜調整
        float roll = rollAngle * 750.0f; // 適宜調整

        mainCamObj.transform.rotation = mainCamObj.transform.rotation * Quaternion.Euler(-pitch, -yaw, roll);
    }

    // 文字列を配列化する
    static float[] ParseFloatArray(string input)
    {
        // 空白で分割して文字列配列に変換
        string[] stringValues = input.Split(' ');

        // float型の配列に変換
        float[] floatArray = new float[stringValues.Length];
        for (int i = 0; i < stringValues.Length; i++)
        {
            if (float.TryParse(stringValues[i], out float floatValue))
            {
                floatArray[i] = floatValue;
            }
            else
            {
                UnityEngine.Debug.Log("パースに失敗");
            }
        }

        return floatArray;
    }


}
