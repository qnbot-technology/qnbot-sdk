# Exo 外骨骼 Java 示例

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

屏幕或终端输出取决于调用项目的标准输出设置；Android 可在 logcat 查看 `System.out`。应持续看到编码器、关节位置和递增的 sequence。QuickStart 使用阻塞运行，放在适合的工作线程；将源码中的组件对象及停止操作接入自己的应用生命周期。示例不提供 Android 应用、界面或 APK。

## 用例与参数

每个方法都接收 `Context context` 和 `SerialTransport transport`。下表列出其余参数；具体类型和顺序见对应源码的 `run` 方法。端口均须完成所需 USB 授权。

| 用例 | Java 源码 | 其余参数 | 预期结果 |
| --- | --- | --- | --- |
| `exo/device_info` | [DeviceInfo.java](src/main/java/com/qnbot/examples/DeviceInfo.java) | `port` | 读取设备信息与能力 |
| `exo/device_lifecycle` | [DeviceLifecycle.java](src/main/java/com/qnbot/examples/DeviceLifecycle.java) | `firstPort`、`secondPort` | 独立停止一台设备，另一台继续输出 |
| `exo/discover_exos` | [DiscoverExos.java](src/main/java/com/qnbot/examples/DiscoverExos.java) | `port` | 发现已授权的 Exo 候选并显示结果 |
| `exo/handset` | [Handset.java](src/main/java/com/qnbot/examples/Handset.java) | `port` | 读取左右手柄的轴、扳机和按钮 |
| `exo/haptics` | [Haptics.java](src/main/java/com/qnbot/examples/Haptics.java) | `port` | 设置触觉并清除 |
| `exo/imu` | [Imu.java](src/main/java/com/qnbot/examples/Imu.java) | `port` | 读取受支持的 IMU 数据 |
| `exo/quick_start` | [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) | `port` | 连接设备并观察持续数据 |
| `exo/telemetry` | [Telemetry.java](src/main/java/com/qnbot/examples/Telemetry.java) | `port` | 遥测、连接状态与过期标记 |

`port` 为实际串口标识。独立 Exo 生命周期用例需要两个不同端口。

## 排错

- 找不到依赖：确认 `sdkMavenDir` 指向完整 `maven/`，使用真实且匹配的组件版本，保留公共依赖仓库。
- USB 无法打开：检查权限、USB Host、线缆及单串口 CDC ACM 支持。
- 没有数据：检查输入是否持续、首帧是否到达；算法流程还需完成提示并具备匹配算法交付。
- 查看 SDK 日志：`adb logcat -s qnbot-sdk:I`。保存用例、输入条件和完整错误信息以便排查。

路径和命令变化见 [变更记录](../../../CHANGELOG.md)。
