# 变更记录

本文件记录 QnBot SDK 示例和安装包交付的变化。

## 尚未发布

- 暂无。

## 1.1.0 - 2026-08-30

- 示例与 SDK 1.1.0 的独立采集、校准和运行流程保持一致。
- 安装说明增加 Linux ARM64 交付平台，并明确 Core 要求 glibc 2.34+、Glove 当前要求 glibc 2.38+。
- 所有目标输出示例都要求客户显式传入 `--package-id`；采集与标定示例使用同一个可重复参数，不提供默认算法包。

## 1.0.0 - 2026-08-25

- 原有 C++ `coroutine_next` 示例已替换为 `async_runtime`，用于演示连接真实手套后的 C++20 异步读取方式。
- Python 与 C++ 示例均提供设备发现、动态手部骨骼（`skeleton`）、快速启动、运行模式、诊断、多路输出、设备生命周期和触觉反馈用法。
- 增加公开 QnBot SDK 示例仓库结构。
- 增加 Python 和 C++ Glove SDK 客户示例。
- 目标手算法包通过 QnBot CLI 安装，示例使用已安装算法包的 package ID。
- Core 和 Glove 的稳定版及候选版安装包通过 GitHub Releases 提供。
