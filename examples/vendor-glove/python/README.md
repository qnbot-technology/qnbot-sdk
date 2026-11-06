# Python 厂商手套示例

准备 Python 3.10+、已连接的厂商手套和本 PR 的匹配 `qnbot-sdk-core` wheel。首个发布版本尚未确定，已发布的旧包不包含此入口。无需 Glove、Exo 或算法包。

在本目录执行，将 wheel 路径和串口路径替换为实际值：

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install /path/to/qnbot_sdk_core-*.whl
python vendor_glove.py --port /dev/cu.usbserial-EXAMPLE --side left
```

Windows 使用 `.venv\Scripts\activate` 激活环境，并传入 `--port COM3`。需要其他波特率时追加 `--baudrate 115200`。

程序先调用 `start()` 打开串口并开始接收，然后输出 `read_info status=... values=...` 和 `read_parameters status=... values=...`，再输出至少 10 行 `sample=... wire_sequence=... bitmap=... timestamp_ms=... blocks=...`。`sample` 应前进；`blocks` 显示实际收到的 bit 索引和字节数。非零 `status` 为设备原始状态。时间戳可重复或为 `None`，组件未装备或未订阅时缺少相应块。

收到样本后，程序调用 `stop()` 释放串口并关闭 SDK；不调用已弃用且不执行物理连接的 `connect()`。示例不设置参数、不写入 Flash、不校时。10 秒内不足 10 帧时程序报错并关闭 SDK；检查设备当前模式、订阅表和连接。报 `missing qnbot_vendor_glove_command` 时，重新安装本 PR 的匹配 Core/native wheel。
