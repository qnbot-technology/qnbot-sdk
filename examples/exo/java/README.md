# Exo 外骨骼 Java 示例

从 [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) 开始，在自己的 Android 项目中读取外骨骼数据。每个用例是一份可独立复制的 Java 源码，文件内直接展示配置、SDK 创建、订阅、运行和清理；用例、输入和结果见下表。

## 准备

1. 取得支持统一 `Sdk` 入口的完整配套 Android Maven 交付，按交付清单校验并解压，保留完整 `maven/`。RC/Stable 也可从 [SDK 发布列表](https://github.com/qnbot-technology/qnbot-sdk/releases) 选择；版本以实际交付记录为准。统一入口的首个发布版本尚未确定，使用明确提供该入口的配套组件版本。
2. 准备 Android API 26 或以上的 `arm64-v8a` 运行环境。现有 Java SDK 需要真实 Android `Context`；当前不提供桌面 JVM 或 x86 运行包。
3. 在自己的项目中接入对应 SDK 组件并启用标准 core library desugaring。USB 输入先由自己的应用取得权限，准备支持的 `SerialTransport`；内置 `UsbSerialTransport` 支持 USB Host 上的单串口 CDC ACM 设备。

## 先读取数据

在自己的工作线程中调用 QuickStart，传入已有 `Context` 和已获授权的传输。`port` 使用实际端口标识；传 `null` 自动发现唯一设备：

```java
import com.qnbot.examples.QuickStart;
import java.util.concurrent.atomic.AtomicBoolean;

AtomicBoolean stop = new AtomicBoolean();
QuickStart.run(context, transport, port, stop::get);
```

应持续看到 `telemetry sequence=` 递增，并带有左右编码器和关节弧度。QuickStart 会阻塞到停止信号生效，结束后自行关闭设备。把它放在工作线程，并把停止按钮、页面销毁和进程退出接到 `stop.set(true)`；不要在 UI 线程直接调用。

其他持续读取用例使用同一个停止信号接法；有限运行用例按 `seconds` 自行结束。

## 用例与参数

每个方法都接收 `Context context` 和 `SerialTransport transport`。下表列出其余参数；具体类型和顺序见对应源码的 `run` 方法。端口均须完成所需 USB 授权。

| 用例 | Java 源码 | 其余参数 | 预期结果 |
| --- | --- | --- | --- |
| `exo/device_info` | [DeviceInfo.java](src/main/java/com/qnbot/examples/DeviceInfo.java) | `port` | 读取设备信息与能力 |
| `exo/device_lifecycle` | [DeviceLifecycle.java](src/main/java/com/qnbot/examples/DeviceLifecycle.java) | `firstPort`、`secondPort`、`seconds` | 独立停止一台设备，另一台继续输出 |
| `exo/discover_exos` | [DiscoverExos.java](src/main/java/com/qnbot/examples/DiscoverExos.java) | 无额外参数 | 通过宿主传输探测 Exo，显示候选及逐设备错误 |
| `exo/handset` | [Handset.java](src/main/java/com/qnbot/examples/Handset.java) | `port`、`cancelled` | 读取左右手柄的轴、扳机和按钮 |
| `exo/haptics` | [Haptics.java](src/main/java/com/qnbot/examples/Haptics.java) | `port`、`left`、`right`、`holdSeconds` | 设置左右触觉并清除 |
| `exo/imu` | [Imu.java](src/main/java/com/qnbot/examples/Imu.java) | `port`、`cancelled` | 读取受支持的 IMU 数据 |
| `exo/quick_start` | [QuickStart.java](src/main/java/com/qnbot/examples/QuickStart.java) | `port`、`cancelled` | 连接设备并观察持续遥测 |
| `exo/telemetry` | [Telemetry.java](src/main/java/com/qnbot/examples/Telemetry.java) | `port`、`cancelled` | 读取遥测、连接状态与过期标记 |

`port` 传实际串口标识。`quick_start` 可传 `null` 自动发现已授权的唯一设备；其他需要端口的用例必须显式提供。独立 Exo 生命周期用例需要两个不同端口。

`device_lifecycle` 启动两台设备并等待 `seconds` 秒，停止第二台后读取第一台的健康状态和最新遥测，随后停止并关闭两台。`haptics` 使用 `left`、`right` 设置两侧强度，保持 `holdSeconds` 秒后清除；强度取值为 0 到 100。

`discover_exos` 在工作线程调用，显示候选列表和每条候选的 `error`。App 负责 USB 选择与授权。发现结束只关闭临时连接，不释放 `transport`；选择候选后可将同一个传输交给 `Sdk`，关闭 SDK 后再按原有规则释放。

`imu` 和 `handset` 先读取设备上报的能力集；设备没有对应能力时直接说明该载荷不可用，不输出空结构。

## 排错

- 找不到依赖：确认 `sdkMavenDir` 指向完整 `maven/`，使用真实且匹配的组件版本，保留公共依赖仓库。
- USB 无法打开：检查权限、USB Host、线缆及单串口 CDC ACM 支持。
- 找不到设备：确认设备已连接、串口未被其他程序占用，并先运行 `discover_exos`。
- 没有遥测输出：确认设备已启动，并确认设备上报了遥测能力。
- 没有 IMU 或手柄输出：确认设备上报了对应能力集。
- 查看 SDK 日志：`adb logcat -s qnbot-sdk:I`。保留用例、输入条件和完整错误信息以便排查。

路径和命令变化见 [变更记录](../../../CHANGELOG.md)。
