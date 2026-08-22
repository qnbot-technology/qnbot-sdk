# QnBot C++ SDK 示例

本目录提供可以直接构建的 C++ 示例，覆盖快速启动、设备发现、retargeting、
calibration、运行模式和触觉反馈。当前示例支持 macOS 与 glibc Linux。

`src/` 中只包含可独立运行的示例源码和示例专用 helper header，不包含 SDK 实现源码。
示例通过已安装的公开 QnBot SDK CMake package 构建。

## 5 分钟快速开始

### 1. 构建示例

这些 Glove 示例依赖已安装的 `qnbot_glove 0.3.0` CMake package；它会传递依赖兼容的
`qnbot_core >=0.3.0,<1.0.0`。使用单配置 Ninja 生成器时，在 configure 阶段选择
Release，生成的程序直接位于 `build/`：

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/qnbot
cmake --build build --config Release
```

使用 Ninja Multi-Config 生成器时，在 build 和运行阶段选择配置；可执行文件位于对应的
配置子目录：

```bash
cmake -S . -B build-multi -G "Ninja Multi-Config" \
  -DCMAKE_PREFIX_PATH=/path/to/qnbot
cmake --build build-multi --config Release
./build-multi/Release/external_input
```

### 2. 安装 retargeting 算法包

仅 macOS arm64 交付的 SDK 内置动态 OpenXR 骨骼由 `skeleton()` 直接提供，不需要
另行安装算法包。其他平台当前不携带该私有 runtime，选择 `skeleton()` 会明确报告能力
不可用；`pose()` 和客户配置的 `output()` 仍可使用。
Core 和 Glove C++ package 不内置客户目标手型的 retargeting 算法。运行配置了
`TargetAlgorithm` 的示例前，需要安装目标手配套的动态 C ABI 算法包。SDK 会从
`QNBOT_ALGORITHM_PATH` 和平台默认 QnBot 算法目录发现它；算法包位于其他目录时，将
环境变量指向包根目录或它的父目录：

```bash
export QNBOT_ALGORITHM_PATH=/opt/qnbot/algorithms
```

真实手套 runtime、debug trace 和 device lifecycle 示例与 Python 示例一致，默认
target 实例名为 `openxr_hand`，算法 ID 为
`qnbot.hand.openxr_hand.retargeting`。可用 `--target-name` 修改实例名，用
`--algorithm-id` 选择 SDK 已发现的其他兼容算法；参数不负责安装或改变发现路径。
`multiple_outputs` 默认使用 `qnbot.hand.glove_to_hand.retargeting` 驱动 `primary` 和
`backup` 两个公开实例。所有这些示例都消费命名 output channel，并打印 retargeting 的
sequence、target 和完整 joints。

`TargetConfig.name` 和 `--target-name` 表示调用方定义的 output 实例名，
`device.output(name)` 按实例名选择 channel。算法 ID 和可选版本负责选择算法，算法包的包内资源
名称仅供 native 实现使用，不会生成或限制公开实例名。

### 3. 运行 Quick Start

连接一只受支持的手套，然后运行：

```bash
./build/quick_start
```

`quick_start` 只保留无法推导的信息：

```cpp
qnbot::TargetConfig target;
target.algorithms = {qnbot::TargetAlgorithm{
    "qnbot.hand.glove_to_hand.retargeting"}};

qnbot::SdkConfig config;
config.devices = {qnbot::GloveConfig{}};
config.targets = {target};
qnbot::Sdk sdk(config);
```

连接完成后 SDK 从物理设备和算法包推导 type、side 与 source；未配置的公开实例名保持
省略，而不是采用算法包内资源名称。

`quick_start` 不显式设置 `config.algorithms.calibration`，因此使用默认的按需终端标定、
默认 operator 和兼容历史记录。首次运行缺少兼容记录时，按照终端提示完成
calibration，随后开始输出 retargeting 前的 pose 和 retargeting 后的 output。

## 选择示例

| 示例 | 标准 | 算法包 | 用途 |
| --- | --- | --- | --- |
| `discover_gloves` | C++17 | 否 | 在创建 SDK 前发现已接入手套，并用所选 `port` 和 `hand` 构建配置 |
| `quick_start` | C++17 | 是 | 自动发现唯一手套，从设备身份和算法包推导 side、source 与 target |
| `external_input` | C++17 | 否 | 无硬件逐帧输入，演示 pose、status、output、订阅、next、取消、health 和关闭 |
| `skeleton` | C++17 | 内置 | 自动发现唯一手套并读取动态 OpenXR 骨骼 |
| `manual_runtime` | C++17 | 是 | 把 `update()` 与 `UpdateResult::sleep()` 接入应用自有循环 |
| `foreground_runtime` | C++17 | 是 | 在当前线程运行，并由 `sigwait` 协调线程安全处理 Ctrl+C |
| `background_runtime` | C++17 | 是 | 后台运行、停止与 join |
| `async_runtime` | C++20 | 是 | 使用 `co_await qnbot::coro::next(...)` 异步读取真实 retargeting output |
| `debug_trace` | C++17 | 是 | 采样记录 serial、glove、calibration、retargeting、output 和 buffer trace |
| `multiple_outputs` | C++17 | 是 | 一只手套驱动 `primary` 和 `backup` 两个 target 实例 |
| `device_lifecycle` | C++17 | 是 | 独立启动左右手套，并停止、断开右手设备后检查 domain health |
| `haptics` | C++17 | 否 | 设置、读取并清除触觉状态 |

只有 `async_runtime` 要求 C++20；其余十个示例保持 C++17。

动态骨骼示例源码见 [`skeleton.cpp`](src/skeleton.cpp)。

## 运行命令

### 无硬件验证

```bash
./build/external_input
./build/manual_runtime --external --updates 10
./build/background_runtime --external --seconds 1
./build/foreground_runtime --external
```

`--external` 是 C++ 示例额外保留的无硬件验证路径，使用 pass-through target，因此不
要求安装 retargeting 算法包；`--target-name` 仍可指定 output channel 名称，但
`--algorithm-id` 不适用于该模式并会被拒绝。

### 真实手套运行

```bash
./build/discover_gloves
./build/skeleton
./build/quick_start
./build/manual_runtime --port /dev/ttyUSB0 --side left --updates 100
./build/foreground_runtime --port /dev/ttyUSB0 --side left
./build/background_runtime --port /dev/ttyUSB0 --side left --seconds 10
./build/async_runtime --port /dev/ttyUSB0 --side left --samples 10
./build/debug_trace \
  --port /dev/ttyUSB0 --side left --detail full --sample-rate 10
./build/multiple_outputs \
  --port /dev/ttyUSB0 \
  --side left \
  --algorithm-id qnbot.hand.glove_to_hand.retargeting
./build/device_lifecycle \
  --left-port /dev/ttyUSB0 \
  --right-port /dev/ttyUSB1 \
  --updates 10
./build/haptics --port /dev/ttyUSB0 --side left --hold 1
```

`pose()` 是手套原始姿态，`skeleton()` 是 SDK 内置的动态 OpenXR 骨骼并输出 24 个
active joints，`output()` 是客户通过 target 配置的 retargeting 输出。需要操作内置骨骼的标定流程时，沿用现有
标定方法并传入 `qnbot::TargetName::skeleton`；普通 target 仍传入自己的名称。
当前 skeleton 示例仅在内置 runtime 可用的 macOS arm64 交付包上运行。

### 只校验配置和参数

以下命令不会打开设备或选择 runtime channel：

```bash
./build/manual_runtime --validate --port /dev/ttyUSB0 --side left
./build/foreground_runtime --validate --port /dev/ttyUSB0 --side left
./build/background_runtime --validate --port /dev/ttyUSB0 --side left --seconds 1
./build/async_runtime --validate --port /dev/ttyUSB0 --side left --samples 10
./build/debug_trace --validate --port /dev/ttyUSB0 --side left
./build/multiple_outputs --validate --port /dev/ttyUSB0 --side left
./build/device_lifecycle --validate \
  --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1
./build/haptics --validate --port /dev/ttyUSB0 --side left --hold 0
```

例如使用已交付算法包校验名为 `primary` 的输出实例配置：

```bash
./build/manual_runtime --validate \
  --port /dev/ttyUSB0 \
  --side left \
  --target-name primary \
  --algorithm-id qnbot.hand.glove_to_hand.retargeting
```

所有显式传入 `--port` 的单手套示例都必须同时传入物理侧别
`--side left|right`，没有默认值。SDK 会在连接时将该配置与设备上报的 side 核对，
不匹配时直接失败。真实硬件运行时去掉 `--validate`。运行模式示例的 `--external` 与
`--port` 互斥，`--external` 不接受 `--side`。

## 自动配置与固定部署

### 单手套自动配置

默认 `SerialConnection` 自动扫描，并且只在找到唯一支持的手套时连接。`connect()`
读取 `DeviceInfo`，冻结设备序列号和物理 side；已选算法的 manifest、安装包资源和唯一
兼容设备共同推导 target type、side 与 source，省略的公开实例名保持省略。任何候选为
空或仍有多个候选时都会失败，不会任意选择。

`Sdk` 构造函数只校验并保存声明，不扫描或打开硬件。同步调用
`glove_domain.connect()` 成功后，才能选择设备、channel、订阅、标定 channel 或输出：

```cpp
auto glove_domain = sdk.glove();
glove_domain.connect();
auto device = glove_domain.device();
auto subscription = device.pose().subscribe(on_pose);
```

连接过程会完整构建候选 runtime，并在所有步骤成功后一次性发布。发现、身份校验、
target 推导、route 构建或 runtime 连接任一步失败时，都不会暴露半完成的设备或 route。

连接后，各层使用同一组 canonical ID。例如右手手套上报
`canonical_sn="QN-R-000042"` 时，source ID 是 `glove.QN-R-000042.right`；未配置实例名
的 target ID 是 `hand.~.right`。配置中的 glove `name` 仍作为 facade
的选择别名；日志、诊断、topic、标定记录和公开 runtime API 都使用 canonical ID，
不再区分逻辑 ID 与 execution ID。

### 双手套自动配置

一只左手套和一只右手套可只声明 side，无需显式设置 `connection`：

```cpp
qnbot::GloveConfig left;
left.side = qnbot::Side::left;

qnbot::GloveConfig right;
right.side = qnbot::Side::right;

qnbot::SdkConfig config;
config.devices = {left, right};
```

SDK 会一次发现候选并按 side 唯一分配，串口枚举顺序不会决定左右数据流。每个物理设备
最多分配一次；缺少某一侧、同侧候选不唯一，或发现后正式连接时 SN/side 发生变化时，
整体连接失败，不发布部分设备、calibration 或 retargeting route。两只同侧手套应分别
显式配置 `port` 或 `serial_number`。`ExternalConnection` 仍需显式配置，且 external
input 没有身份握手，因此必须配置 side。

### 显式固定部署

固定部署可显式配置 `port` 或 `serial_number`，并按需提供 glove `side`、target
`type`、`name`、`side` 或 `source`。这些值用于缩小并校验候选，不能覆盖设备上报的
物理身份：

```cpp
qnbot::SerialConnection serial;
serial.serial_number = "GLOVE-001";

qnbot::GloveConfig glove;
glove.name = "primary";
glove.side = qnbot::Side::right;
glove.connection = serial;

qnbot::TargetConfig target;
target.type = qnbot::TargetType::hand;
target.name = "wuji_hand";
target.side = qnbot::Side::right;
target.source = qnbot::DeviceSelector{
    "glove", "primary", qnbot::Side::right};
target.algorithms = {qnbot::TargetAlgorithm{
    "qnbot.hand.glove_to_hand.retargeting", "1.0.0"}};

qnbot::SdkConfig config;
config.devices = {glove};
config.targets = {target};
qnbot::Sdk sdk(config);
```

如果端口才是稳定 locator，把 `serial.serial_number` 换成：

```cpp
serial.port = "/dev/ttyUSB0";
```

## Calibration

配置 retargeting target 且缺少兼容标定记录时，常规程序仍按
`connect()`、`start()`、`run_forever()` 运行。默认 `auto_terminal` 由 SDK
terminal driver 在交互式前台 runner 中显示 prompt、接收确认并报告进度；C++ 示例
不实现终端输入循环。

`config.algorithms.calibration.operator_id` 默认为 `"default"`，用于隔离不同操作员的
标定记录。`config.algorithms.calibration.force` 默认为 `false`；设为 `true` 时，当前
SDK 实例会忽略启动前已有记录并为每条需要标定的 route 强制执行一次。成功后，同一
实例在 stop 或后续重连时复用新结果；新建且仍配置 `force=true` 的 SDK 实例会再次
强制标定。

只有需要自定义存储、external interaction、强制重标定或区分 operator 时，才需要显式
配置 calibration。

### External calibration control

GUI 或远程宿主使用 `external` 模式，订阅 `calibration_progress(name)`，并将当前
`request_id` 原样传给 `calibration_control(name)` 的 `confirm`、`skip` 或
`cancel`。wrapper 不负责 stage、采样、复用、持久化或激活。

外部控制必须按 progress state 提供：

| Progress state | 有效 UI action |
| --- | --- |
| `awaiting_confirmation` | `confirm` 与可选 stage 的 `skip` 二选一；也可 `cancel` |
| `collecting` | 保留当前 `request_id`，只提供 `cancel` |
| 最终 stage 的 `persisting` 或 `activating`，且仍有 request ID | 保留同一 `request_id`，只提供 `cancel` |

`confirm` 后，同一请求在采集期间仍然有效；最终 stage 还会保持到持久化和激活结束，
以便用户继续取消。wrapper 不根据这些状态推进 workflow，只映射 action。

## 设备发现

### 创建配置前发现手套

`qnbot::discover_gloves()` 不需要 `Sdk` 实例，返回当前已连接受支持手套的快照。端口被
其他进程占用时记录仍会保留 `port`，身份字段为空，并通过
`GloveDiscoveryErrorCode::port_busy`（原生稳定码 `PORT_BUSY`）报告错误。成功记录的
`port` 和 `hand` 可直接用于构造 `GloveConfig` 与 `SerialConnection`。设备插拔后应
重新发现，因为返回值不是持续更新的设备列表。

## 生命周期与运行模式

C++ 必须保留返回的 RAII `Subscription`，否则临时对象析构时会立刻取消订阅。普通示例
让它存活到最终关闭；`glove.close()` 会回收仍然有效的 Glove subscriptions，因此关闭
前不再逐个调用 `unsubscribe()`。只有在 domain 继续运行、但需要提前停止某个 callback
时才显式调用 `unsubscribe()`。

`manual_runtime` 和 `background_runtime` 会完整执行对应运行模式后自行退出。
`foreground_runtime` 会等待 Ctrl+C，并通过 `sigwait` 协调线程请求停止和显式关闭。
它会在创建 SDK 前阻塞 `SIGINT`，再由专用 `sigwait` 协调线程调用 `request_stop()`；
SDK 调用不会发生在异步信号处理函数中。

`foreground_runtime` 和 `background_runtime` 会在运行期间打印 `running health`，并在
主动停止完成后打印 `stopped lifecycle health`。停止遥测后设备会进入 `stale=true`，
因此即使 `warning_count=0`、`error_count=0`，共享健康值也可能是 `ok=false`；这是预期
的停止生命周期状态，不表示运行失败。判断运行期是否健康应查看停止请求前的
`running health`，停止后的输出用于确认设备的 `connected`/`stale` 状态。
