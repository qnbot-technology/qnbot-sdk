# Glove 手套 Java 示例

从 [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) 开始，在自己的 Android 项目中读取手套数据。每个用例是一份可独立复制的 Java 源码，文件内直接展示配置、SDK 创建、订阅、运行和清理；用例、输入和结果见下表。

## 准备

1. 取得支持统一 `Sdk` 入口的完整配套 Android Maven 交付，按交付清单校验并解压，保留完整 `maven/`。RC/Stable 也可从 [SDK 发布列表](https://github.com/qnbot-technology/qnbot-sdk/releases) 选择；版本以实际交付记录为准。统一入口的首个发布版本尚未确定，使用明确提供该入口的配套组件版本。
2. 准备 Android API 26 或以上的 `arm64-v8a` 运行环境。现有 Java SDK 需要真实 Android `Context`；当前不提供桌面 JVM 或 x86 运行包。
3. 在自己的项目中接入对应 SDK 组件并启用标准 core library desugaring。USB 输入先由自己的应用取得权限，准备支持的 `SerialTransport`；内置 `UsbSerialTransport` 支持 USB Host 上的单串口 CDC ACM 设备。

## 先读取数据

在自己的工作线程中调用 QuickStart，传入已有 `Context` 和已获授权的传输。`port` 使用实际端口标识；传 `null` 自动发现唯一设备：

```java
import com.qnbot.examples.QuickStart;
import com.qnbot.sdk.config.Side;
import java.util.concurrent.atomic.AtomicBoolean;

AtomicBoolean stop = new AtomicBoolean();
QuickStart.run(context, transport, port, Side.RIGHT, stop::get);
```

应持续看到 `pose sequence=` 递增，并带有拇指和食指位置。QuickStart 会阻塞到停止信号生效，结束后自行关闭设备。把它放在工作线程，并把停止按钮、页面销毁和进程退出接到 `stop.set(true)`；不要在 UI 线程直接调用。

其他持续读取用例使用同一个停止信号接法；有限运行用例按 `seconds`、`updates` 或 `samples` 自行结束。

## 用例与参数

除 `device_selection` 外，每个方法都接收 `Context context` 和 `SerialTransport transport`。下表列出其余参数；具体类型和顺序见对应源码的 `run` 方法。端口均须完成所需 USB 授权。

| 用例 | Java 源码 | 其余参数 | 预期结果 |
| --- | --- | --- | --- |
| `glove/algorithm_configuration` | [AlgorithmConfiguration.java](src/main/java/com/qnbot/examples/AlgorithmConfiguration.java) | `port`、`side`、`packageId`、`boneConfigMode`、`algorithmConfigPath`、`seconds` | 使用骨长策略和算法参数读取目标输出 |
| `glove/async_runtime` | [AsyncRuntime.java](src/main/java/com/qnbot/examples/AsyncRuntime.java) | `port`、`side`、`packageId`、`samples`、`cancelled` | 一次异步读取和连续流消费，处理停止和清理 |
| `glove/background_runtime` | [BackgroundRuntime.java](src/main/java/com/qnbot/examples/BackgroundRuntime.java) | `port`、`side`、`packageId`、`seconds` | 后台推进并持续读取目标输出 |
| `glove/debug_trace` | [DebugTrace.java](src/main/java/com/qnbot/examples/DebugTrace.java) | `port`、`side`、`packageId`、`sampleRate`、`detail`、`cancelled` | 启用诊断记录并观察目标输出 |
| `glove/device_info` | [DeviceInfo.java](src/main/java/com/qnbot/examples/DeviceInfo.java) | `port`、`side` | 读取设备信息和连接类型 |
| `glove/device_lifecycle` | [DeviceLifecycle.java](src/main/java/com/qnbot/examples/DeviceLifecycle.java) | `leftPort`、`rightPort`、`updates` | 独立停止一台设备，另一台继续输出 |
| `glove/device_selection` | [DeviceSelection.java](src/main/java/com/qnbot/examples/DeviceSelection.java) | 无额外参数 | 按名称、左右手选择来源并观察预期歧义 |
| `glove/discover_gloves` | [DiscoverGloves.java](src/main/java/com/qnbot/examples/DiscoverGloves.java) | 无额外参数 | 通过宿主传输探测手套，显示候选及逐设备错误 |
| `glove/external_input_manual_update` | [ExternalInputManualUpdate.java](src/main/java/com/qnbot/examples/ExternalInputManualUpdate.java) | `side`、`packageId`、`captureConfig`、`frames`、`confirm`、`cancelled` | 由应用提交外部帧并手动推进 |
| `glove/external_input_run_background` | [ExternalInputRunBackground.java](src/main/java/com/qnbot/examples/ExternalInputRunBackground.java) | `side`、`packageId`、`captureConfig`、`frames`、`confirm`、`cancelled` | 由应用提交外部帧，SDK 后台推进 |
| `glove/foreground_runtime` | [ForegroundRuntime.java](src/main/java/com/qnbot/examples/ForegroundRuntime.java) | `port`、`side`、`packageId`、`cancelled` | 在调用线程持续推进并读取目标输出 |
| `glove/haptics` | [Haptics.java](src/main/java/com/qnbot/examples/Haptics.java) | `port`、`side`、`holdSeconds` | 设置触觉并清除 |
| `glove/host_capture_calibration` | [HostCaptureCalibration.java](src/main/java/com/qnbot/examples/HostCaptureCalibration.java) | `leftPort`、`rightPort`、`packageIds`、`selectedPackageId`、`force`、`confirm`、`cancelled` | 显式完成左右手采集与标定并读取新输出 |
| `glove/imu` | [Imu.java](src/main/java/com/qnbot/examples/Imu.java) | `port`、`side`、`cancelled` | 读取受支持的 IMU 原始计数 |
| `glove/manual_runtime` | [ManualRuntime.java](src/main/java/com/qnbot/examples/ManualRuntime.java) | `port`、`side`、`packageId`、`updates` | 由应用控制每次更新 |
| `glove/moxian/telemetry` | [MoxianTelemetry.java](src/main/java/com/qnbot/examples/MoxianTelemetry.java) | `port`、`cancelled` | 读取位姿、IMU、关节角、腕部坐标系下的指尖位姿和 460 个原始压感值 |
| `glove/multiple_outputs` | [MultipleOutputs.java](src/main/java/com/qnbot/examples/MultipleOutputs.java) | `port`、`side`、`packageId`、`cancelled` | 同一来源产生两个独立命名的目标输出 |
| `glove/quick_start` | [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) | `port`、`side`、`cancelled` | 连接设备并观察持续姿态 |
| `glove/skeleton` | [Skeleton.java](src/main/java/com/qnbot/examples/Skeleton.java) | `port`、`side`、`cancelled` | 观察骨骼关节与手腕坐标 |
| `glove/subscription_config` | [SubscriptionConfig.java](src/main/java/com/qnbot/examples/SubscriptionConfig.java) | `port`、`side`、`seconds`、`subscription` | 比较 Latest 与 FIFO 订阅及回调频率 |
| `glove/time_sync` | [TimeSync.java](src/main/java/com/qnbot/examples/TimeSync.java) | `port`、`side`、`referenceTimeMillis`、`seconds` | 刷新外部时间并观察授时后的时间戳 |

`port` 传实际串口标识。`quick_start`、`device_info`、`device_lifecycle`、`host_capture_calibration` 和 `time_sync` 可传 `null` 自动发现已授权的唯一设备；其他需要端口的用例必须显式提供。Glove 左右手可以使用同一 Receiver 端口。`seconds` 是正的运行秒数，`updates` 是正的更新次数，`samples` 是正的样本数量。`subscription` 传 `SubscriptionConfig.latest(maxRateHz)` 或 `SubscriptionConfig.fifo(capacity, maxRateHz)`。`debug_trace` 用 `sampleRate` 和 `detail` 选择诊断采样与详细程度。`haptics` 保持拇指 80、食指 40，持续 `holdSeconds` 秒后清除。

`cancelled` 是非阻塞、线程安全的 `BooleanSupplier`。停止按钮、页面销毁和进程退出把结果设为 `true`；示例在下一轮检查后结束并关闭资源。

## 算法与采集

算法用例的 `packageId` 使用目标应用中已准备、与 Android SDK 匹配的真实算法包。通过产品交付说明中的 QnBot CLI 准备算法；桌面算法安装不代表 Android 应用已经具备运行条件。

普通运行、配置、调试和骨骼用例使用 SDK 默认的采集策略，不自行处理采集提示。算法示例假定产品要求的采集与标定已经完成；首次准备请使用 `host_capture_calibration`，把 `confirm` 接到应用 UI，或按产品交付流程先完成算法准备。`host_capture_calibration` 先完成左右手采集，再选择目标并执行标定，是这个显式流程的完整示例。`packageIds` 为空时使用内置 Skeleton；多个包时用 `selectedPackageId` 选定一个，`force` 请求重新采集。`cancelled` 停止当前采集或标定，取消以 `CancellationException` 报告。

两个 `external_input` 示例由应用提供帧，因此也由应用决定采集交互。`captureConfig` 使用已有 `CaptureConfig`：`new CaptureConfig()` 保留 SDK 的 `AUTO_TERMINAL` 交互；普通 Android App 没有终端输入时显式选择 `EXTERNAL`，并填写所选 `operatorId`。`confirm` 是非阻塞 `Predicate<String>`：展示提示并读取已记录的 UI 输入，已确认返回 `true`，尚未输入返回 `false`，不要等待用户。`AUTO_TERMINAL` 不调用 `confirm`。`frames` 传 `null` 时使用文件内的合成五指帧；需要真实输入时传非阻塞 `Supplier<ExternalGloveFrame>`，返回五指帧或在输入结束时返回 `null`，时间戳每帧增加 10 毫秒。

`algorithm_configuration` 的 `boneConfigMode` 使用 `AUTO`、`ENABLED` 或 `DISABLED`；`algorithmConfigPath` 传算法支持的可读配置文件路径，不指定时传 `null`。需要先准备骨长或导入算法时，仍只复制这个文件，在工作线程调用其中的 `importAlgorithmArchive`、`importBoneConfig` 或 `prepareBoneConfig`，再调用 `run`。`prepareBoneConfig` 的链接展示回调必须立即返回；会话过期、失败和取消通过异常报告。

`time_sync` 的 `referenceTimeMillis` 每次返回新的外部 Unix 毫秒时间；可传 `System::currentTimeMillis`。输出 `reference_time_ms`、`source_time_ms` 和 `observed_at_ms`；源时间与观察时间基准不同，不能直接相减。需要包含 `Glove.syncTime` 的匹配 Core/Glove 交付；首个包含该能力的发布版本尚未确定。

### 墨线手套

使用包含墨线支持的配套 Android SDK。连接按出厂设置主动上报运动和压感的墨线手套。传入实际串口，在工作线程调用 `MoxianTelemetry.run(context, transport, selectedPort, stop::get)`，手侧由设备识别。运行示例前，按 [Java 骨架用例](src/main/java/com/qnbot/examples/Skeleton.java) 准备匹配的 Skeleton 算法，并在同一应用中使用设备名 `primary` 和操作者 `default` 完成当前佩戴者的标定。应用可独立调用 `device.pressure()` 读取压感。将 `stop` 设为 `true` 结束运行。

## 排错

- 找不到依赖：确认 `sdkMavenDir` 指向完整 `maven/`，使用真实且匹配的组件版本，保留公共依赖仓库。
- USB 无法打开：检查权限、USB Host、线缆及单串口 CDC ACM 支持；设备选择和外部帧用例无需 USB。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并先运行 `discover_gloves`。
- 没有目标输出：确认已按产品流程准备算法和采集/标定；普通示例不再代替应用确认采集提示。
- 查看 SDK 日志：`adb logcat -s qnbot-sdk:I`。保留用例、输入条件和完整错误信息以便排查。

路径和命令变化见 [变更记录](../../../CHANGELOG.md)。
