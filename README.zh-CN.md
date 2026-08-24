# QnBot SDK 客户示例

本仓库提供 QnBot SDK 的客户示例、运行说明和公开发布包入口。

## 快速入口

| 语言 | 安装包 | 示例说明 |
| --- | --- | --- |
| Python | 通过 PyPI 安装 [qnbot-sdk-glove](https://pypi.org/project/qnbot-sdk-glove/) | [Python 示例](examples/python/README.zh-CN.md) |
| C++ | 从 [Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台匹配的 Core 和 Glove C++ 包 | [C++ 示例](examples/cpp/README.zh-CN.md) |

运行需要目标手算法包的示例前，请先按照产品交付说明，通过随产品提供的 CLI 安装对应
算法包。请确保 SDK、算法包、操作系统和 CPU 架构相互匹配。

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
