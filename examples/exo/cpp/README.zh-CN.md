# QnBot SDK Exo C++ 示例

`src/` 目录存放可以直接运行的示例源码，不存放 SDK 自身的实现代码。

本目录提供可直接构建的 Exo C++ 客户示例，覆盖设备发现、设备信息、遥测与状态、IMU、
手柄、触觉反馈和多设备生命周期。

## 准备工作

- CMake 3.16 或更高版本；
- 支持 C++17 的编译器；
- 当前操作系统和 CPU 架构匹配的 QnBot C++ SDK/Exo 交付包。

从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台的
SDK/Exo 交付包。

## 构建

将 `/path/to/qnbot` 替换为解压后的 SDK CMake package 路径：

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/qnbot
cmake --build build --config Release
```

多配置生成器的可执行文件通常位于 `build/Release/`，单配置 Ninja 通常位于 `build/`。

## 快速运行

连接一台受支持的 Exo 设备后运行：

```bash
./build/quick_start
```

该示例无需参数：SDK 自动发现唯一连接的设备，持续打印遥测；按 `Ctrl+C` 停止。没有发现
设备或同时发现多台设备时，先运行 `discover_exos`，再使用支持显式设备参数的示例。

## 示例列表

| 示例 | C++ 标准 | 是否需要设备 | 用途 |
| --- | --- | --- | --- |
| `discover_exos` | C++17 | 否 | 查看枚举到的串口及其设备信息或失败原因 |
| `quick_start` | C++17 | 是 | 零参数读取单台设备遥测 |
| `device_info` | C++17 | 是 | 启动前读取设备信息与能力集 |
| `telemetry` | C++17 | 是 | 读取遥测与状态通道 |
| `imu` | C++17 | 是 | 读取躯干与扩展 IMU 载荷 |
| `handset` | C++17 | 是 | 读取左右手柄轴、扳机和按键 |
| `haptics` | C++17 | 是 | 设置并清除左右触觉强度 |
| `device_lifecycle` | C++17 | 是，两台 | 分别管理两台设备 |

`imu` 和 `handset` 先读取设备上报的能力集；设备没有对应能力时直接说明该载荷不可用，
不输出空结构。

单设备示例取得 `auto exo = sdk.exo()` 后，优先使用 `exo.start()`、`exo.run_forever()`、
`exo.stop()` 和 `exo.close()` 管理整个 Exo 域；`device` 对象只负责选择设备、读取数据和
执行设备能力操作。`device_lifecycle` 是多设备例外，用设备级调用演示分别控制两台设备。

## 常用命令

```bash
./build/discover_exos
./build/quick_start
./build/device_info --port /dev/ttyUSB0
./build/telemetry --port /dev/ttyUSB0
./build/imu --port /dev/ttyUSB0
./build/handset --port /dev/ttyUSB0
./build/haptics --port /dev/ttyUSB0 --left 60 --right 60 --hold 1
./build/device_lifecycle --first-port /dev/ttyUSB0 --second-port /dev/ttyUSB1
```

Windows 请将串口替换为实际的 `COM` 端口，例如 `--port COM3`。

## 参数说明

- `--port`：设备串口；使用该参数的示例必须显式提供。
- `--name`：应用为设备指定的名称，默认 `primary`。
- `--left`、`--right`：`haptics` 的左右触觉强度，取值为 0 到 100。
- `--hold`：`haptics` 保持触觉的时间，单位为秒。
- `--first-port`、`--second-port`：`device_lifecycle` 使用的两个串口。
- `--seconds`：`device_lifecycle` 观察两台设备的时长，单位为秒。

## 常见问题

- 无法配置工程：确认 CMake 版本符合要求，并确认 Exo 交付包已解压且 `CMAKE_PREFIX_PATH`
  指向该路径。
- 找不到设备：确认设备已连接、串口未被其他程序占用，并运行 `discover_exos`。
- 没有遥测输出：确认设备已启动，并确认设备上报了遥测能力。
- 没有 IMU 或手柄输出：确认设备上报了对应能力集。

仍无法解决时，请保留运行命令和完整错误信息并联系 QnBot 对接人员。
