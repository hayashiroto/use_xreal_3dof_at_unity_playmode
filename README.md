# Summary
* Retrieve accelerometer data of nreal air using `hidapi`.
* Create a C binary that calculates quaternions using `cglm` and outputs them to the standard output.
* Create a process from Unity's main thread and execute an external binary.
* Receive the output quaternions and apply them to Unity's main camera.

# 概要
* hidapiを用いてnreal airの加速度センサー情報を取得。
* cglmを用いてクォータニオンを算出し標準出力するcのバイナリを作成。
* unityのメインスレッドから、プロセスを作成し外部のバイナリを実行。
* 出力されたクォータニオンを受け取り, unityのメインカメラに反映。　
* macOSで使用可能

![カメラのクォータニオンと](https://github.com/hayashiroto/use_xreal_3dof_at_unity_playmode/assets/69220422/28e383c4-9875-4b8e-9902-60c38583df14)

# 従来の開発方法
* AR　グラスの機能を使えないし、いちいちビルドする必要があった
![XREALをMACに接続](https://github.com/hayashiroto/use_xreal_3dof_at_unity_playmode/assets/69220422/6e8fe829-3ca8-49c0-9c44-a72ebbff50f7)


# 新しい開発方法
* ARグラスをかけながら、エディタなどで開発をし、そのままUNITYのプレイモードで動作確認することができる。
* その時、加速度センサーの情報も反映される。
![IMG_1909](https://github.com/hayashiroto/use_xreal_3dof_at_unity_playmode/assets/69220422/b92b0758-31f5-44bf-8875-9b6f4e7bb385)


# environment info

```
unity 2022.3.13f1
macOS Ventura 13.5
tip m1
Apple clang version 15.0.0 (clang-1500.0.40.1)
Target: x86_64-apple-darwin22.6.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
```

# how to use

* build c program
```bash
[ your_repo ]git clone https://github.com/hayashiroto/use_xreal_3dof_at_unity_playmode.git
[ your_repo ]cd xreal_unity_3dof_playmode_02
[ xreal_unity_3dof_playmode_02 ]cd Assets/get_xreal_rotation/     
[ get_xreal_rotation ]git clone https://github.com/recp/cglm.git
[ get_xreal_rotation ]vim .
[ get_xreal_rotation ]make    
[ get_xreal_rotation ]./inspector 
0.000008 -0.000007 -0.000007 1.000000
0.000008 -0.000009 -0.000007 1.000000
-0.000010 -0.000004 0.000004 1.000000
0.000008 -0.000006 -0.000006 1.000000
0.000015 -0.000001 0.000006 1.000000
....
success
```
* open unity with unity hub
* play

# main script
* https://github.com/hayashiroto/use_xreal_3dof_at_unity_playmode/blob/main/Assets/get_xreal_rotation/inspector.c
* https://github.com/hayashiroto/use_xreal_3dof_at_unity_playmode/blob/main/Assets/ar_test_ipc.cs

# 注意事項
* cのビルドでエラーが発生した際はエラーハンドリングして解決してください
* kill button を押してunityのプレイモードは終わらせてください。
* 完全に同期しているわけではんく、コードの中でいい感じに経験則に基づいて調整しています。いい調整方法がありましたら、PR出してください。

# Notes
* Handle errors if they occur during C build.
* Press the "kill" button to exit Unity play mode.
* It's not perfectly synchronized; adjustments are based on empirical knowledge in the code. If you have better adjustment methods, please submit a pull request.

# Repository Used as Reference
* https://github.com/abls/real-air/tree/master

# Libraries Used in C
* https://github.com/libusb/hidapi
* https://github.com/recp/cglm
