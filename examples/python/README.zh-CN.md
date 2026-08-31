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

需要目标手输出时，先安装 QnBot CLI 0.1.0，或产品交付说明指定的兼容版本。

macOS 或 Linux：

```bash
curl -fsSL https://get.qnbot.com/cli | bash
```

Windows PowerShell：

```powershell
irm https://get.qnbot.com/cli.ps1 | iex
```

安装产品交付的算法包并确认安装结果：

```bash
qnbot --version
qnbot algorithm install ./package.zip
qnbot algorithm list
```

将 `./package.zip` 替换为与当前操作系统、CPU 架构和 SDK 版本匹配的算法包文件。

## 安装

```bash
python -m pip install qnbot-sdk-glove
```

也可以从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases)
下载与当前平台匹配的 wheel。

## 快速运行

连接一只受支持的手套，并确认目标手算法包已经安装后运行：

```bash
python src/quick_start.py --port /dev/ttyUSB0 --side left --package-id <package-id>
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
| `host_capture_calibration.py` | 是，两只 | 是 | 先完成左右手采集，再按算法包保存标定结果 |

## 采集与标定

普通目标手程序不需要自行编排采集或标定。选择算法包并启动后，SDK 会先复用当前操作员
已有的兼容结果；缺少结果时，默认终端流程会显示算法包提供的动作提示，依次完成原始帧
采集、标定计算、保存和激活，完成后才开始产生目标关节输出。

Capture 收集手套原始帧，Calibration 使用这些原始帧为选定算法包和 target 计算结果。
需要重新计算但希望保留已有原始帧时，使用 `CalibrationConfig(force=True)`；需要连原始帧
也重新采集时，使用 `CaptureConfig(force=True)`。两个 `force` 都只影响对应阶段，不表示
持续采集或每帧重新标定。

GUI、ROS2 或远程程序可以使用 `external` 交互显示 SDK 的 prompt 和进度，并将当前
`request_id` 原样传给 `confirm()`、`skip()` 或 `cancel()`；应用不自行实现阶段顺序、
采样、计算或保存。只有“先采集、后选择算法包”的上位机业务，才需要运行
`host_capture_calibration.py` 的独立两阶段流程。

## 常用命令

```bash
python src/discover_gloves.py
python src/external_input.py
python src/skeleton.py --port /dev/ttyUSB0 --side left
python src/quick_start.py --port /dev/ttyUSB0 --side left --package-id <package-id>
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

首次采集时可以同时评估多个候选算法包：

```bash
python src/host_capture_calibration.py \
  --package-id <package-id-1> \
  --package-id <package-id-2>
```

需要忽略可复用原始帧并完整重采左右手时，显式增加 `--force`：

```bash
python src/host_capture_calibration.py --package-id <package-id> --force
```

传入一个包时，采集后直接为该包完成标定；传入多个包时，SDK 合并这些包所需采集，
采集完成后提示从已传入包中选择一个完成标定。后续加入新的 `--package-id <package-id>` 时，
SDK 会把各 stage 标记为
`reusable` 或 `needs_capture`。默认 `force=False`，可复用的采集结果不会重复采集；只需
按提示补齐新候选实际需要的 stage。

Windows 请把串口参数替换为实际的 `COM` 端口，例如 `--port COM3`。

## 参数说明

- `--port`：手套串口；使用 `--port` 的参数化示例必须显式提供串口。
- `--side`：手套物理侧，取值为 `left` 或 `right`。
- `--package-id`：已经安装的目标手算法包 ID；需要目标输出的示例必须显式提供，该参数不会安装算法包。
- `--force`：仅用于上位机两阶段示例；忽略可复用原始帧并完整重新采集，不表示持续采集。
- `--target-name`：应用为输出指定的名称。

`discover_gloves.py` 会自动发现设备并枚举结果。`quick_start.py` 和 `skeleton.py` 要求显式传入
`--port` 与 `--side`，因此连接多只手套时也不会产生选择歧义。

`quick_start.py` 和其他需要目标输出的示例都要求显式传入 `--package-id <package-id>`。
`skeleton.py` 在 `connect()` 后、`start()` 前完成 `device.skeleton()` 配置；启动前重复配置
是幂等的，设备启动后不能再新增该能力。

`host_capture_calibration.py` 使用 SDK 默认 operator 和 `openxr_hand` 目标保存标定，结果可由
其他目标输出示例直接复用。采集确认可输入 `y`、`yes`、`confirm` 或直接按 Enter。

### Receiver、重连与终端输出

现有 `discover_gloves.py`、`device_lifecycle.py`、`skeleton.py` 以及其他读取 pose、status 和 Skeleton
的按 side 示例同样适用于受支持的无线 Receiver，不需要专用配置。左右手声明可以显式选择
同一个 Receiver 串口；若其中一只已配置手套在首次连接时离线，其他在线手套仍可使用，
但离线 source 及其关联输出不会加入当前 SDK 实例，打开离线手套后需要重新创建 SDK。
Receiver 不提供 haptics；需要触觉反馈时请使用提供 haptics 的直连手套。

已经成功连接的 source 后续断连时会自动恢复。恢复期间状态为 `connected=false`、
`stale=true` 并保留 `last_error`，成功后 `reconnect_count` 增加；已启动 source 恢复 telemetry 后 `stale=false`，
尚未 start 的 source 仍为 `stale=true`。在提供 haptics 的直连手套上，`haptics.py` 示例在
断连期间发送命令会立即失败，不排队、不重放；恢复后需要重新设置 haptics。

`foreground_runtime.py` 等使用内置终端的示例只在交互式 TTY 中显示颜色。设置
`NO_COLOR` 或在非 TTY、pipe、重定向环境运行时输出纯文本；Debug 和 diagnostic JSON
始终不包含 ANSI 转义序列。

## 生命周期与 Debug 排障

SDK 实例退出前应停止已启动的设备；需要重新纳入首次连接时离线的 source 时，请重新创建 SDK。

## 常见问题

- 无法安装 SDK：确认 Python 版本符合要求、操作系统和 CPU 架构匹配支持范围，并检查 PyPI 网络连接。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并尝试运行 `discover_gloves.py`。
- 找不到算法包：按照产品交付说明重新通过 CLI 安装，并确认传入了正确的 package ID。
- 没有目标手输出：确认已完成终端提示的标定，并保持手套连接。

如问题仍未解决，请保留运行命令和完整错误信息并联系 QnBot 对接人员。
