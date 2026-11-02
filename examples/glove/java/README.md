# Glove 手套 Java 示例

从 [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) 开始，在自己的 Android 项目中读取设备数据。每个用例只需复制对应的一个 Java 文件，文件内直接展示配置、SDK 创建、数据读取、执行和清理；用例、参数和结果见下表。

## 准备

1. 取得本组件 Java 示例源码及完整配套 Android Maven 交付，按交付清单校验并解压，保留完整 `maven/`。RC/Stable 也可从 [SDK 发布列表](https://github.com/qnbot-technology/qnbot-sdk/releases) 选择；版本以实际交付记录为准。
2. 准备 Android API 26 或以上的 `arm64-v8a` 运行环境。现有 Java SDK 需要真实 Android `Context`；当前不提供桌面 JVM 或 x86 运行包。
3. 在自己的项目中接入对应 SDK 组件并启用标准 core library desugaring。USB 输入先由自己的应用取得权限，准备支持的 `SerialTransport`；内置 `UsbSerialTransport` 支持 USB Host 上的单串口 CDC ACM 设备。

## 先读取数据

在自己的工作线程中调用 QuickStart，传入已有 Context 和已获授权的传输。`selectedPort` 使用实际端口标识，左右手与设备一致：

```java
import com.qnbot.examples.QuickStart;
import com.qnbot.sdk.config.Side;

QuickStart.run(context, transport, selectedPort, Side.RIGHT);
```

屏幕或终端输出取决于调用项目的标准输出设置；Android 可在 logcat 查看 `System.out`。应持续看到手套姿态和递增的 sequence。QuickStart 使用阻塞运行，放在适合的工作线程；将源码中的组件对象及停止操作接入自己的应用生命周期。示例不提供 Android 应用、界面或 APK。

## 用例与参数

除 `device_selection` 外，每个方法都接收 `Context context` 和 `SerialTransport transport`。下表列出其余参数；具体类型和顺序见对应源码的 `run` 方法。端口均须完成所需 USB 授权。

| 用例 | Java 源码 | 其余参数 | 预期结果 |
| --- | --- | --- | --- |
| `glove/algorithm_configuration` | [AlgorithmConfiguration.java](src/main/java/com/qnbot/examples/AlgorithmConfiguration.java) | `port`、`side`、`packageId`、`boneConfigMode`、`algorithmConfigPath`、`confirm` | 使用已有骨长策略与算法配置文件读取目标输出 |
| `glove/async_runtime` | [AsyncRuntime.java](src/main/java/com/qnbot/examples/AsyncRuntime.java) | `port`、`side`、`packageId`、`samples`、`confirm`、`cancelled` | 一次异步读取和连续流消费，处理停止、取消和运行结束 |
| `glove/background_runtime` | [BackgroundRuntime.java](src/main/java/com/qnbot/examples/BackgroundRuntime.java) | `port`、`side`、`packageId`、`seconds`、`confirm` | 后台推进并持续读取目标输出 |
| `glove/debug_trace` | [DebugTrace.java](src/main/java/com/qnbot/examples/DebugTrace.java) | `port`、`side`、`packageId`、`confirm` | 启用诊断记录并观察目标输出 |
| `glove/device_info` | [DeviceInfo.java](src/main/java/com/qnbot/examples/DeviceInfo.java) | `port`、`side` | 读取设备信息与能力 |
| `glove/device_lifecycle` | [DeviceLifecycle.java](src/main/java/com/qnbot/examples/DeviceLifecycle.java) | `leftPort`、`rightPort` | 独立停止一台设备，另一台继续输出 |
| `glove/device_selection` | [DeviceSelection.java](src/main/java/com/qnbot/examples/DeviceSelection.java) | 无额外参数 | 按名称、左右手选择来源并观察歧义错误 |
| `glove/discover_gloves` | [DiscoverGloves.java](src/main/java/com/qnbot/examples/DiscoverGloves.java) | `port`、`side` | 发现已授权的手套候选并显示结果 |
| `glove/external_input_manual_update` | [ExternalInputManualUpdate.java](src/main/java/com/qnbot/examples/ExternalInputManualUpdate.java) | `side`、`packageId`、`framePath`、`confirm`、`cancelled` | 提交外部帧并手动推进，观察目标输出 |
| `glove/external_input_run_background` | [ExternalInputRunBackground.java](src/main/java/com/qnbot/examples/ExternalInputRunBackground.java) | `side`、`packageId`、`framePath`、`confirm`、`cancelled` | 提交外部帧，后台推进并观察目标输出 |
| `glove/foreground_runtime` | [ForegroundRuntime.java](src/main/java/com/qnbot/examples/ForegroundRuntime.java) | `port`、`side`、`packageId`、`confirm` | 阻塞运行并在运行结束后释放资源 |
| `glove/haptics` | [Haptics.java](src/main/java/com/qnbot/examples/Haptics.java) | `port`、`side` | 设置触觉并清除 |
| `glove/host_capture_calibration` | [HostCaptureCalibration.java](src/main/java/com/qnbot/examples/HostCaptureCalibration.java) | `leftPort`、`rightPort`、`packageIds`、`selectedPackageId`、`force`、`confirm`、`cancelled` | 按多算法包计划采集，选择标定目标并读取结果；支持默认骨骼及重新采集 |
| `glove/imu` | [Imu.java](src/main/java/com/qnbot/examples/Imu.java) | `port`、`side` | 读取受支持的 IMU 数据 |
| `glove/manual_runtime` | [ManualRuntime.java](src/main/java/com/qnbot/examples/ManualRuntime.java) | `port`、`side`、`packageId`、`seconds`、`confirm` | 应用手动推进并读取目标输出 |
| `glove/multiple_outputs` | [MultipleOutputs.java](src/main/java/com/qnbot/examples/MultipleOutputs.java) | `port`、`side`、`packageId`、`confirm` | 同一来源产生两个独立命名的目标输出 |
| `glove/quick_start` | [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) | `port`、`side` | 连接设备并观察持续数据 |
| `glove/skeleton` | [Skeleton.java](src/main/java/com/qnbot/examples/Skeleton.java) | `port`、`side`、`confirm` | 观察骨骼关节与手腕坐标 |
| `glove/subscription_config` | [SubscriptionConfig.java](src/main/java/com/qnbot/examples/SubscriptionConfig.java) | `port`、`side`、`seconds` | 比较 Latest 与 FIFO 订阅及回调频率 |
| `glove/time_sync` | [TimeSync.java](src/main/java/com/qnbot/examples/TimeSync.java) | `port`、`side`、`referenceTimeMillis`、`seconds` | 刷新外部时间并观察授时后的时间戳 |

`port` 为实际串口标识。多个设备的端口按参数顺序传入；Glove 左右手可以使用同一 Receiver 端口，两个独立 Exo 必须使用不同端口。`seconds` 是运行秒数，`samples` 是正样本数量。

算法用例的 `packageId` 使用目标应用中已准备、与 Android SDK 匹配的真实算法包。按产品交付指南通过 QnBot CLI 准备算法；桌面算法安装不代表 Android 应用已经具备运行条件。

`confirm` 是调用方提供的 `Predicate<String>`：显示当前动作提示并等待操作员决定，返回 `true` 确认、`false` 取消。不要无条件确认真实采集。带 `cancelled` 的方法通过调用方提供的 `BooleanSupplier` 读取取消请求。普通算法用例采用自动准备；显式采集和标定见 `host_capture_calibration`。

`algorithm_configuration` 的 `boneConfigMode` 使用 `AUTO`、`ENABLED` 或 `DISABLED`；`algorithmConfigPath` 传算法支持的可读配置文件路径，不指定时传 `null`。

`host_capture_calibration` 接收采集 `packageIds`、要标定的 `selectedPackageId` 与 `force`。空包列表使用内置骨骼；多个包时明确选定列表中的一个包。`force=true` 请求重新采集。按当前提示完成两手采集，随后观察标定结果和新的目标输出。

外部输入的 `framePath` 指向自己的 JSONL。每行包含有限且严格递增的 `timestamp_ms` 和 `fingers`；五指 `thumb/index/middle/ring/pinky` 均包含三元素 `position`（米）和四元素 `quaternion_xyzw`（x/y/z/w）。例如两帧时间为 10、20 毫秒。输入需覆盖算法要求的动作和采集等待，短文件不保证完成准备。

`time_sync` 的 `referenceTimeMillis` 每次返回新的外部 Unix 毫秒时间；可传 `System::currentTimeMillis`。输出 `reference_time_ms`、`source_time_ms` 和 `observed_at_ms`；源时间是授时后的绝对时间，观察时间为 SDK 相对时间，不能直接相减。需要包含 `Glove.syncTime` 的匹配 Core/Glove 交付；首个包含该能力的发布版本尚未确定。

## 排错

- 找不到依赖：确认 `sdkMavenDir` 指向完整 `maven/`，使用真实且匹配的组件版本，保留公共依赖仓库。
- USB 无法打开：检查权限、USB Host、线缆及单串口 CDC ACM 支持；设备选择和外部帧用例无需 USB。
- 没有数据：检查输入是否持续、首帧是否到达；算法流程还需完成提示并具备匹配算法交付。
- 查看 SDK 日志：`adb logcat -s qnbot-sdk:I`。保存用例、输入条件和完整错误信息以便排查。

路径和命令变化见 [变更记录](../../../CHANGELOG.md)。
