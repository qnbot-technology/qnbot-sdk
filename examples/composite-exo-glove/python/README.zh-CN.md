# Composite Exo+Glove Python 示例

本目录演示使用同一个 `Sdk` 访问 Exo 与 Glove 的组合设备。组合设备通过一个共享串口连接，两个成员分别由 `sdk.exo()` 和 `sdk.glove()` 访问。

## 准备工作

- Python 3.10 或更高版本；
- 一台同时提供 Exo 与 Glove 数据的受支持设备；
- 已安装 `qnbot-sdk-exo` 和 `qnbot-sdk-glove`。

## 安装

使用产品交付的 Python 安装包：

```bash
python -m pip install qnbot-sdk-exo qnbot-sdk-glove
```

也可以使用本目录的依赖文件：

```bash
python -m pip install -r requirements.txt
```

## 运行

可直接运行的 Python 源码位于 `src/`。

自动发现共享串口并显示两个成员的信息：

```bash
python src/discover_exo_glove.py
```

使用显式串口启动组合设备：

```bash
python src/quick_start.py --port /dev/ttyUSB0
```

Windows 请将串口替换为实际的 `COM` 端口，例如 `--port COM3`。

## 示例列表

| 示例 | 用途 |
| --- | --- |
| `discover_exo_glove.py` | 使用 `CompositeExoGloveConfig` 和 `SerialConnection(auto_discover=True)` 发现并显示 Glove、Exo 信息 |
| `quick_start.py` | 使用显式共享串口创建组合设备，并通过同一 SDK 访问两个成员 |
| `device_info.py` | 分别读取两个成员的设备信息 |
| `telemetry.py` | 读取 Glove pose、Exo telemetry 和 Exo status |
| `imu.py` | 读取 Glove 与 Exo 的 IMU 数据 |
| `exo_haptics.py` | 仅设置和清除 Exo 侧 haptics |
| `lifecycle.py` | 演示 start、一次读取、stop、close |

`telemetry.py` 和 `imu.py` 会持续运行，按 `Ctrl+C` 停止。`exo_haptics.py` 的 `--left`、`--right` 参数取值为设备支持的触觉强度，`--hold` 参数以秒为单位控制保持时间。

组合示例分别使用 `glove_domain` 和 `exo_domain` 执行成员域的整体启动、停止和关闭；
持续读取 Glove 与 Exo 时保留 `sdk.run_forever()`，因为一个聚合运行器需要同时驱动两个成员域。

生命周期的完整用法请参考 [Exo 示例](../../exo/python/README.zh-CN.md) 和 [Glove 示例](../../glove/python/README.zh-CN.md) 中对应的生命周期说明。

## 常见问题

- 找不到设备：确认组合设备已连接、共享串口未被其他程序占用，并先运行 `discover_exo_glove.py`。
- 只有一个成员有输出：确认设备固件支持 Exo 与 Glove 组合数据，并检查两个成员的设备信息。
- 无法安装依赖：确认 Python 版本和安装包平台与当前系统匹配。
