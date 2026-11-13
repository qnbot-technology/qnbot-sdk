# 变更记录

## 完整标定密钥复用（发布版本未定）

- Python/C++ `algorithm_configuration` 示例通过 `--calibration-key` 使用完整标定文件，通过 `--key-output` 保存本次实际使用的文件并显示路径与摘要，便于与原始数据一起保存。设备名、目标名、操作者、手侧及算法包须与导出结果一致。
- 新参数省略时，示例保持原有流程。C++ 项目使用匹配的新 Core、Glove 头文件与运行库重新构建；原有业务调用代码继续可用。

## 压感全点观察与首帧反馈（发布版本未定）

- Python/C++/Java `pressure` 示例显示全部 460 点的原始值概览、峰值索引和截顶点数；可指定测点，默认观察峰值点。显示选中点的网格行列，三语言指南给出经协议核实的左右手区域查表。kPa 以一位小数显示，估计值和 SDK 原始数据不变。
- 示例启动后等待首帧，约 5 秒没有压感时报告检查型号、串口和上报状态。全零帧有效；1 Hz 显示可能跳过短促按压。
- 解析旧示例输出的脚本需将 `first_raw`、`first_estimated_kpa` 改为 `point_index`、`point_raw`、`point_estimated_kpa`；需要固定观察首点时传 `--point 0`，Java 调用四参数重载并传 `0`。现有 SDK 数据接口和 Java 三参数示例调用继续可用。

## 触觉 ENF 定位手套压感逐点估计（发布版本未定）

- `GlovePressure` 在同一帧包含 460 个原始计数和同序的 kPa 估计值：Python/C++ 为 `estimated_kpa`，Java 为 `estimatedKpa`；`pressure` 示例打印首点的两组值。
- 使用旧 `GlovePressure` 布局的 C++ 应用需重新编译；直接构造 `GlovePressure` 的 Python 代码需同时传入 `estimated_kpa`。只读取 `readings` 的调用无需改代码。

## Java 示例单文件调整（发布版本未定）

- 每个 Java 用例只需复制一个文件；采集确认、外部帧读取与清理直接写在该用例中。
- 删除共享的采集和外部帧辅助文件，用例、参数与结果统一见各组件 Java README。
- SDK API、外部输入格式和 SDK 依赖不变；复制旧示例源码时，按各组件 Java README 调整调用参数。

## Java 示例范围对齐（发布版本未定）

- Java 与 Python/C++ 保持 21/8/8 个独立用例，直接展示配置、创建、订阅、执行和清理；单组件使用对应组件接口，组合用例共用一个 SDK。
- 移除示例 Android 应用、APK、场景选择器和共享宿主文件。在自己的 Android 项目中调用；旧的根 Gradle/APK 命令停止使用。
- SDK 坐标、现有 API 及 Android 运行要求保持不变；首次包含本调整的示例发布版本尚未确定。


本文件记录 QnBot SDK 示例和安装包交付的变化。

## 1.4.0 - 2026-09-30

- SDK 1.4.0 正式版本同步发布四平台示例和交付内容。

## 未发布

- Java 源码覆盖 Glove、Exo 与组合设备的适用用例，含外部输入、采集标定、异步订阅与清理。`glove/time_sync` 使用匹配 Core/Glove 交付演示初始与周期授时，并显示源时间及观察时间。

- 组合设备的 Glove 触觉示例明确显示当前请求成功返回但被忽略、无实际振动，并提示不可用警告；不再把成功请求描述为硬件已执行。
- 组合 IMU 示例先检查 Exo IMU 能力，不支持时提示并退出；Python 与 C++ 保持一致。组合示例按产品发布说明选择兼容同一 Core 版本的独立版本组件。

- Glove 普通示例统一使用默认自动标定；只有 `host_capture_calibration` 显式选择手动标定，先启动设备再采集、标定。旧版未启动设备就调用 `start_capture` 的顺序会失败并提示先启动设备。
- 公开示例按组件重新组织为 `examples/glove/`、`examples/exo/` 和 `examples/composite-exo-glove/`，每个组件各自提供独立的 Python 和 C++ 工程；此前 `examples/python/` 和 `examples/cpp/` 下的示例路径全部变更。
- 新增 Exo 外骨骼组件的 Python 和 C++ 示例：设备发现、快速运行、设备信息、遥测与状态、IMU、手柄、触觉反馈和多设备生命周期。
- 新增 `examples/composite-exo-glove/` Python 与 C++ 公共示例，覆盖共享串口自动发现、显式串口、成员信息、遥测、Exo IMU、Glove 触觉和生命周期。
- 删除 bindings 与 Rust crate 中重复的 Exo 客户示例，统一从 `examples/exo/` 获取。
- Glove 示例的参数、输入和输出保持不变。
- 目标手输出改为事件驱动推进：固定的 60 Hz 上限已移除，SDK 不设默认推进频率或 CPU 上限，手动模式下由应用循环控制 `update()` 节奏。输入快于算法时按最新状态收敛，每个目标只保留最新待处理输入、最新成功和最新错误；输出只交付最新值，不提供逐帧 FIFO，已合并的中间结果无法通过重新订阅取回。合并数量可通过 Debug 的 `retargeting/coalescing` 摘要和 `retargeting_coalesced_errors` 诊断查看。
- 手动更新示例不再用 `has_next_task` 判断是否需要等待：`update()` 之后直接 `update.sleep()`；在空闲、或帧由应用自己提供时，`sleep()` 可能返回 `NO_TASKS`，由应用短暂让出后继续。`has_next_task` 与 `ran_task_count` 不再表示手套事件工作。
- 手套启动与标定统一为 `CalibrationMode`（Auto/Manual/None）：不传参数启动仍为 Auto，普通自动流程不受影响；由应用手动管理采集与标定的设备必须先 `start(Manual)` 再开始采集，未启动设备就调用 `start_capture` 的旧顺序会失败并提示先启动设备。
- Rust crate 移除公开常量 `GLOVE_CONTROL_HZ`，运行任务的调度改为显式调度声明；没有绑定、示例或 payload 引用该常量。
- 本批次不提供吞吐、帧率、CPU 或续航数字：平台性能尚未实测，也不承诺 CPU 占用或推进频率。

## 1.2.0 - 2026-09-15

- 算法包统一通过 QnBot CLI 在线查找和安装，并由 package ID 表示客户选择。
- 示例覆盖有线 IMU、VetraGlove Nano、连接状态和 Debug 诊断等 1.2.0 客户流程。
- SDK 与 CLI 都提供 Linux x86_64、Linux ARM64、macOS ARM64 和 Windows x86_64 四平台交付。

## 0.5.2 - 2026-09-11

- Python 和 C++ quick start 改为零参数自动发现单只手套并读取姿态，不再要求算法包。
- 外部输入示例改为要求客户选择已安装的算法包并接收目标手输出，并分别提供后台运行和手动更新两种清晰入口；其他运行模式示例不再提供无算法的外部输入变体。

## 1.1.0 - 2026-08-30

- 示例与 SDK 1.1.0 的独立采集、校准和运行流程保持一致。
- 安装说明增加 Linux ARM64 交付平台，并明确 Glove 当前要求 glibc 2.38+。
- 所有目标输出示例都要求客户显式传入 `--package-id`；采集与标定示例使用同一个可重复参数，不提供默认算法包。

## 1.0.0 - 2026-08-25

- 原有 C++ `coroutine_next` 示例已替换为 `async_runtime`，用于演示连接真实手套后的 C++20 异步读取方式。
- Python 与 C++ 示例均提供设备发现、动态手部骨骼（`skeleton`）、快速启动、运行模式、诊断、多路输出、设备生命周期和触觉反馈用法。
- 增加公开 QnBot SDK 示例仓库结构。
- 增加 Python 和 C++ Glove SDK 客户示例。
- 目标手算法包通过 QnBot CLI 安装，示例使用已安装算法包的 package ID。
- QnBot SDK 的稳定版及候选版安装包通过 GitHub Releases 提供。
