# QnBot SDK Python 示例

`src/` 目录存放可以直接运行的示例源码，不存放 SDK 自身的实现代码。

本目录提供可直接运行的 Python 客户示例，覆盖设备发现、手套姿态、IMU 原始数据、
目标手输出、运行模式、触觉反馈以及采集与标定。
可运行的示例源码统一位于 `src/` 目录。

## 准备工作

- Python 3.10 或更高版本；
- 可以访问 PyPI 的 Python 环境；
- 运行真实手套示例时，需要与当前操作系统和 CPU 架构匹配的受支持手套；
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

## 无硬件设备选择示例

`device_selection.py` 使用 Glove **1.5.0** 及配套 Core **1.5.0**。
建议先创建独立虚拟环境，再安装当前配套版本：

```bash
python -m venv .venv-selection
# macOS / Linux
source .venv-selection/bin/activate
# Windows PowerShell 使用 .venv-selection\Scripts\Activate.ps1
python -m pip install qnbot-sdk-glove==1.5.0
python -m pip check
python src/device_selection.py
```

无需连接手套、安装算法包或传入参数。程序配置 `primary` 左右手和 `backup` 左手，
分别输出三次成功选择得到的 `source_id`，接着输出 `expected ambiguity: name=primary`
和 `expected ambiguity: side=left`，最后输出 `device selection complete` 并退出。
这些歧义提示是预期结果，不表示运行失败。

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
| `device_selection.py` | 否 | 否 | 按名称和左右手选择三台外部输入设备，观察歧义提示 |
| `discover_gloves.py` | 是 | 否 | 查看当前可用手套 |
| `device_info.py` | 是 | 否 | 读取手套设备信息和连接类型 |
| `external_input_run_background.py` | 否 | 是 | 由应用提供手套帧，SDK 在后台运行 |
| `external_input_manual_update.py` | 否 | 是 | 由应用提供手套帧，并由应用手动更新 |
| `skeleton.py` | 是 | 否 | 读取手部骨骼数据 |
| `subscription_config.py` | 是 | 否 | 选择最新值或有界 FIFO，并限制订阅交付频率 |
| `algorithm_configuration.py` | 是 | 是 | 使用已有骨长配置和算法参数覆盖文件 |
| `quick_start.py` | 是 | 否 | 零参数读取单只手套姿态 |
| `time_sync.py` | 是 | 否 | 在后台运行并定期使用外部时间源提供的 Unix 毫秒时间同步 Glove 时钟后读取姿态 |
| `imu.py` | 是 | 否 | 读取手套的带单位 IMU 数据 |
| `emf.py` | 是，支持 EMF 的手套 | 否 | 读取与 pose 配对的原始 EMF 矩阵和实际通道信息 |
| `manual_runtime.py` | 是 | 是 | 由应用控制每次更新 |
| `foreground_runtime.py` | 是 | 是 | 在当前线程持续运行 |
| `background_runtime.py` | 是 | 是 | 在后台运行并主动停止 |
| `async_runtime.py` | 是 | 是 | 使用 `asyncio` 异步读取输出 |
| `debug_trace.py` | 是 | 是 | 生成诊断日志 |
| `multiple_outputs.py` | 是 | 是 | 从一只手套读取多路目标输出 |
| `device_lifecycle.py` | 是，两只 | 否 | 多只手套的独立启停：停止、重启其中一只，其他手套继续输出 |
| `haptics.py` | 是 | 否 | 设置并清除触觉反馈 |
| `host_capture_calibration.py` | 是，两只 | 否，可选目标包 | 完成左右手采集并保存、应用标定结果 |
| `pressure.py` | 是，HMP-MX | 否 | 读取整帧压力摘要和所选点 |

两个 external input 示例都需要显式提供 `--package-id`，并在终端提示期间持续推帧；需要应用接管提示时加 `--capture-interaction external`。

## 常用命令

```bash
python src/device_selection.py  # 需先安装上面指定的兼容版本
python src/discover_gloves.py
python src/external_input_run_background.py --package-id <package-id>
python src/external_input_manual_update.py --package-id <package-id>
python src/skeleton.py --port /dev/ttyUSB0 --side left
python src/quick_start.py
python src/time_sync.py
python src/imu.py --port /dev/ttyUSB0 --side left
python src/manual_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id> --updates 100
python src/foreground_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id>
python src/background_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id> --seconds 10
python src/async_runtime.py --port /dev/ttyUSB0 --side left --package-id <package-id> --samples 10
python src/debug_trace.py --port /dev/ttyUSB0 --side left --package-id <package-id> --detail full --sample-rate 10
python src/multiple_outputs.py --port /dev/ttyUSB0 --side left --package-id <package-id>
python src/device_lifecycle.py --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1 --updates 10
python src/haptics.py --port /dev/ttyUSB0 --side left --hold 1
python src/host_capture_calibration.py --package-id <package-id>
```

`async_runtime.py` 持续打印收到的目标关节输出；完成指定样本后停止并释放资源。后台运行提前失败时，示例输出错误并退出，不再等待下一条数据；输出处理失败时也会报告错误，包括后台恰好正常结束的情况。暂时没有数据时仍继续等待。按 `Ctrl+C` 取消运行时也会完成清理。

`time_sync.py` 使用后台运行模式，每隔 1 秒重新获取一次外部时间源的 Unix 毫秒时间戳并调用
`glove.sync_time(reference_time_ms)`。传入参数的单位是毫秒，不是纳秒；按 `Ctrl+C` 停止。

Windows 请将串口替换为实际的 `COM` 端口，例如 `--port COM3`。

`host_capture_calibration.py` 不传包参数时使用内置骨骼，无需另外安装算法包：

```bash
python src/host_capture_calibration.py
```

`host_capture_calibration.py` 可同时接收多个包，完成左右手采集后选择一个已配置 target，
保存并应用标定结果：

```bash
python src/host_capture_calibration.py \
  --package-id <package-id-1> \
  --package-id <package-id-2>
```

程序会分别选择左右手设备，并查询当前设备的采集计划是否已有兼容、可复用的已保存采集
数据。需要忽略这些可复用数据并重新采集时增加 `--force`。只标定 SDK 内置 Skeleton 时
使用固定 ID：

```bash
python src/host_capture_calibration.py \
  --package-id qnbot_hand.dynamic_openxr_hand
```

## 参数与配置

常用命令见上文。参数含义、订阅交付配置、骨长与算法参数配置见[Python SDK 配置参考](https://docs.qnbot.com/zh-CN/docs/vetra-glove-pro/sdk/python/latest/configuration-reference)。
需要目标手输出时，先安装配套算法包；缺少标定时按终端提示完成动作，完成后开始输出目标关节结果。

## 常见问题

- 无法安装 SDK：确认 Python 版本、平台和网络。
- 找不到手套：确认设备连接、串口未被占用，先运行 `discover_gloves.py`。
- 找不到算法包：通过 CLI 安装匹配的产品算法包，并检查 `--package-id`。
- 没有目标手输出：按终端提示完成标定；外部输入示例在标定期间持续推帧。
- 终端提示不可用：在交互式终端运行，或选择 `--capture-interaction external`。

## 有线和无线共用示例

USB 直连、Receiver 无线和左右手混合连接使用同一套配置。查看设备：

```bash
python src/discover_gloves.py
python src/device_info.py --side left
python src/device_info.py --side right
```

`device_info` 输出 SN、手别、型号、固件和 `transport_type`；`device_lifecycle` 演示两只手套独立启停，参数见上面的常用命令。

## 多路 IMU 与原始 EMF

IMU 示例以 `angular_velocity_rad_s` 显示 Helix 角速度（rad/s），以 `acceleration_mps2` 显示加速度（m/s²），普通手套输出原始计数；缺失向量与有效零值分开显示。示例同时订阅手部和腕部位姿，分别显示 fresh/held、`timestamp_ms` 和 `received_ms`，腕部 `position_m` 使用米、`quaternion_xyzw` 按 x/y/z/w 排列。`emf.py` 输出原始 EMF 矩阵和通道信息，不需要目标手算法包。

```bash
python src/imu.py --port /dev/your-glove --side left
python src/emf.py --port /dev/your-glove --side right
```

## 独立手势检查

安装 Glove 2.0.0 及后续正式版的配套 Core、Glove 包，连接手套或 Receiver 后运行：

```bash
python src/capture_quality.py --port /dev/your-glove --side left --stage open
```

`--stage` 支持 close、open、mcp90、mcp90_pip90、pinch_first、pinch_middle、pinch_ring、pinch_little，默认 open。按提示先完成 close，再完成所选动作；程序收齐 30 帧后输出 `accepted`、建议及中英文原因。
