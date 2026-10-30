# Exo+Glove 组合设备 Java 示例

从 [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) 开始，在自己的 Android 项目中读取设备数据。每个用例只需复制对应的一个 Java 文件，文件内直接展示配置、SDK 创建、数据读取、执行和清理；用例、参数和结果见下表。

## 准备

1. 取得本组件 Java 示例源码及完整配套 Android Maven 交付，按交付清单校验并解压，保留完整 `maven/`。RC/Stable 也可从 [SDK 发布列表](https://github.com/qnbot-technology/qnbot-sdk/releases) 选择；版本以实际交付记录为准。
2. 准备 Android API 26 或以上的 `arm64-v8a` 运行环境。现有 Java SDK 需要真实 Android `Context`；当前不提供桌面 JVM 或 x86 运行包。
3. 在自己的项目中接入对应 SDK 组件并启用标准 core library desugaring。USB 输入先由自己的应用取得权限，准备支持的 `SerialTransport`；内置 `UsbSerialTransport` 支持 USB Host 上的单串口 CDC ACM 设备。

## 先读取数据

在自己的工作线程中调用 QuickStart，传入已有 Context 和已获授权的传输。`selectedPort` 使用实际端口标识：

```java
import com.qnbot.examples.QuickStart;

QuickStart.run(context, transport, selectedPort);
```

屏幕或终端输出取决于调用项目的标准输出设置；Android 可在 logcat 查看 `System.out`。应持续看到左右手姿态和递增的 sequence。QuickStart 使用阻塞运行，放在适合的工作线程；将源码中的组件对象及停止操作接入自己的应用生命周期。示例不提供 Android 应用、界面或 APK。

## 用例与参数

每个方法都接收 `Context context` 和 `SerialTransport transport`。下表列出其余参数；具体类型和顺序见对应源码的 `run` 方法。端口均须完成所需 USB 授权。

| 用例 | Java 源码 | 其余参数 | 预期结果 |
| --- | --- | --- | --- |
| `composite-exo-glove/device_info` | [DeviceInfo.java](src/main/java/com/qnbot/examples/DeviceInfo.java) | `port` | 左右手来源与 Exo 设备信息 |
| `composite-exo-glove/discover_exo_glove` | [DiscoverExoGlove.java](src/main/java/com/qnbot/examples/DiscoverExoGlove.java) | `port` | 在共享端口发现组合设备 |
| `composite-exo-glove/glove_haptics` | [GloveHaptics.java](src/main/java/com/qnbot/examples/GloveHaptics.java) | `port` | 提交与清除请求，观察组合手套不产生触觉的提示 |
| `composite-exo-glove/glove_retargeting` | [GloveRetargeting.java](src/main/java/com/qnbot/examples/GloveRetargeting.java) | `port`、`packageId`、`confirm` | 读取组合设备双手目标输出 |
| `composite-exo-glove/imu` | [Imu.java](src/main/java/com/qnbot/examples/Imu.java) | `port` | 读取受支持的 IMU 数据 |
| `composite-exo-glove/lifecycle` | [Lifecycle.java](src/main/java/com/qnbot/examples/Lifecycle.java) | `port` | 启动、读取、停止并关闭组合设备 |
| `composite-exo-glove/quick_start` | [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) | `port` | 连接设备并观察持续数据 |
| `composite-exo-glove/telemetry` | [Telemetry.java](src/main/java/com/qnbot/examples/Telemetry.java) | `port` | 左右手姿态、状态与 Exo 遥测 |

`port` 为组合设备的共享串口标识。

算法用例的 `packageId` 使用目标应用中已准备、与 Android SDK 匹配的真实算法包。按产品交付指南通过 QnBot CLI 准备算法；桌面算法安装不代表 Android 应用已经具备运行条件。

`confirm` 是调用方提供的 `Predicate<String>`：显示当前动作提示并等待操作员决定，返回 `true` 确认、`false` 取消。不要无条件确认真实采集。带 `cancelled` 的方法通过调用方提供的 `BooleanSupplier` 读取取消请求。普通算法用例采用自动准备；显式采集和标定见 Glove 的 `host_capture_calibration`。

组合手套的原始 IMU 不可用；IMU 示例只读支持的 Exo 数据。Glove 触觉请求被忽略，不产生实际振动。

## 排错

- 找不到依赖：确认 `sdkMavenDir` 指向完整 `maven/`，使用真实且匹配的组件版本，保留公共依赖仓库。
- USB 无法打开：检查权限、USB Host、线缆及单串口 CDC ACM 支持。
- 没有数据：检查输入是否持续、首帧是否到达；算法流程还需完成提示并具备匹配算法交付。
- 查看 SDK 日志：`adb logcat -s qnbot-sdk:I`。保存用例、输入条件和完整错误信息以便排查。

路径和命令变化见 [变更记录](../../../CHANGELOG.md)。
