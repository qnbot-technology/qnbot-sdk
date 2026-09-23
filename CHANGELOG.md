# 变更记录

本文件记录 QnBot SDK 示例和安装包交付的变化。

## 1.3.0 - 2026-09-23

- 与 SDK 1.3.0 的采集、标定、设备接入和算法包使用流程保持一致。
- 公开示例继续提供 Linux x86_64、Linux ARM64、macOS ARM64 和 Windows x86_64 四平台入口。

## 未发布

- 公开示例按组件重新组织为 `glove/` 与 `exo/`，每个组件各自提供独立的 Python 和 C++ 工程；此前 `examples/python/` 和 `examples/cpp/` 下的示例路径全部变更。
- 新增 Exo 外骨骼组件的 Python 和 C++ 示例：设备发现、快速运行、设备信息、遥测与状态、IMU、手柄、触觉反馈和多设备生命周期。
- 新增 `composite-exo-glove/` Python 与 C++ 公共示例，覆盖共享串口自动发现、显式串口、成员信息、遥测、IMU、Exo 触觉和生命周期。
- 删除 bindings 与 Rust crate 中重复的 Exo 客户示例，统一从 `exo/` 获取。
- Glove 示例的程序行为、参数和输出保持不变。

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
