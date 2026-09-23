# QnBot SDK Exo+Glove 共享串口示例

本目录演示 Exo 和 Glove 通过同一个共享串口接入，并在一个 SDK 实例中分别访问两个成员。
Python 和 C++ 示例覆盖自动发现、显式串口、设备信息、遥测、IMU、Exo 侧触觉和基础生命周期。

## 目录

- [Python 示例](python/README.zh-CN.md)
- [C++ 示例](cpp/README.zh-CN.md)

## 使用前准备

- Python 3.10 或更高版本，或支持 C++17 的编译器和 CMake 3.16+；
- 一台同时提供 Exo 与 Glove 数据的受支持设备；
- Python 同时安装 `qnbot-sdk-exo` 和 `qnbot-sdk-glove`；
- C++ 同时安装对应版本的 Exo 与 Glove 交付包。

## 支持的配置方式

示例使用 `CompositeExoGloveConfig` 描述组合设备，并在其中配置一个共享 `SerialConnection`。
`discover_exo_glove` 使用自动发现；其他示例使用命令行传入的显式串口。

同一个 `Sdk` 实例提供 `sdk.glove()` 和 `sdk.exo()` 两个访问入口。详细命令、参数和预期输出
请阅读对应语言目录的 README。

## 常见问题

- 找不到设备：确认组合设备已连接，且共享串口没有被其他程序占用。
- 只有一个成员有输出：确认设备固件同时提供 Exo 和 Glove 数据，并检查两个成员的信息。
- C++ 找不到包：确认 `CMAKE_PREFIX_PATH` 同时包含 Exo 和 Glove 的 CMake 配置文件。
