# QnBot SDK 客户示例

本仓库按产品组件提供 QnBot SDK 的客户示例、运行说明和公开发布包入口。先确认你要使用的
组件，再进入该组件的示例说明。

## 选择组件

| 组件 | Python 安装 | C++ 交付包 | 示例说明 |
| --- | --- | --- | --- |
| Glove 手套 | 通过 PyPI 安装 [qnbot-sdk-glove](https://pypi.org/project/qnbot-sdk-glove/) | 从 [Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台匹配的 QnBot C++ SDK/Glove 交付包 | [Glove 示例](examples/glove/README.zh-CN.md) |
| Exo 外骨骼 | 使用产品交付的 `qnbot-sdk-exo` 安装包 | 从 [Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台匹配的 QnBot C++ SDK/Exo 交付包 | [Exo 示例](examples/exo/README.zh-CN.md) |
| Exo+Glove 共享串口 | 同时安装 `qnbot-sdk-exo` 和 `qnbot-sdk-glove` | 同时安装 Exo 与 Glove C++ 交付包 | [Exo+Glove 示例](examples/composite-exo-glove/README.zh-CN.md) |

只使用单个组件时，进入对应组件示例；需要共享串口组合设备时，同时安装两个组件并进入 Exo+Glove 示例。

## 仓库内容

```text
├── examples/
│   ├── glove/             # 手套组件的 Python 与 C++ 示例
│   │   ├── python/
│   │   └── cpp/
│   ├── exo/               # 外骨骼组件的 Python 与 C++ 示例
│   │   ├── python/
│   │   └── cpp/
│   └── composite-exo-glove/ # Exo+Glove 共享串口示例
│       ├── python/
│       └── cpp/
├── CHANGELOG.md           # 示例与交付变更
├── LICENSE
└── README.zh-CN.md
```

## 开始使用

- 手套用户请阅读 [Glove 示例说明](examples/glove/README.zh-CN.md)。
- 外骨骼用户请阅读 [Exo 示例说明](examples/exo/README.zh-CN.md)。
- Exo+Glove 共享串口用户请阅读 [Exo+Glove 示例说明](examples/composite-exo-glove/README.zh-CN.md)。
- 变更记录见 [CHANGELOG.md](CHANGELOG.md)。

## 支持

如需获取安装包、算法包、设备接入支持或产品交付说明，请联系 QnBot 对接人员。

## 许可证

本仓库内容采用 [MIT License](LICENSE)。
