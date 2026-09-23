# QnBot SDK C++ 示例

`src/` 目录存放可以直接运行的示例源码，不存放 SDK 自身的实现代码。

本目录提供可直接构建的 C++ 客户示例，覆盖设备发现、手套姿态、有线 IMU 原始数据、
目标手输出、运行模式、触觉反馈以及采集与标定。
可运行的示例源码统一位于 `src/` 目录。

## 准备工作

- CMake 3.16 或更高版本；
- 支持 C++17 的编译器；`async_runtime` 需要 C++20；
- 当前操作系统和 CPU 架构匹配的 QnBot C++ SDK/Glove 交付包；
- 目标手输出示例所需的算法包。

从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台的
SDK/Glove 交付包。需要目标手输出时，先安装 QnBot CLI 0.2.0，或产品交付说明指定的
兼容版本。

macOS 或 Linux：

```bash
curl -fsSL https://get.qnbot.com/cli | bash
```

Windows PowerShell：

```powershell
irm https://get.qnbot.com/cli.ps1 | iex
```

使用 CLI 安装产品交付的算法包：

```bash
qnbot --version
qnbot algorithm install ./package.zip
qnbot algorithm list
```

`package.zip` 必须与当前操作系统、CPU 架构和 SDK 版本匹配。

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

连接一只受支持的手套后运行：

```bash
./build/quick_start
```

该示例无需参数，也不加载算法包。SDK 会自动发现唯一连接的手套并持续输出姿态；按
`Ctrl+C` 停止。没有发现手套或同时发现多只手套时，先运行 `discover_gloves`，再使用支持
显式设备参数的示例。

## 示例列表

| 示例 | C++ 标准 | 是否需要手套 | 是否需要目标手算法包 | 用途 |
| --- | --- | --- | --- | --- |
| `discover_gloves` | C++17 | 是 | 否 | 查看当前可用手套 |
| `external_input_run_background` | C++17 | 否 | 是 | 由应用提供手套帧，SDK 在后台运行 |
| `external_input_manual_update` | C++17 | 否 | 是 | 由应用提供手套帧，并由应用手动更新 |
| `skeleton` | C++17 | 是 | 否 | 读取手部骨骼数据 |
| `quick_start` | C++17 | 是 | 否 | 零参数读取单只手套姿态 |
| `imu` | C++17 | 是 | 否 | 读取有线 IMU 原始计数 |
| `manual_runtime` | C++17 | 是 | 是 | 由应用控制每次更新 |
| `foreground_runtime` | C++17 | 是 | 是 | 在当前线程持续运行 |
| `background_runtime` | C++17 | 是 | 是 | 在后台运行并主动停止 |
| `async_runtime` | C++20 | 是 | 是 | 使用 coroutine 异步读取输出 |
| `debug_trace` | C++17 | 是 | 是 | 生成诊断日志 |
| `multiple_outputs` | C++17 | 是 | 是 | 从一只手套读取多路目标输出 |
| `device_lifecycle` | C++17 | 是，两只 | 是 | 分别管理左右手设备 |
| `haptics` | C++17 | 是 | 否 | 设置并清除触觉反馈 |
| `host_capture_calibration` | C++17 | 是，两只 | 是 | 完成左右手采集并保存、应用标定结果 |

两个 external input 示例都需要显式提供算法包：后台示例适合普通应用，手动更新示例适合
由应用控制每次更新的循环。使用算法包的示例必须显式传入 `--package-id`，两个 external
input 示例也不例外。

手动更新示例在每次 `update()` 后调用 update result 的 `sleep()` 等待下一项 SDK 工作；
自动连接恢复由 SDK 自行推进，不需要运行方式或额外的 `update()` 调用。

## 常用命令

无需连接手套：

```bash
./build/external_input_run_background --package-id <package-id>
./build/external_input_manual_update --package-id <package-id>
```

真实手套：

```bash
./build/discover_gloves
./build/skeleton --port /dev/ttyUSB0 --side left
./build/quick_start
./build/imu --port /dev/ttyUSB0 --side left
./build/manual_runtime --port /dev/ttyUSB0 --side left --package-id <package-id> --updates 100
./build/foreground_runtime --port /dev/ttyUSB0 --side left --package-id <package-id>
./build/background_runtime --port /dev/ttyUSB0 --side left --package-id <package-id> --seconds 10
./build/async_runtime --port /dev/ttyUSB0 --side left --package-id <package-id> --samples 10
./build/debug_trace --port /dev/ttyUSB0 --side left --package-id <package-id> --detail full --sample-rate 10
./build/multiple_outputs --port /dev/ttyUSB0 --side left --package-id <package-id>
./build/device_lifecycle --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1 --package-id <package-id> --updates 10
./build/haptics --port /dev/ttyUSB0 --side left --hold 1
./build/host_capture_calibration --package-id <package-id>
```

`host_capture_calibration` 可同时接收多个包，完成左右手采集后选择一个已配置 target，保存并
应用标定结果：

```bash
./build/host_capture_calibration \
  --package-id <package-id-1> \
  --package-id <package-id-2>
```

程序会分别选择左右手设备，并查询当前设备的采集计划是否已有兼容、可复用的已保存采集
数据。需要忽略这些可复用数据并重新采集时增加 `--force`。只标定 SDK 内置 Skeleton 时
使用固定 ID：

```bash
./build/host_capture_calibration \
  --package-id qnbot_hand.dynamic_openxr_hand
```

使用多配置生成器时，将命令中的 `./build/` 替换为 `./build/Release/`。

## 参数说明

- `--port`：手套串口；使用 `--port` 的参数化示例必须显式提供串口。
- `--side`：手套物理侧，取值为 `left` 或 `right`。
- `--package-id`：已经安装的目标手算法包 ID；该参数不会安装算法包。
- `--target-name`：应用为输出指定的名称。
- `--force`：`host_capture_calibration` 忽略可复用采集并重新采集。
- `--detail`、`--sample-rate`：控制 `debug_trace` 的诊断日志详细程度和采样率。

`debug_trace` 生成的诊断日志固定写入 `<QNBOT_HOME>/logs`。按 `Ctrl+C` 后，示例会
先停止运行并关闭 SDK；关闭成功后，本次会话包含最终汇总。直接终止进程则可能留下不完整
日志。

缺少标定时按终端提示操作，完成后示例会开始输出目标关节结果。

当 SDK 能将该端口识别为受支持的有线直连手套，且同时显式提供 `--port`（配置字段
`port`）和 `--side`（配置字段 `side`）时，无法报告设备身份的旧版有线直连手套仍可
连接，并发出“身份未验证”告警。断线恢复只重试该串口，不能确认仍是同一只物理手套。
自动发现、只提供其中一个参数、按 `serial_number` 选择、无线 Receiver 以及
无法识别端口类型时仍要求设备身份校验。

`skeleton` 在 SDK 创建后、`start()` 前配置 Skeleton；启动前重复配置是幂等的。

`imu` 在启动前调用 `subscribe()`，输出有符号 16 位 `gyroscope_raw` 和
`accelerometer_raw` 原始计数。SDK 不做单位换算、校准、滤波、偏置修正或融合。有线 IMU
原始数据不需要目标手算法包；无线 Receiver/QnTP IMU 当前不支持。异步程序可使用
`imu.next()` 等待样本。

## 常见问题

- CMake 找不到 `qnbot_glove`：确认 `CMAKE_PREFIX_PATH` 指向解压后的匹配 SDK 包。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并运行 `discover_gloves`。
- 找不到算法包：通过 CLI 安装正确的产品算法包，并检查 `--package-id`。
- 没有目标手输出：确认已完成终端提示的标定，并保持手套连接。
- 没有 IMU 输出：确认使用支持 IMU 的 USB 直连手套，并在启动前订阅 IMU channel。

仍无法解决时，请保留构建命令、运行命令和完整错误信息并联系 QnBot 对接人员。
