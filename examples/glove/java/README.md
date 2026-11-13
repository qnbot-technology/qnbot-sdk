# Glove 手套 Java 示例

从 [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) 开始，在自己的 Android 项目中读取手套数据。每个用例是一份可独立复制的 Java 源码，文件内直接展示配置、SDK 创建、订阅、运行和清理；用例、输入和结果见下表。

## 准备

1. 取得支持统一 `Sdk` 入口的完整配套 Android Maven 交付，按交付清单校验并解压，保留完整 `maven/`。RC/Stable 也可从 [SDK 发布列表](https://github.com/qnbot-technology/qnbot-sdk/releases) 选择；版本以实际交付记录为准。统一入口的首个发布版本尚未确定，使用明确提供该入口的配套组件版本。
2. 准备 Android API 26 或以上的 `arm64-v8a` 运行环境。现有 Java SDK 需要真实 Android `Context`；当前不提供桌面 JVM 或 x86 运行包。
3. 在自己的项目中接入对应 SDK 组件并启用标准 core library desugaring。USB 输入先由自己的应用取得权限，准备支持的 `SerialTransport`；内置 `UsbSerialTransport` 支持 USB Host 上的单串口 CDC ACM 设备。

## 先读取数据

在自己的工作线程中调用 QuickStart，传入已有 `Context` 和已获授权的传输；SDK 自动发现唯一连接的手套：

```java
import com.qnbot.examples.QuickStart;

QuickStart.run(context, transport);
```

应持续看到 `pose sequence=` 递增，并带有拇指和食指位置。QuickStart 用 `runForever()` 阻塞工作线程，原样调用不提供停止句柄。需要在应用内主动停止时，复制源码并由应用持有 `Sdk`，再从其他线程关闭；原样运行可随进程退出结束。不要在 UI 线程直接调用。

需要主动停止的外部输入和采集标定用例使用 `cancelled` 停止信号；有限运行用例按 `seconds`、`updates` 或 `samples` 自行结束。

## 用例与参数

除 `device_selection` 外，每个方法都接收 `Context context` 和 `SerialTransport transport`。下表列出其余参数；具体类型和顺序见对应源码的 `run` 方法。端口均须完成所需 USB 授权。

| 用例 | Java 源码 | 其余参数 | 预期结果 |
| --- | --- | --- | --- |
| `glove/algorithm_configuration` | [AlgorithmConfiguration.java](src/main/java/com/qnbot/examples/AlgorithmConfiguration.java) | `port`、`side`、`packageId`、`boneConfigMode`、`algorithmConfigPath`、`seconds` | 使用骨长策略和算法参数读取目标输出 |
| `glove/async_runtime` | [AsyncRuntime.java](src/main/java/com/qnbot/examples/AsyncRuntime.java) | `port`、`side`、`packageId`、`samples` | 一次异步读取和连续流消费，处理停止和清理 |
| `glove/background_runtime` | [BackgroundRuntime.java](src/main/java/com/qnbot/examples/BackgroundRuntime.java) | `port`、`side`、`packageId`、`seconds` | 后台推进并持续读取目标输出 |
| `glove/debug_trace` | [DebugTrace.java](src/main/java/com/qnbot/examples/DebugTrace.java) | `port`、`side`、`packageId`、`sampleRate`、`detail` | 启用诊断记录并观察目标输出 |
| `glove/device_info` | [DeviceInfo.java](src/main/java/com/qnbot/examples/DeviceInfo.java) | `port`、`side` | 读取设备信息和连接类型 |
| `glove/device_lifecycle` | [DeviceLifecycle.java](src/main/java/com/qnbot/examples/DeviceLifecycle.java) | `leftPort`、`rightPort`、`updates` | 独立停止一台设备，另一台继续输出 |
| `glove/device_selection` | [DeviceSelection.java](src/main/java/com/qnbot/examples/DeviceSelection.java) | 无额外参数 | 按名称、左右手选择来源并观察预期歧义 |
| `glove/discover_gloves` | [DiscoverGloves.java](src/main/java/com/qnbot/examples/DiscoverGloves.java) | 无额外参数 | 通过宿主传输探测手套，显示候选及逐设备错误 |
| `glove/external_input_manual_update` | [ExternalInputManualUpdate.java](src/main/java/com/qnbot/examples/ExternalInputManualUpdate.java) | `side`、`packageId`、`captureConfig`、`frames`、`confirm`、`cancelled` | 由应用提交外部帧并手动推进 |
| `glove/external_input_run_background` | [ExternalInputRunBackground.java](src/main/java/com/qnbot/examples/ExternalInputRunBackground.java) | `side`、`packageId`、`captureConfig`、`frames`、`confirm`、`cancelled` | 由应用提交外部帧，SDK 后台推进 |
| `glove/foreground_runtime` | [ForegroundRuntime.java](src/main/java/com/qnbot/examples/ForegroundRuntime.java) | `port`、`side`、`packageId` | 在调用线程持续推进并读取目标输出 |
| `glove/haptics` | [Haptics.java](src/main/java/com/qnbot/examples/Haptics.java) | `port`、`side`、`holdSeconds` | 设置触觉并清除 |
| `glove/host_capture_calibration` | [HostCaptureCalibration.java](src/main/java/com/qnbot/examples/HostCaptureCalibration.java) | `leftPort`、`rightPort`、`packageIds`、`selectedPackageId`、`force`、`confirm`、`cancelled` | 显式完成左右手采集与标定并读取新输出 |
| `glove/emf` | [Emf.java](src/main/java/com/qnbot/examples/Emf.java) | `port`、`side` | 读取支持设备的原始 EMF 整数矩阵 |
| `glove/imu` | [Imu.java](src/main/java/com/qnbot/examples/Imu.java) | `port`、`side` | 读取手套的带单位 IMU 数据 |
| `glove/manual_runtime` | [ManualRuntime.java](src/main/java/com/qnbot/examples/ManualRuntime.java) | `port`、`side`、`packageId`、`updates` | 由应用控制每次更新 |
| `glove/pressure` | [Pressure.java](src/main/java/com/qnbot/examples/Pressure.java) | `port` | 读取 460 个原始压感值和 kPa 估计（仅 HMP-MX 提供） |
| `glove/multiple_outputs` | [MultipleOutputs.java](src/main/java/com/qnbot/examples/MultipleOutputs.java) | `port`、`side`、`packageId` | 同一来源产生两个独立命名的目标输出 |
| `glove/quick_start` | [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) | 无额外参数 | 自动发现唯一手套并观察持续姿态 |
| `glove/skeleton` | [Skeleton.java](src/main/java/com/qnbot/examples/Skeleton.java) | `port`、`side` | 观察骨骼关节与手腕坐标 |
| `glove/subscription_config` | [SubscriptionConfig.java](src/main/java/com/qnbot/examples/SubscriptionConfig.java) | `port`、`side`、`seconds`、`subscription` | 比较 Latest 与 FIFO 订阅及回调频率 |
| `glove/time_sync` | [TimeSync.java](src/main/java/com/qnbot/examples/TimeSync.java) | `referenceTimeMillis`（可省略） | 刷新外部时间并观察授时后的时间戳 |

`port` 传实际串口标识。`device_info`、`device_lifecycle` 和 `host_capture_calibration` 可传 `null` 自动发现已授权的唯一设备；其他需要端口的用例必须显式提供。`quick_start` 和 `time_sync` 不接收端口，始终自动发现唯一手套。Glove 左右手可以使用同一 Receiver 端口。`seconds` 是正的运行秒数，`updates` 是正的更新次数，`samples` 是正的样本数量。`subscription` 传 `SubscriptionConfig.latest(maxRateHz)` 或 `SubscriptionConfig.fifo(capacity, maxRateHz)`。`debug_trace` 用 `sampleRate` 和 `detail` 选择诊断采样与详细程度。`haptics` 保持拇指 80、食指 40，持续 `holdSeconds` 秒后清除。

`cancelled` 是非阻塞、线程安全的 `BooleanSupplier`，只用于需要主动停止的外部输入和采集标定用例。停止按钮、页面销毁和进程退出把结果设为 `true`；示例在下一轮检查后结束并关闭资源。`quick_start`、`skeleton`、`imu`、`debug_trace`、`multiple_outputs`、`foreground_runtime` 和 `pressure` 使用 `runForever()`，不需要停止回调；`async_runtime` 用中断取消等待。

`time_sync` 默认使用宿主时钟；传入 `referenceTimeMillis` 可保留外部 Unix 毫秒时钟。它在后台持续运行并每秒授时一次；中断工作线程后停止、等待并关闭 SDK。

## 算法与采集

算法用例的 `packageId` 使用目标应用中已准备、与 Android SDK 匹配的真实算法包。通过产品交付说明中的 QnBot CLI 准备算法；桌面算法安装不代表 Android 应用已经具备运行条件。

普通运行、配置、调试和骨骼用例使用 SDK 默认的采集策略，不自行处理采集提示。算法示例假定产品要求的采集与标定已经完成；首次准备请使用 `host_capture_calibration`，把 `confirm` 接到应用 UI，或按产品交付流程先完成算法准备。`host_capture_calibration` 先完成左右手采集，再选择目标并执行标定，是这个显式流程的完整示例。`packageIds` 为空时使用内置 Skeleton；多个包时用 `selectedPackageId` 选定一个，`force` 请求重新采集。`cancelled` 停止当前采集或标定，取消以 `CancellationException` 报告。

两个 `external_input` 示例由应用提供帧，因此也由应用决定采集交互。`captureConfig` 使用已有 `CaptureConfig`：`new CaptureConfig()` 保留 SDK 的 `AUTO_TERMINAL` 交互；普通 Android App 没有终端输入时显式选择 `EXTERNAL`，并填写所选 `operatorId`。`confirm` 是非阻塞 `Predicate<String>`：展示提示并读取已记录的 UI 输入，已确认返回 `true`，尚未输入返回 `false`，不要等待用户。`AUTO_TERMINAL` 不调用 `confirm`。`frames` 传 `null` 时使用文件内的合成五指帧；需要真实输入时传非阻塞 `Supplier<ExternalGloveFrame>`，返回五指帧或在输入结束时返回 `null`，时间戳每帧增加 10 毫秒。

`algorithm_configuration` 的 `boneConfigMode` 使用 `AUTO`、`ENABLED` 或 `DISABLED`；`algorithmConfigPath` 传算法支持的可读配置文件路径，不指定时传 `null`。需要先准备骨长或导入算法时，仍只复制这个文件，在工作线程调用其中的 `importAlgorithmArchive`、`importBoneConfig` 或 `prepareBoneConfig`，再调用 `run`。`prepareBoneConfig` 的链接展示回调必须立即返回；会话过期、失败和取消通过异常报告。

`time_sync` 的 `referenceTimeMillis` 每次返回新的外部 Unix 毫秒时间；省略时使用 `System::currentTimeMillis`。输出 `reference_time_ms`、`source_time_ms` 和 `observed_at_ms`；源时间与观察时间基准不同，不能直接相减。需要包含 `Glove.syncTime` 的匹配 Core/Glove 交付；首个包含该能力的发布版本尚未确定。

### 触觉ENF定位手套（VetraGlove HMP-MX）

使用支持触觉ENF定位手套的配套 Android SDK。连接按出厂设置主动上报压感的 VetraGlove HMP-MX 手套，传入实际串口，在工作线程调用 `Pressure.run(context, transport, port)`。手侧由设备识别。

**460 点压感是 HMP-MX 独有的通道：只有这只手套提供 `device.pressure()`，Nano、Pro 和 Receiver 手套没有该通道。**

示例只读取压感，每秒最多显示一帧 `pressure`。`points=460` 是测点数；`raw_min`、`raw_mean`、`raw_max` 汇总全部测点的原始计数（0–255），`peak_index` 是最大原始值首次出现的索引。`point_index`、`point_raw`、`point_estimated_kpa` 是选中测点的索引、原始计数和同一帧的 kPa 估计，默认选择峰值点。调用 `Pressure.run(context, transport, port, 459)` 可持续观察第 459 点；原有三参数调用继续选择峰值点。索引范围为 0–459，按设备上报顺序排列，按下表定位区域。

`point_row`、`point_column` 分别为 `point_index / 20` 向下取整和 `point_index % 20`。行号 0–22、列号 0–19；映射表中第 0 行在下方，第 0 列在左侧。这是索引网格，不是毫米坐标。按实际手套侧别查表：

| 区域 | 行 | 右手列 | 左手列 |
| --- | --- | --- | --- |
| 小指 | 17–22 | 0–3 | 16–19 |
| 无名指 | 17–22 | 4–7 | 12–15 |
| 中指 | 17–22 | 8–11 | 8–11 |
| 食指 | 17–22 | 12–15 | 4–7 |
| 拇指 | 17–22 | 16–19 | 0–3 |
| 掌心 | 0–14 | 0–15 | 4–19 |

五指各 24 点，掌心 240 点。第 15–16 行及掌心行之外的 4 列共 100 点在映射表中未分区；示例仍保留并汇总这些值，不把它们当作无效数据或零值。例如索引 459 为第 22 行第 19 列，对应右手拇指或左手小指。

kPa 是依据手掌汇总气压测量得到的估计，未逐点标定，不是力（N），也没有已验证的逐点精度。原始值 107–255 的估计均截顶到约 40.01 kPa，`capped_points` 是落在这个截顶区间的测点数；它不表示传感器已物理饱和或实际压力等于 40.01 kPa。示例保留原始值，kPa 仅显示一位小数。

启动后先显示等待提示。启动完成后约 5 秒仍没有首帧压感时，调用抛出异常并关闭资源；检查 HMP-MX 型号、串口和设备是否上报压感。460 点全零仍是有效帧。收到首帧后持续运行，缺少压感块时不生成新帧；1 Hz 显示可能跳过短促按压，观察时保持按压片刻。与其他持续示例一样，由调用方持有并关闭 SDK 才能支持应用内主动停止。姿态、IMU 和 Skeleton 使用对应的通用示例。

## 排错

- 找不到依赖：确认 `sdkMavenDir` 指向完整 `maven/`，使用真实且匹配的组件版本，保留公共依赖仓库。
- USB 无法打开：检查权限、USB Host、线缆及单串口 CDC ACM 支持；设备选择和外部帧用例无需 USB。
- Helix USB CDC：使用支持 Helix 的配套 SDK 时，QuickStart 自动发现唯一手套，DeviceInfo 可显式传入 Helix 的端口和手别。DeviceInfo 显示设备序列号、型号、固件版本、手别和连接类型。Helix 的 `imu()` 通道直接输出多路带单位数据。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并先运行 `discover_gloves`。
- 没有目标输出：确认已按产品流程准备算法和采集/标定；普通示例不再代替应用确认采集提示。
- 查看 SDK 日志：`adb logcat -s qnbot-sdk:I`。保留用例、输入条件和完整错误信息以便排查。

路径和命令变化见 [变更记录](../../../CHANGELOG.md)。

## 原始 EMF

使用配套新版 Core、Glove 包调用 `Emf.run(context, transport, port, side)`。程序输出 session、实际存在的帧序号或 sync、通道及四组 3×3 原始整数矩阵，单位为 demodulated_integral。SDK 自动准备配套采集，无需设置频率或加载目标手算法包。没有数据时等待首条观测；设备不支持时返回能力错误。
