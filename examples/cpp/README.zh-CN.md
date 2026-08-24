# QnBot SDK C++ 示例

本目录提供可直接构建的 C++ 客户示例，演示设备发现、手套数据读取、目标手输出、运行模式
和触觉反馈。
所有可执行示例均位于 `src/`，并由本目录的 `CMakeLists.txt` 统一构建。

## 准备工作

运行前请准备：

- CMake 3.16 或更高版本；
- 支持 C++17 的编译器；`async_runtime` 需要 C++20；
- 当前操作系统和 CPU 架构匹配的 Core、Glove C++ 包；
- 需要真实手套的示例所使用的受支持设备；
- 需要目标手输出的示例所使用的算法包。

请从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases)
下载相互兼容的 Core 和 Glove C++ 包。目标手算法包请按照产品交付说明，通过随产品提供
的 CLI 安装。

## 构建

将 `/path/to/qnbot` 替换为解压后的 SDK CMake package 所在路径：

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/qnbot
cmake --build build --config Release
```

使用多配置生成器时，可执行文件通常位于 `build/Release/`；使用单配置 Ninja 时通常位于
`build/`。

## 快速运行

连接一只受支持的手套，并确认目标手算法包已经安装后运行：

```bash
./build/quick_start
```

首次运行如出现标定提示，请按终端提示完成操作。成功后程序会持续输出姿态和目标手结果；
按 `Ctrl+C` 停止。

## 示例列表

| 示例 | C++ 标准 | 是否需要手套 | 是否需要目标手算法包 | 用途 |
| --- | --- | --- | --- | --- |
| `discover_gloves` | C++17 | 是 | 否 | 查看当前可用手套 |
| `external_input` | C++17 | 否 | 否 | 使用外部输入检查基本数据流程 |
| `skeleton` | C++17 | 是 | 否 | 读取手部骨骼数据 |
| `quick_start` | C++17 | 是 | 是 | 最小目标手输出示例 |
| `manual_runtime` | C++17 | 是 | 是 | 在应用循环中主动更新 |
| `foreground_runtime` | C++17 | 是 | 是 | 在当前线程持续运行 |
| `background_runtime` | C++17 | 是 | 是 | 在后台运行并主动停止 |
| `async_runtime` | C++20 | 是 | 是 | 异步读取输出 |
| `debug_trace` | C++17 | 是 | 是 | 记录运行诊断信息 |
| `multiple_outputs` | C++17 | 是 | 是 | 从一只手套读取多路目标输出 |
| `device_lifecycle` | C++17 | 是，两只 | 是 | 分别管理左右手设备 |
| `haptics` | C++17 | 是 | 否 | 设置并清除触觉反馈 |

## 常用命令

无硬件示例：

```bash
./build/external_input
./build/manual_runtime --external --updates 10
./build/background_runtime --external --seconds 1
./build/foreground_runtime --external
```

真实手套示例：

```bash
./build/discover_gloves
./build/skeleton
./build/quick_start
./build/manual_runtime --port /dev/ttyUSB0 --side left --updates 100
./build/foreground_runtime --port /dev/ttyUSB0 --side left
./build/background_runtime --port /dev/ttyUSB0 --side left --seconds 10
./build/async_runtime --port /dev/ttyUSB0 --side left --samples 10
./build/debug_trace --port /dev/ttyUSB0 --side left --detail full --sample-rate 10
./build/multiple_outputs --port /dev/ttyUSB0 --side left --package-id qnbot-dexhand
./build/device_lifecycle --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1 --updates 10
./build/haptics --port /dev/ttyUSB0 --side left --hold 1
```

使用多配置生成器时，请把命令中的 `./build/` 替换为对应配置目录，例如
`./build/Release/`。

## 参数说明

- `--port`：手套串口；省略时由示例自动发现设备。
- `--side`：手套物理侧，取值为 `left` 或 `right`。
- `--package-id`：已经安装的目标手算法包 ID，例如 `qnbot-dexhand`；该参数不会安装算法包。
- `--target-name`：应用为输出指定的名称。
- `--external`：使用无硬件输入，仅用于支持该参数的示例。
- `--validate`：只检查示例参数，不连接设备。

## 常见问题

- CMake 找不到 `qnbot_glove`：确认 `CMAKE_PREFIX_PATH` 指向已解压的匹配 SDK 包。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并尝试运行 `discover_gloves`。
- 找不到算法包：按照产品交付说明重新通过 CLI 安装，并确认传入了正确的 package ID。
- 没有目标手输出：确认已完成终端提示的标定，并保持手套连接。

如问题仍未解决，请保留构建命令、运行命令和完整错误信息并联系 QnBot 对接人员。
