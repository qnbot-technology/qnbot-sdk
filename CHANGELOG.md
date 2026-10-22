# 变更记录

本文件记录 QnBot SDK 示例和安装包交付的变化。

## 1.4.0 - 2026-09-30

- SDK 1.4.0 正式版本同步发布四平台示例和交付内容。

## 未发布

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
