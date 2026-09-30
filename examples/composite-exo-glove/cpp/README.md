# QnBot SDK Exo + Glove C++ 示例

`src/` 目录存放可直接构建和运行的示例源码，不存放 SDK 自身的实现代码。

本目录提供使用同一共享串口连接访问一台 Exo 和左右两只 Glove 的 C++ 示例。每个示例都通过
`qnbot::CompositeExoGloveConfig` 配置组合设备，再从同一个 `qnbot::Sdk` 获取
`sdk.glove()` 和 `sdk.exo()` 入口。示例先保存 `glove` 和 `exo` 领域对象，再通过它们访问成员设备。

默认组合流程使用根 `Sdk` 控制整体生命周期：

```cpp
auto glove = sdk.glove();
auto exo = sdk.exo();
auto left_glove_device = glove.device(qnbot::Side::left);
auto right_glove_device = glove.device(qnbot::Side::right);
auto exo_device = exo.device();

sdk.start();
sdk.run_forever(); // 或 sdk.run_background()
sdk.stop();
sdk.close();
```

只有需要单独控制某个领域时，才调用 `glove.start()`、`glove.stop()`、`exo.start()` 或 `exo.stop()`。

## 准备工作

- CMake 3.16 或更高版本；
- 支持 C++17 的编译器；
- 同一版本的 QnBot C++ Exo 和 Glove 交付包。

从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台的
SDK 交付包，并确保 CMake 能找到 `qnbot_exo` 和 `qnbot_glove`。

## 构建

将 `/path/to/qnbot` 替换为解压后的 SDK CMake package 路径：

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/qnbot
cmake --build build --config Release
```

多配置生成器的可执行文件通常位于 `build/Release/`，单配置 Ninja 通常位于 `build/`。

## 示例列表

| 示例 | 是否需要设备 | 用途 |
| --- | --- | --- |
| `discover_exo_glove` | 是 | 使用自动发现，展示左右 Glove source 和 Exo 信息 |
| `quick_start` | 是 | 使用默认自动发现，同时订阅左右 Glove pose 和 Exo telemetry |
| `device_info` | 是 | 展示左右 Glove source，并读取 Exo 设备信息 |
| `telemetry` | 是 | 读取左右 Glove pose/status、Exo telemetry 及 Exo status |
| `imu` | 是 | 读取 Exo IMU，并提示当前 composite CDC 不提供 Glove `0x10/0x81` 原始 IMU |
| `glove_haptics` | 是 | 通过共享串口设置和清除左右 Glove 触觉反馈 |
| `lifecycle` | 是 | 演示启动、读取一次、停止和关闭 |

## 运行

自动发现示例不需要串口参数：

```bash
./build/discover_exo_glove
./build/quick_start
```

除 `quick_start` 外，其余示例使用同一个共享串口：

```bash
./build/device_info --port /dev/ttyUSB0
./build/telemetry --port /dev/ttyUSB0
./build/imu --port /dev/ttyUSB0
./build/glove_haptics --port /dev/ttyUSB0
./build/lifecycle --port /dev/ttyUSB0
```

Windows 请将串口替换为实际的 `COM` 端口，例如 `--port COM3`。

持续运行的示例会打印数据直到按 `Ctrl+C` 停止。`lifecycle` 和 `glove_haptics` 会自行完成
停止和关闭。

组合示例默认使用根 `Sdk` 执行整体启动、停止和关闭，并按 Glove 的 `side`
以及唯一 Exo 成员访问设备。`quick_start` 没有算法目标，启动后由无任务运行器保持等待，直到按 `Ctrl+C` 请求停止。

需要了解更多生命周期用法时，请参考 [Exo C++ 生命周期示例](../../exo/cpp/README.md)
和 [Glove C++ 生命周期示例](../../glove/cpp/README.md)。

## 常见问题

- CMake 找不到依赖：确认 `CMAKE_PREFIX_PATH` 指向包含 `qnbot_exo` 和 `qnbot_glove`
  配置文件的 SDK 目录，并且两个交付包版本一致。
- 自动发现没有找到三个成员：确认设备已连接、串口未被其他程序占用；也可以运行其他示例
  时通过 `--port` 指定共享串口。
- 没有 Exo IMU 或 Glove 触觉输出：设备可能不提供对应能力，示例会输出能力不可用或错误提示。
