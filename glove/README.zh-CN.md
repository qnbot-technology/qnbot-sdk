# QnBot SDK Glove 示例

本目录提供手套组件（Glove）的 Python 与 C++ 客户示例，覆盖设备发现、手套姿态、有线
IMU 原始数据、目标手输出、运行模式、触觉反馈以及采集与标定。

## 准备工作

- 与当前操作系统和 CPU 架构匹配的受支持手套；
- Python 用户需要 Python 3.10 或更高版本，并通过 PyPI 安装 `qnbot-sdk-glove`；
- C++ 用户需要 CMake 3.16 或更高版本、支持 C++17 的编译器，以及当前平台匹配的
  QnBot C++ SDK/Glove 交付包；
- 目标手输出示例所需的算法包。

需要目标手输出时，先安装 QnBot CLI 0.2.0，或产品交付说明指定的兼容版本。

macOS 或 Linux：

```bash
curl -fsSL https://get.qnbot.com/cli | bash
```

Windows PowerShell：

```powershell
irm https://get.qnbot.com/cli.ps1 | iex
```

使用 CLI 安装产品交付的算法包：

```bash
qnbot --version
qnbot algorithm install ./package.zip
qnbot algorithm list
```

`package.zip` 必须与当前操作系统、CPU 架构和 SDK 版本匹配。不需要目标手输出的示例
（发现、姿态、Skeleton、IMU、触觉反馈）不加载算法包。

## 语言示例

| 语言 | 环境 | 说明 |
| --- | --- | --- |
| Python | Python 3.10 或更高版本，通过 PyPI 安装 `qnbot-sdk-glove` | [Python 示例](python/README.zh-CN.md) |
| C++ | CMake 3.16 或更高版本、C++17 编译器、C++ SDK/Glove 交付包 | [C++ 示例](cpp/README.zh-CN.md) |

两个工程各自独立：只需要安装你要使用的语言所对应的交付包。

## 支持

仍无法解决时，请保留运行命令和完整错误信息并联系 QnBot 对接人员。
