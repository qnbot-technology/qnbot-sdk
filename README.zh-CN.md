# QnBot SDK 客户示例

本仓库提供 QnBot SDK 的客户示例、运行说明和公开发布包入口。

Python 用户默认通过 PyPI 安装：

```bash
python -m pip install qnbot-sdk-glove
```

也可以从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases)
下载与当前平台匹配的 wheel。C++ 用户从同一 Releases 页面下载匹配平台的 C++ 交付包。

## 快速入口

| 语言 | 安装包 | 示例说明 |
| --- | --- | --- |
| Python | 通过 PyPI 安装 [qnbot-sdk-glove](https://pypi.org/project/qnbot-sdk-glove/) | [Python 示例](examples/python/README.zh-CN.md) |
| C++ | 从 [Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台匹配的 QnBot C++ SDK/Glove 交付包 | [C++ 示例](examples/cpp/README.zh-CN.md) |

## 安装 CLI 和目标手算法包

运行需要目标手算法包的示例前，先安装 QnBot CLI 0.1.0，或产品交付说明指定的兼容版本。

macOS 或 Linux：

```bash
curl -fsSL https://get.qnbot.com/cli | bash
```

Windows PowerShell：

```powershell
irm https://get.qnbot.com/cli.ps1 | iex
```

安装产品交付的算法包并确认安装结果：

```bash
qnbot --version
qnbot algorithm install ./package.zip
qnbot algorithm list
```

将 `./package.zip` 替换为与当前操作系统、CPU 架构和 SDK 版本匹配的算法包文件。

## 仓库内容

```text
├── examples/
│   ├── python/            # Python 示例与运行说明
│   └── cpp/               # C++ 示例与构建说明
├── CHANGELOG.md           # 示例与交付变更
├── LICENSE
└── README.zh-CN.md
```

## 开始使用

- Python 用户请阅读 [Python 示例说明](examples/python/README.zh-CN.md)。
- C++ 用户请阅读 [C++ 示例说明](examples/cpp/README.zh-CN.md)。
- 变更记录见 [CHANGELOG.md](CHANGELOG.md)。

## 支持

如需获取安装包、算法包、设备接入支持或产品交付说明，请联系 QnBot 对接人员。

## 许可证

本仓库内容采用 [MIT License](LICENSE)。
