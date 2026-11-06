# C++ 厂商手套示例

准备 C++17 编译器、CMake 3.16+、已连接的厂商手套和本 PR 匹配的 C++ Core/native 安装包。首个发布版本尚未确定，已发布的旧包不包含此入口。无需 Glove、Exo 或算法包。

在本目录执行，将 Core 安装前缀和串口路径替换为实际值：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/core/install
cmake --build build
./build/vendor_glove --port /dev/cu.usbserial-EXAMPLE --side left
```

Windows 多配置构建使用 `cmake --build build --config Release`，运行 `build\Release\vendor_glove.exe --port COM3 --side left`。需要其他波特率时追加 `--baudrate 115200`。按安装包说明使 native 动态库位于可加载路径。

程序先调用 `start()` 打开串口并开始接收，然后输出 `read_info status=... values=...` 和 `read_parameters status=... values=...`，再输出至少 10 行 `sample=... wire_sequence=... bitmap=... timestamp_ms=... blocks=...`。`sample` 应前进；`blocks` 显示实际收到的 bit 索引和字节数。非零 `status` 为设备原始状态。时间戳可重复或为 `null`，组件未装备或未订阅时缺少相应块。

收到样本后，程序调用 `stop()` 释放串口并关闭 SDK；不调用已弃用且不执行物理连接的 `connect()`。示例不设置参数、不写入 Flash、不校时。10 秒内不足 10 帧时程序报错并清理资源；检查设备当前模式、订阅表和连接。链接报 `qnbot_vendor_glove_command` 未定义时，更新匹配 Core/native 安装包、删除旧 `build` 目录并重新构建。
