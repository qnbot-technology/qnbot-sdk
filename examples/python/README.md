# QnBot SDK Python 示例

本目录提供可以直接运行的 Python 示例，覆盖快速启动、设备发现、retargeting、
calibration、Debug Trace 和触觉反馈。

`src/` 中只包含可独立运行的示例源码，不包含 SDK 实现源码，也不是可安装的 Python
package。示例通过公开发布的 `qnbot_sdk` package 运行。

## 5 分钟快速开始

### 1. 安装 SDK

从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases)
下载相互兼容的 QnBot Core 和 Glove Python wheel。将当前平台对应的
`qnbot-sdk-core` wheel 和 `qnbot-sdk-glove` wheel 放入
`wheelhouse/`，然后直接从本地交付包创建环境：

```bash
uv venv --python 3.10
uv pip install \
  --python .venv/bin/python \
  --no-index \
  --find-links wheelhouse \
  --prerelease=allow \
  -r requirements.txt
```

`requirements.txt` 不固定公开版本；`wheelhouse/` 中的 Release 资产决定安装的
稳定版或 RC 版本，并由 Glove wheel 的依赖元数据选择兼容的 Core 版本。这样不会把
内部 dev 版本或单一操作系统的 wheel 写入公开仓库。Windows 用户可将
`--python` 参数改为 `.venv\\Scripts\\python.exe`。Git 会忽略本地虚拟环境、wheel、
缓存、trace 和采集数据。

### 2. 安装 retargeting 算法包

仅 macOS arm64 交付的 SDK 内置动态 OpenXR 骨骼由 `skeleton()` 直接提供，不需要
另行安装算法包。Linux 和 Windows 当前不携带该私有 runtime，选择 `skeleton()` 会
明确报告能力不可用；`pose()` 和客户配置的 `output()` 仍可使用。
Core wheel 不包含具体机器人的 retargeting 算法。运行使用 `TargetAlgorithm` 的示例前，
需要安装目标手配套的算法包。SDK 会从 `QNBOT_ALGORITHM_PATH` 和各平台默认的 QnBot
算法目录发现软件包。算法包位于其他位置时，将环境变量指向包根目录或其父目录：

```bash
export QNBOT_ALGORITHM_PATH=/opt/qnbot/algorithms
```

常规单 target 示例默认使用 target `openxr_hand` 和算法
`qnbot.hand.openxr_hand.retargeting`。带命令行参数的 runtime 示例接受
`--target-name` 用于命名输出实例，`--algorithm-id` 用于选择 SDK 已发现的兼容算法；
这些参数不会安装算法包，也不会改变发现路径。

`TargetConfig.name` 和 `--target-name` 是调用方定义的 output 实例名，用于组成公开
target ID，并让 `output(name=...)` 按实例名选择对应 channel。算法 ID 和可选版本负责选择算法；
算法包
自己的包内资源名称只供 native 实现使用，不会生成或限制公开实例名。

`quick_start.py` 有意保持精简：它选择
`qnbot.hand.glove_to_hand.retargeting`，并让 SDK 从已安装算法包中推导唯一 target
类型、side 和 source；省略的公开实例名保持省略。

手部算法包使用 Python 和 C++ SDK wrapper 均可加载的动态 C ABI 交付格式。算法包的
动态 C ABI manifest 标识算法 ID，发布资源中的包内资源供 native 实现使用。这种共享
交付格式独立于安装 SDK binding 的 Python wheel，包内资源不会成为公开 target 实例名。

### 3. 运行 Quick Start

连接一只受支持的手套，然后运行：

```bash
python src/quick_start.py
```

`quick_start.py` 使用单手套自动配置：

```python
Sdk(
    devices=(GloveConfig(),),
    targets=(
        TargetConfig(
            algorithms=(TargetAlgorithm("qnbot.hand.glove_to_hand.retargeting"),),
        ),
    ),
)
```

`quick_start.py` 不显式配置 `AlgorithmsConfig.calibration`，因此 SDK 使用默认的按需
终端标定、默认 operator 和兼容的已有记录。首次运行缺少兼容记录时，按照终端提示完成
calibration，随后开始输出 retargeting 前的 pose 和 retargeting 后的 output。

## 选择示例

| 目标 | 示例 | 算法包 | 主要数据 | 主要 API |
| --- | --- | --- | --- | --- |
| 配置前发现已连接手套 | `discover_gloves.py` | 否 | Port、hand、SN | `discover_gloves()`、`GloveConfig`、`SerialConnection(port=...)` |
| 无硬件输入姿态并获得 pass-through 输出 | `external_input.py` | 否 | Pose + pass-through output | `ExternalConnection`、`push_frame()`、`subscribe()` |
| 读取动态 OpenXR 手部骨骼 | `skeleton.py` | 内置 | Skeleton | `skeleton()`、`subscribe()` |
| 将一只真实手套 retarget 到目标手 | `quick_start.py` | 是 | Pose + output | `TargetAlgorithm`、`output()` |
| 记录真实手套输入与 retargeting 输出 | `debug_trace.py` | 是 | Pose + output | `DebugConfig`、`TargetAlgorithm`、`output()` |
| 将 retargeting 输出接入应用循环 | `manual_runtime.py` | 是 | Output | `output()`、`latest()`、`update()` |
| 在当前线程消费 retargeting 输出 | `foreground_runtime.py` | 是 | Output | `output()`、`subscribe()`、`run_forever()` |
| 在后台消费 retargeting 输出 | `background_runtime.py` | 是 | Output | `output()`、`subscribe()`、`run_background()` |
| 使用 asyncio 消费 retargeting 输出 | `async_runtime.py` | 是 | Output | `output()`、`await next()`、`async for` |
| 控制手套振动 | `haptics.py` | 否 | Haptics state | `haptics()`、`set()`、`latest()`、`clear()` |
| 一只手套驱动多个 target | `multiple_outputs.py` | 是 | 多路 output | `run_forever()`、`output(name)`、`subscribe()` |
| 独立控制每只手套并读取 output | `device_lifecycle.py` | 是 | 每个 device 的 output | `device(name)`、`device(side)`、`output()`、per-device lifecycle |

以下示例需要 SDK 已发现算法包，并消费真实 retargeting `HandJointCommand` 输出：
`quick_start.py`、`debug_trace.py`、`manual_runtime.py`、`foreground_runtime.py`、
`background_runtime.py`、`async_runtime.py`、`multiple_outputs.py` 和
`device_lifecycle.py`。

`discover_gloves.py`、`external_input.py`、`skeleton.py` 和 `haptics.py` 不需要客户安装算法包。
`external_input.py` 为无硬件链路检查提供
pass-through 输出，不是真实 retargeting。

`pose()` 是手套原始姿态，`skeleton()` 是 SDK 内置的动态 OpenXR 骨骼并输出 24 个
active joints，`output()` 是客户通过 target 配置的 retargeting 输出。需要操作内置骨骼的标定流程时，沿用
`calibration_progress(name=TargetName.SKELETON)` 和
`calibration_control(name=TargetName.SKELETON)`；普通 target 仍传入自己的名称。
当前 skeleton 示例仅在内置 runtime 可用的 macOS arm64 交付包上运行。

部分示例有意演示多设备或多 output 实例部署：

- `device_lifecycle.py` 需要两只不同的串口手套，并拒绝重复绑定同一端口。
- `multiple_outputs.py` 使用同一个兼容算法 ID 创建 `primary` 和 `backup` 两个实例；两者
  复用同一个包内资源，但分别产生独立 output channel。

## 运行命令

```bash
python src/discover_gloves.py
python src/external_input.py
python src/skeleton.py
python src/quick_start.py
python src/debug_trace.py \
  --port /dev/ttyUSB0 --side left --detail full --sample-rate 10
python src/manual_runtime.py \
  --port /dev/ttyUSB0 --side left --updates 100
python src/foreground_runtime.py --port /dev/ttyUSB0 --side left
python src/background_runtime.py \
  --port /dev/ttyUSB0 --side left --seconds 10
python src/async_runtime.py \
  --port /dev/ttyUSB0 --side left --samples 10
python src/haptics.py --port /dev/ttyUSB0 --side left --hold 1
python src/multiple_outputs.py \
  --port /dev/ttyUSB0 \
  --side left \
  --algorithm-id qnbot.hand.glove_to_hand.retargeting
python src/device_lifecycle.py \
  --left-port /dev/ttyUSB0 \
  --right-port /dev/ttyUSB1 \
  --updates 10
```

前台示例持续运行到用户按下 Ctrl+C；专用 background 和 async 示例根据命令行参数
停止。

`foreground_runtime.py` 和 `background_runtime.py` 会在运行期间打印
`running health`，并在主动停止完成后打印 `stopped lifecycle health`。停止遥测后设备会
进入 `stale=true`，因此即使 `warning_count=0`、`error_count=0`，共享健康值也可能是
`ok=false`；这是预期的停止生命周期状态，不表示运行失败。判断运行期是否健康应查看
停止请求前的 `running health`，停止后的输出用于确认设备的 `connected`/`stale` 状态。

上述单 target retargeting 命令默认使用 OpenXR。仅当兼容算法包已安装在 SDK 可发现的
目录中时，才需要为带参数的 runtime 示例添加类似
`--target-name primary --algorithm-id qnbot.hand.glove_to_hand.retargeting` 的参数。
`quick_start.py` 保持无参数；适配其他交付 target 时直接修改文件中的常量。

`external_input.py` 是可无硬件运行的示例。它的 target 没有算法链，因此 output 明确为
pass-through，不是真实 retargeting 结果。串口和 haptics 示例需要连接手套。

## 自动配置与固定部署

### 单手套与双手套自动发现

空的串口连接表示自动发现。单个未指定 side 的 `GloveConfig()` 仍要求只有一个未占用
的受支持设备。一只左手套和一只右手套可以同时省略 `connection`：

```python
devices = (
    GloveConfig(side=Side.LEFT),
    GloveConfig(side=Side.RIGHT),
)
```

SDK 会一次探测全部候选，并按 side 唯一分配；串口枚举顺序不会决定左右数据流。每个
物理设备最多分配一次。缺少某一侧、同一侧存在多个候选，或发现后正式连接时 SN/side
发生变化，都会导致整体连接失败，不发布部分 device、calibration route 或
retargeting route。需要区分两只同侧手套时，应显式配置不同的 `port` 或
`serial_number`。

### 连接与身份确定

调用 `connect()` 时，SDK 读取 `DeviceInfo` 并固定每只手套的 serial number 和 side。
所选算法的 manifest 与交付资源决定 target type 和 side，唯一启用且兼容的手套决定
source；公开实例名不会从包内资源推导。仅在候选唯一时推导其他省略值；如果 device、
source、算法版本或包内 target resource 为零个或多个，错误信息会指出缺少的约束。

`Sdk(...)` 只校验并保存声明，不扫描或打开硬件。选择 device、channel、subscription、
calibration channel 或 output 前，需要先同步调用 `glove.connect()`：

```python
sdk = create_sdk()
glove = sdk.glove()
glove.connect()
device = glove.device()
subscription = device.pose().subscribe(on_pose)
```

连接过程以原子方式确定并发布完整 runtime。如果 discovery、身份校验、target 推导、
route 构建或 runtime 连接失败，不会暴露部分 device 或 route。

连接后，各层使用同一组 canonical ID。例如右手手套上报
`canonical_sn="QN-R-000042"` 时，source ID 为 `glove.QN-R-000042.right`；未配置
实例名的 target ID 为 `hand.~.right`。配置中的 glove `name` 仍是 facade
选择别名；日志、诊断、topic、calibration 记录和公开 runtime API 使用 canonical ID，
不再区分逻辑 ID 与 execution ID。

### 显式固定部署

固定部署可设置 `SerialConnection(port=...)` 或
`SerialConnection(serial_number=...)`，并按需提供 `side`、`source`、target `type`
或 target `name`。显式值用于缩小并校验候选，不能覆盖设备上报的物理 side。当 side
能够唯一标识已连接手套时，多手套配置也可以省略 connection。`ExternalConnection`
仍需显式配置，因为 external input 没有身份握手，所以必须提供 glove side。

所有接受显式 `--port` 的单手套命令都同时要求 `--side left|right`。应使用物理手套
上报的 side，不提供默认值；静默假设错误 side 可能成功打开串口，但全部 telemetry 都会
因 side 不匹配而被拒绝。`quick_start.py` 和 `discover_gloves.py` 保持自动发现，不接受
`--side`。

需要确定性部署时，可以显式提供全部约束：

```python
Sdk(
    devices=(
        GloveConfig(
            name="primary",
            side=Side.RIGHT,
            connection=SerialConnection(serial_number="GLOVE-001"),
        ),
    ),
    targets=(
        TargetConfig(
            type=TargetType.HAND,
            name="wuji_hand",
            side=Side.RIGHT,
            source=DeviceSelector(type="glove", name="primary", side=Side.RIGHT),
            algorithms=(
                TargetAlgorithm(
                    "qnbot.hand.glove_to_hand.retargeting",
                    version="1.0.0",
                ),
            ),
        ),
    ),
)
```

如果端口是稳定的部署定位符，可改用 `SerialConnection(port="/dev/ttyUSB0")`。

## Calibration

算法首次运行时，可能需要完成引导式 calibration 才会产生关节命令。常规配置使用
`AlgorithmsConfig` 内嵌的 `calibration=CalibrationConfig()` 及其默认
`auto_terminal` 交互。依次调用 `connect()`、`start()` 和 `run_forever()` 后，Rust
终端驱动会在交互式终端显示算法包提供的各阶段提示、等待确认并报告采样进度。Python
示例不自行实现终端输入或标定流程。calibration 完成后才会产生关节输出；后续运行会
复用存储目录中的兼容记录。

算法示例将可复用的 calibration 记录保存在平台用户数据目录：macOS 为
`~/Library/Application Support/QnBot/calibrations`，Linux 为
`${XDG_DATA_HOME:-~/.local/share}/qnbot/calibrations`，Windows 为
`%LOCALAPPDATA%\QnBot\calibrations`。设置 `calibration.store.path` 可指定其他目录。

`operator_id` 默认为 `"default"`，用于隔离不同 operator 的永久记录。设置 `force=True`
会忽略创建当前 SDK 实例之前已有的记录，并对每条必要 route 强制标定一次。强制标定成功
后，当前 SDK 实例在 stop 或 reconnect 后仍复用新结果；另一个设置 `force=True` 的 SDK
实例仍会再次强制标定。

只有应用需要非默认存储目录、external 交互、强制重新标定或其他 operator ID 时，才需要
显式配置 calibration。

### External calibration control

GUI、ROS2 bridge 或远程应用应选择 `external` 交互，并直接映射 typed channel：

```python
from qnbot_sdk import (
    AlgorithmsConfig,
    CalibrationConfig,
    CalibrationInteractionMode,
)

sdk = Sdk(
    devices=(device_config,),
    targets=(target_config,),
    algorithms=AlgorithmsConfig(
        calibration=CalibrationConfig(
            interaction=CalibrationInteractionMode.EXTERNAL,
        ),
    ),
)
glove = sdk.glove()
glove.connect()
device = glove.device()
progress = device.calibration_progress(name="openxr_hand")
control = device.calibration_control(name="openxr_hand")
```

宿主 UI 渲染 `progress`，并将当前 `request_id` 原样传给 `control.confirm()`、
`control.skip()` 或 `control.cancel()`。Rust 负责校验请求以及阶段顺序、采样、复用、持久化
和激活；wrapper 不持有输入循环。

外部控制的有效操作取决于当前状态：

| Progress 状态 | 有效 UI 操作 |
| --- | --- |
| `awaiting_confirmation` | 在 `confirm` 与可选阶段的 `skip` 中二选一，也可以 `cancel` |
| `collecting` | 保留当前 `request_id`，只提供 `cancel` |
| 最终阶段带 request ID 的 `persisting` 或 `activating` | 保留同一 `request_id`，只提供 `cancel` |

`confirm` 和 `skip` 是等待状态下互斥的选择。调用 `confirm` 后，采集期间以及最终阶段的
持久化与激活期间仍保持同一请求，以便用户继续取消。

## 设备发现

### 创建配置前发现手套

`discover_gloves()` 是不需要 `Sdk` 实例的同步快照。它返回所有已连接且受支持的候选，
包括端口被其他进程占用的设备。这类记录仍保留 port，并带有 typed `PORT_BUSY` 问题；
无法读取的身份字段为 `None`。程序逻辑应使用 `error.code`，
`error.message` 只用于显示诊断信息。

示例选择一条完整记录，并使用已校验的 hand 和显式串口构建配置：

```python
from qnbot_sdk import SerialConnection
from qnbot_sdk.glove import GloveConfig, discover_gloves

selected = next(device for device in discover_gloves() if device.error is None)
config = GloveConfig(
    side=selected.hand,
    connection=SerialConnection(port=selected.port),
)
```

返回的 tuple 是调用时刻的快照，因此设备接入或移除后应重新发现。

## 生命周期与 Debug 排障

### 运行与资源清理

常规示例配置一只手套，只在需要 source-bound channel 时调用 `glove.device()`。只有
`device_lifecycle.py` 会按显式 name 或 side 选择，因为它需要独立控制两只物理手套。

常规示例默认使用 `run_forever()`。后台执行由 `background_runtime.py` 单独演示，
`async_runtime.py` 也使用后台执行以保证 asyncio event loop 可继续运行。只有
`manual_runtime.py` 使用手动 update，用于演示如何接入应用自有循环。

所有脚本都位于 `src/`。每个示例都可以独立运行，并且只使用公开的 `qnbot_sdk`
API。Glove domain 示例在 `finally` 中关闭 `glove`；`glove.close()` 会释放所有剩余的
Glove subscription，因此这些示例不会在关闭前逐个 unsubscribe。应用需要在 domain
保持打开时停止单个 callback，才应调用 `subscription.unsubscribe()`。

### Debug Trace

`debug_trace.py` 默认在 QnBot 的平台 user-data 日志目录下，为每个启用模块写入一个
只追加的 JSONL 文件：

| 平台 | 默认目录 |
| --- | --- |
| macOS | `~/Library/Application Support/QnBot/logs` |
| Linux | `${XDG_DATA_HOME:-~/.local/share}/qnbot/logs` |
| Windows | `%LOCALAPPDATA%\QnBot\logs` |

Linux 仅在 `XDG_DATA_HOME` 为绝对路径时使用它，否则回退到
`~/.local/share/qnbot/logs`。使用 `--log-dir` 可以选择其他目录。该示例启用支持的
`serial`、`glove`、`calibration`、`retargeting`、`output` 和 `buffer` 六个模块。

使用 `--detail` 选择足以回答当前问题的最小诊断数据量：

| Detail | 保留数据 | 主要用途 |
| --- | --- | --- |
| `summary`（默认） | ID、sequence/timing、数量、质量状态和简明错误 | 健康巡检和快速定位故障层 |
| `full` | summary 的全部内容，加解码后的手套姿态、calibration 选择/进度、retargeting 输入/结果和 output 命令 | 已确认串口解码正常后分析 calibration 与 retargeting |
| `raw` | full 的全部内容，加串口字节、已接受的原始帧和完整 buffer 值 | 复现串口、分帧、协议和解码问题 |

三个级别逐级包含：`summary` 是 `full` 的子集，`full` 是 `raw` 的子集。warning 和
error 级记录不受常规采样限制，并在诊断畸形或被拒绝输入时保留有界的原始证据。正常
串口 chunk 和已接受的原始帧只出现在 `raw`。
