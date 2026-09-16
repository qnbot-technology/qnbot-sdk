# QnBot SDK Python 示例

本目录提供可直接运行的 Python 客户示例，覆盖设备发现、手套姿态、有线 IMU 原始数据、
目标手输出、运行模式、触觉反馈以及采集与标定。
可运行的示例源码统一位于 `src/` 目录。

## 准备工作

- Python 3.10 或更高版本；
- 可以访问 PyPI 的 Python 环境；
- 与当前操作系统和 CPU 架构匹配的受支持手套；
- 目标手输出示例所需的算法包。

需要目标手输出时，先安装 QnBot CLI 0.2.0，或产品交付说明指定的兼容版本。

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

## 安装

```bash
python -m pip install qnbot-sdk-glove
```

也可以从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases)
下载与当前平台匹配的 wheel。

## 快速运行

连接一只受支持的手套后运行：

```bash
python src/quick_start.py
```

该示例无需参数，也不加载算法包。SDK 会自动发现唯一连接的手套并持续输出姿态；按
`Ctrl+C` 停止。没有发现手套或同时发现多只手套时，先运行 `discover_gloves.py`，再使用
支持显式设备参数的示例。

## 示例列表

| 示例 | 是否需要手套 | 是否需要目标手算法包 | 用途 |
| --- | --- | --- | --- |
| `discover_gloves.py` | 是 | 否 | 查看当前可用手套 |
| `external_input_run_background.py` | 否 | 是 | 由应用提供手套帧，SDK 在后台运行 |
| `external_input_manual_update.py` | 否 | 是 | 由应用提供手套帧，并由应用手动更新 |
| `skeleton.py` | 是 | 否 | 读取手部骨骼数据 |
| `quick_start.py` | 是 | 否 | 零参数读取单只手套姿态 |
| `imu.py` | 是 | 否 | 读取有线 IMU 原始计数 |
| `manual_runtime.py` | 是 | 是 | 由应用控制每次更新 |
| `foreground_runtime.py` | 是 | 是 | 在当前线程持续运行 |
| `background_runtime.py` | 是 | 是 | 在后台运行并主动停止 |
| `async_runtime.py` | 是 | 是 | 使用 `asyncio` 异步读取输出 |
| `debug_trace.py` | 是 | 是 | 生成诊断日志 |
| `multiple_outputs.py` | 是 | 是 | 从一只手套读取多路目标输出 |
| `device_lifecycle.py` | 是，两只 | 是 | 分别管理左右手设备 |
| `haptics.py` | 是 | 否 | 设置并清除触觉反馈 |
| `host_capture_calibration.py` | 是，两只 | 是 | 完成左右手采集并保存、应用标定结果 |

两个 external input 示例都需要显式提供算法包：后台示例适合普通应用，手动更新示例适合
由应用控制每次更新的循环。使用算法包的示例必须显式传入 `--package-id`，两个 external
input 示例也不例外。

## 常用命令

```bash
python src/discover_gloves.py
python src/external_input_run_background.py --package-id <package-id>
python src/external_input_manual_update.py --package-id <package-id>
python src/skeleton.py --port /dev/ttyUSB0 --side left
python src/quick_start.py
python src/imu.py --port /dev/ttyUSB0 --side left
python src/manual_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id> --updates 100
python src/foreground_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id>
python src/background_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id> --seconds 10
python src/async_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id> --samples 10
python src/debug_trace.py --port /dev/ttyUSB0 --side left --package-id <package-id> --detail full --sample-rate 10
python src/multiple_outputs.py --port /dev/ttyUSB0 --side left --package-id <package-id>
python src/device_lifecycle.py --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1 --package-id <package-id> --updates 10
python src/haptics.py --port /dev/ttyUSB0 --side left --hold 1
python src/host_capture_calibration.py --package-id <package-id>
```

Windows 请将串口替换为实际的 `COM` 端口，例如 `--port COM3`。

`host_capture_calibration.py` 可同时接收多个包，完成左右手采集后选择一个已配置 target，
保存并应用标定结果：

```bash
python src/host_capture_calibration.py \
  --package-id <package-id-1> \
  --package-id <package-id-2>
```

需要忽略可复用采集并重新采集时增加 `--force`。只标定 SDK 内置 Skeleton 时使用固定 ID：

```bash
python src/host_capture_calibration.py \
  --package-id qnbot_hand.dynamic_openxr_hand
```

## 参数说明

- `--port`：手套串口；使用 `--port` 的参数化示例必须显式提供串口。
- `--side`：手套物理侧，取值为 `left` 或 `right`。
- `--package-id`：已经安装的目标手算法包 ID；该参数不会安装算法包。
- `--target-name`：应用为输出指定的名称。
- `--force`：`host_capture_calibration.py` 忽略可复用采集并重新采集。
- `--detail`、`--sample-rate`：控制 `debug_trace.py` 的诊断日志详细程度和采样率。

`debug_trace.py` 生成的诊断日志固定写入 `<QNBOT_HOME>/logs`。

缺少标定时按终端提示操作，完成后示例会开始输出目标关节结果。

当 SDK 能将该端口识别为受支持的有线直连手套，且同时显式提供 `--port`（配置字段
`port`）和 `--side`（配置字段 `side`）时，无法报告设备身份的旧版有线直连手套仍可
连接，并发出“身份未验证”告警。断线恢复只重试该串口，不能确认仍是同一只物理手套。
自动发现、只提供其中一个参数、按 `serial_number` 选择、无线 Receiver 以及
无法识别端口类型时仍要求设备身份校验。

`skeleton.py` 在 SDK 创建后、`start()` 前配置 Skeleton；启动前重复配置是幂等的。

`imu.py` 在启动前调用 `subscribe()`，输出有符号 16 位 `gyroscope_raw` 和
`accelerometer_raw` 原始计数。SDK 不做单位换算、校准、滤波、偏置修正或融合。有线 IMU
原始数据不需要目标手算法包；无线 Receiver/QnTP IMU 当前不支持。异步程序可使用
`await imu.next()` 等待样本。

## 常见问题

- 无法安装 SDK：确认 Python 版本符合要求、操作系统和 CPU 架构匹配支持范围，并检查
  PyPI 网络连接。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并运行 `discover_gloves.py`。
- 找不到算法包：通过 CLI 安装正确的产品算法包，并检查 `--package-id`。
- 没有目标手输出：确认已完成终端提示的标定，并保持手套连接。
- 没有 IMU 输出：确认使用支持 IMU 的 USB 直连手套，并在启动前订阅 IMU channel。

仍无法解决时，请保留运行命令和完整错误信息并联系 QnBot 对接人员。
