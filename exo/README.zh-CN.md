# QnBot SDK Exo 示例

本目录提供外骨骼组件（Exo）的 Python 与 C++ 客户示例，覆盖设备发现、设备信息、遥测与
状态、IMU、手柄、触觉反馈和多设备生命周期。示例只依赖已发布的 Exo 组件。

## 准备工作

- 与当前操作系统和 CPU 架构匹配的受支持 Exo 设备；
- Python 用户需要 Python 3.10 或更高版本，并安装产品交付的 `qnbot-sdk-exo` 安装包；
- C++ 用户需要 CMake 3.16 或更高版本、支持 C++17 的编译器，以及当前平台匹配的
  QnBot C++ SDK/Exo 交付包。

## 语言示例

| 语言 | 环境 | 说明 |
| --- | --- | --- |
| Python | Python 3.10 或更高版本，已安装 `qnbot-sdk-exo` | [Python 示例](python/README.zh-CN.md) |
| C++ | CMake 3.16 或更高版本、C++17 编译器、C++ SDK/Exo 交付包 | [C++ 示例](cpp/README.zh-CN.md) |

两个工程各自独立：只需要安装你要使用的语言所对应的交付包。

## 支持

仍无法解决时，请保留运行命令和完整错误信息并联系 QnBot 对接人员。
