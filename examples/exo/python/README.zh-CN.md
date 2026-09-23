# QnBot SDK Exo Python 示例

`src/` 目录存放可以直接运行的示例源码，不存放 SDK 自身的实现代码。

本目录提供可直接运行的 Exo Python 客户示例，覆盖设备发现、设备信息、遥测与状态、IMU、
手柄、触觉反馈和多设备生命周期。

## 准备工作

- Python 3.10 或更高版本；
- 与当前操作系统和 CPU 架构匹配的受支持 Exo 设备；
- 已安装 `qnbot-sdk-exo`。

## 安装

使用产品交付的 Exo Python 安装包：

```bash
python -m pip install ./qnbot_sdk_exo-<version>-py3-none-any.whl
```

也可以从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases)
下载与当前平台匹配的 Exo 交付包。

## 快速运行

连接一台受支持的 Exo 设备后运行：

```bash
python src/quick_start.py
```

该示例无需参数：SDK 自动发现唯一连接的设备，持续打印遥测；按 `Ctrl+C` 停止。没有发现
设备或同时发现多台设备时，先运行 `discover_exos.py`，再使用支持 `--port` 的示例。

## 示例列表

| 示例 | 是否需要设备 | 用途 |
| --- | --- | --- |
| `discover_exos.py` | 否 | 查看枚举到的串口及其设备信息或失败原因 |
| `quick_start.py` | 是 | 零参数读取单台设备遥测 |
| `device_info.py` | 是 | 启动前读取设备信息与能力集 |
| `telemetry.py` | 是 | 读取遥测与状态通道 |
| `imu.py` | 是 | 读取躯干与扩展 IMU 载荷 |
| `handset.py` | 是 | 读取左右手柄轴、扳机和按键 |
| `haptics.py` | 是 | 设置并清除左右触觉强度 |
| `device_lifecycle.py` | 是，两台 | 分别管理两台设备 |

`imu.py` 和 `handset.py` 先读取设备上报的能力集；设备没有对应能力时直接说明该载荷
不可用，不输出空结构。

单设备示例取得 `exo = sdk.exo()` 后，优先使用 `exo.start()`、`exo.run_forever()`、
`exo.stop()` 和 `exo.close()` 管理整个 Exo 域；`device` 对象只负责选择设备、读取数据和
执行设备能力操作。`device_lifecycle.py` 是多设备例外，用设备级调用演示分别控制两台设备。

## 常用命令

```bash
python src/discover_exos.py
python src/quick_start.py
python src/device_info.py --port /dev/ttyUSB0
python src/telemetry.py --port /dev/ttyUSB0
python src/imu.py --port /dev/ttyUSB0
python src/handset.py --port /dev/ttyUSB0
python src/haptics.py --port /dev/ttyUSB0 --left 60 --right 60 --hold 1
python src/device_lifecycle.py --first-port /dev/ttyUSB0 --second-port /dev/ttyUSB1
```

Windows 请将串口替换为实际的 `COM` 端口，例如 `--port COM3`。

## 参数说明

- `--port`：设备串口；使用该参数的示例必须显式提供。
- `--name`：应用为设备指定的名称，默认 `primary`。
- `--left`、`--right`：`haptics.py` 的左右触觉强度，取值为 0 到 100。
- `--hold`：`haptics.py` 保持触觉的时间，单位为秒。
- `--first-port`、`--second-port`：`device_lifecycle.py` 使用的两个串口。
- `--seconds`：`device_lifecycle.py` 观察两台设备的时长，单位为秒。

## 常见问题

- 无法安装 SDK：确认 Python 版本符合要求，并使用与当前平台匹配的 Exo 交付包。
- 找不到设备：确认设备已连接、串口未被其他程序占用，并运行 `discover_exos.py`。
- 没有遥测输出：确认设备已启动，并确认设备上报了遥测能力。
- 没有 IMU 或手柄输出：确认设备上报了对应能力集。

仍无法解决时，请保留运行命令和完整错误信息并联系 QnBot 对接人员。
