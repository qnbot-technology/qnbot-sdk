# Composite Exo+Glove Python 示例

本目录演示使用同一个 `Sdk` 访问一台 Exo 与左右两只 Glove 的组合设备。组合设备通过一个共享串口连接，示例先保存 `exo = sdk.exo()` 和 `glove = sdk.glove()`，再通过领域对象访问成员设备。

## 准备工作

- Python 3.10 或更高版本；
- 一台同时提供 Exo、左 Glove 与右 Glove 数据的受支持设备；
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

自动发现共享串口并显示 Glove source 与 Exo 信息：

```bash
python src/discover_exo_glove.py
```

使用默认自动发现启动组合设备：

```bash
python src/quick_start.py
```

需要固定共享串口时，其他示例可以使用 `--port` 指定实际串口。

## 示例列表

| 示例 | 用途 |
| --- | --- |
| `discover_exo_glove.py` | 使用 `CompositeExoGloveConfig` 和 `SerialConnection(auto_discover=True)` 发现共享串口并显示左右 Glove source、Exo 信息 |
| `quick_start.py` | 使用默认自动发现创建组合设备，并通过同一 SDK 访问 Exo 和左右两只 Glove |
| `device_info.py` | 显示左右 Glove source，并读取 Exo 设备信息 |
| `telemetry.py` | 读取左右 Glove pose/status、Exo telemetry 和 Exo status |
| `imu.py` | 读取 Exo IMU，并提示当前 composite CDC 不提供 Glove `0x10/0x81` 原始 IMU |
| `glove_haptics.py` | 通过共享串口设置和清除左右 Glove haptics |
| `lifecycle.py` | 演示 start、一次读取、stop、close |

`telemetry.py` 和 `imu.py` 会持续运行，按 `Ctrl+C` 停止。`glove_haptics.py` 的 `--hold` 参数以秒为单位控制保持时间。

组合示例默认使用根 `Sdk` 执行整体启动、停止和关闭，并按 Glove 的 `side`
以及唯一 Exo 成员访问设备。推荐生命周期顺序是：

```python
sdk.start()
sdk.run_forever()  # 或 sdk.run_background()
sdk.stop()
sdk.close()
```

只有需要单独控制某个领域时，才使用 `glove.start()`、`glove.stop()`、`exo.start()` 或 `exo.stop()`。
`quick_start.py` 没有算法目标，启动后由无任务运行器保持等待，直到按 `Ctrl+C` 请求停止。

生命周期的完整用法请参考 [Exo 示例](../../exo/python/README.md) 和 [Glove 示例](../../glove/python/README.md) 中对应的生命周期说明。

## 常见问题

- 找不到设备：确认组合设备已连接、共享串口未被其他程序占用，并先运行 `discover_exo_glove.py`。
- 只有一个成员有输出：确认设备固件支持 Exo、左 Glove 与右 Glove 组合数据，并检查三个 source 是否均出现。
- 无法安装依赖：确认 Python 版本和安装包平台与当前系统匹配。
