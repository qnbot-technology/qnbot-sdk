# Exo+Glove 组合设备 Java 示例

从 [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) 开始，在自己的 Android 项目中读取组合设备数据。每个用例是一份可独立复制的 Java 源码，文件内直接展示配置、SDK 创建、订阅、运行和清理；用例、输入和结果见下表。

## 准备

1. 取得支持统一 `Sdk` 入口的完整配套 Android Maven 交付，按交付清单校验并解压，保留完整 `maven/`。RC/Stable 也可从 [SDK 发布列表](https://github.com/qnbot-technology/qnbot-sdk/releases) 选择；版本以实际交付记录为准。统一入口的首个发布版本尚未确定，使用明确提供该入口的配套组件版本。
2. 准备 Android API 26 或以上的 `arm64-v8a` 运行环境。现有 Java SDK 需要真实 Android `Context`；当前不提供桌面 JVM 或 x86 运行包。
3. 在自己的项目中接入对应 SDK 组件并启用标准 core library desugaring。USB 输入先由自己的应用取得权限，准备支持的 `SerialTransport`；内置 `UsbSerialTransport` 支持 USB Host 上的单串口 CDC ACM 设备。

## 先读取数据

在自己的工作线程中调用 QuickStart，传入已有 `Context` 和已获授权的传输；SDK 自动发现唯一组合设备：

```java
import com.qnbot.examples.QuickStart;

QuickStart.run(context, transport);
```

应持续看到 `left sequence=` 和 `right sequence=` 递增，并带有左右手拇指和食指位置。QuickStart 用 `runForever()` 阻塞工作线程，原样调用不提供停止句柄。需要在应用内主动停止时，复制源码并由应用持有 `Sdk`，再从其他线程关闭；原样运行可随进程退出结束。不要在 UI 线程直接调用。

有限运行用例按 `seconds` 自行结束。

## 用例与参数

每个方法都接收 `Context context` 和 `SerialTransport transport`。下表列出其余参数；具体类型和顺序见对应源码的 `run` 方法。端口均须完成所需 USB 授权。

| 用例 | Java 源码 | 其余参数 | 预期结果 |
| --- | --- | --- | --- |
| `composite-exo-glove/device_info` | [DeviceInfo.java](src/main/java/com/qnbot/examples/DeviceInfo.java) | `port` | 左右手来源与 Exo 设备信息 |
| `composite-exo-glove/discover_exo_glove` | [DiscoverExoGlove.java](src/main/java/com/qnbot/examples/DiscoverExoGlove.java) | `port` | 在共享端口发现组合设备 |
| `composite-exo-glove/glove_haptics` | [GloveHaptics.java](src/main/java/com/qnbot/examples/GloveHaptics.java) | `port`、`holdSeconds` | 提交与清除请求，观察组合手套不产生触觉的提示 |
| `composite-exo-glove/glove_retargeting` | [GloveRetargeting.java](src/main/java/com/qnbot/examples/GloveRetargeting.java) | `port`、`packageId` | 读取组合设备双手目标输出 |
| `composite-exo-glove/imu` | [Imu.java](src/main/java/com/qnbot/examples/Imu.java) | `port` | 读取受支持的 IMU 数据 |
| `composite-exo-glove/lifecycle` | [Lifecycle.java](src/main/java/com/qnbot/examples/Lifecycle.java) | `port` | 启动、读取、停止并关闭组合设备 |
| `composite-exo-glove/quick_start` | [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) | 无额外参数 | 自动发现唯一组合设备并观察持续数据 |
| `composite-exo-glove/telemetry` | [Telemetry.java](src/main/java/com/qnbot/examples/Telemetry.java) | `port` | 左右手姿态、状态与 Exo 遥测 |

`port` 传组合设备的共享串口标识。`quick_start` 不接收端口，始终自动发现唯一组合设备；其他需要端口的用例必须显式提供。

`glove_retargeting` 的 `packageId` 使用目标应用中已准备、与 Android SDK 匹配的真实算法包。普通运行示例使用 SDK 默认采集策略；需要显式采集或标定时，使用 Glove 的 `host_capture_calibration`，或在可交互终端运行对应流程。

组合手套的原始 IMU 不可用；`imu` 只读取设备支持的 Exo 数据。Glove 触觉请求成功返回但被忽略，不产生实际振动；观察该提示和警告即为当前示例的预期结果。

## 排错

- 找不到依赖：确认 `sdkMavenDir` 指向完整 `maven/`，使用真实且匹配的组件版本，保留公共依赖仓库。
- USB 无法打开：检查权限、USB Host、线缆及单串口 CDC ACM 支持。
- 找不到设备：确认组合设备已连接、共享串口未被其他程序占用，并先运行 `discover_exo_glove`。
- 只有一个成员有输出：确认设备固件支持 Exo、左 Glove 与右 Glove 组合数据。
- 没有目标输出：确认已按产品流程准备算法和采集/标定。
- 查看 SDK 日志：`adb logcat -s qnbot-sdk:I`。保留用例、输入条件和完整错误信息以便排查。

路径和命令变化见 [变更记录](../../../CHANGELOG.md)。
