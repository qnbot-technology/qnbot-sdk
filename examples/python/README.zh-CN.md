# QnBot SDK Python 示例

本目录提供可直接运行的 Python 客户示例，演示安装、设备发现、手套数据读取、目标手输出、
运行模式和触觉反馈。
所有可执行示例均位于 `src/`，每个文件都可以单独运行。

## 准备工作

运行前请准备：

- Python 3.10 或更高版本；
- 可以访问 PyPI 的 Python 环境；
- 需要真实手套的示例所使用的受支持设备；
- 需要目标手输出的示例所使用的算法包。

目标手算法包请按照产品交付说明，通过随产品提供的 CLI 安装。

## 安装

```bash
pip install qnbot-sdk-glove
```

## 快速运行

连接一只受支持的手套，并确认目标手算法包已经安装后运行：

```bash
python src/quick_start.py
```

首次运行如出现标定提示，请按终端提示完成操作。成功后程序会持续输出姿态和目标手结果；
按 `Ctrl+C` 停止。

## 示例列表

| 示例 | 是否需要手套 | 是否需要目标手算法包 | 用途 |
| --- | --- | --- | --- |
| `discover_gloves.py` | 是 | 否 | 查看当前可用手套 |
| `external_input.py` | 否 | 否 | 使用外部输入检查基本数据流程 |
| `skeleton.py` | 是 | 否 | 读取手部骨骼数据 |
| `quick_start.py` | 是 | 是 | 最小目标手输出示例 |
| `manual_runtime.py` | 是 | 是 | 在应用循环中主动更新 |
| `foreground_runtime.py` | 是 | 是 | 在当前线程持续运行 |
| `background_runtime.py` | 是 | 是 | 在后台运行并主动停止 |
| `async_runtime.py` | 是 | 是 | 使用 `asyncio` 异步读取输出 |
| `debug_trace.py` | 是 | 是 | 记录运行诊断信息 |
| `multiple_outputs.py` | 是 | 是 | 从一只手套读取多路目标输出 |
| `device_lifecycle.py` | 是，两只 | 是 | 分别管理左右手设备 |
| `haptics.py` | 是 | 否 | 设置并清除触觉反馈 |

## 常用命令

```bash
python src/discover_gloves.py
python src/external_input.py
python src/skeleton.py
python src/quick_start.py
python src/manual_runtime.py --port /dev/ttyUSB0 --side left --updates 100
python src/foreground_runtime.py --port /dev/ttyUSB0 --side left
python src/background_runtime.py --port /dev/ttyUSB0 --side left --seconds 10
python src/async_runtime.py --port /dev/ttyUSB0 --side left --samples 10
python src/debug_trace.py --port /dev/ttyUSB0 --side left --detail full --sample-rate 10
python src/multiple_outputs.py --port /dev/ttyUSB0 --side left --package-id qnbot-dexhand
python src/device_lifecycle.py --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1 --updates 10
python src/haptics.py --port /dev/ttyUSB0 --side left --hold 1
```

Windows 请把串口参数替换为实际的 `COM` 端口，例如 `--port COM3`。

## 参数说明

- `--port`：手套串口；省略时由示例自动发现设备。
- `--side`：手套物理侧，取值为 `left` 或 `right`。
- `--package-id`：已经安装的目标手算法包 ID，例如 `qnbot-dexhand`；该参数不会安装算法包。
- `--target-name`：应用为输出指定的名称。

`quick_start.py` 使用默认算法包 ID。如需使用交付给你的其他算法包，请修改示例中的
`TARGET_PACKAGE_ID`，或在支持该参数的示例中传入 `--package-id`。

## 常见问题

- 无法安装 SDK：确认 Python 版本符合要求、操作系统和 CPU 架构匹配支持范围，并检查 PyPI 网络连接。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并尝试运行 `discover_gloves.py`。
- 找不到算法包：按照产品交付说明重新通过 CLI 安装，并确认传入了正确的 package ID。
- 没有目标手输出：确认已完成终端提示的标定，并保持手套连接。

如问题仍未解决，请保留运行命令和完整错误信息并联系 QnBot 对接人员。
