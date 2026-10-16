# QnBot SDK C++ 示例

`src/` 目录存放可以直接运行的示例源码，不存放 SDK 自身的实现代码。

本目录提供可直接构建的 C++ 客户示例，覆盖设备发现、手套姿态、有线 IMU 原始数据、
目标手输出、运行模式、触觉反馈以及采集与标定。
可运行的示例源码统一位于 `src/` 目录。

## 准备工作

- CMake 3.16 或更高版本；
- 支持 C++17 的编译器；`async_runtime` 需要 C++20；
- 当前操作系统和 CPU 架构匹配的 QnBot C++ SDK/Glove 交付包；
- 目标手输出示例所需的算法包。

从 [GitHub Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) 下载当前平台的
SDK/Glove 交付包。需要目标手输出时，先安装 QnBot CLI 0.2.0，或产品交付说明指定的
兼容版本。

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

`package.zip` 必须与当前操作系统、CPU 架构和 SDK 版本匹配。

## 构建

将 `/path/to/qnbot` 替换为解压后的 SDK CMake package 路径：

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/qnbot
cmake --build build --config Release
```

多配置生成器的可执行文件通常位于 `build/Release/`，单配置 Ninja 通常位于 `build/`。

## 快速运行

连接一只受支持的手套后运行：

```bash
./build/quick_start
```

该示例无需参数，也不加载算法包。SDK 会自动发现唯一连接的手套并持续输出姿态；按
`Ctrl+C` 停止。没有发现手套或同时发现多只手套时，先运行 `discover_gloves`，再使用支持
显式设备参数的示例。

## 示例列表

| 示例 | C++ 标准 | 是否需要手套 | 是否需要目标手算法包 | 用途 |
| --- | --- | --- | --- | --- |
| `discover_gloves` | C++17 | 是 | 否 | 查看当前可用手套 |
| `external_input_run_background` | C++17 | 否 | 是 | 由应用提供手套帧，SDK 在后台运行 |
| `external_input_manual_update` | C++17 | 否 | 是 | 由应用提供手套帧，并由应用手动更新 |
| `skeleton` | C++17 | 是 | 否 | 读取手部骨骼数据 |
| `subscription_config` | C++17 | 是 | 否 | 选择最新值或有界 FIFO，并限制订阅交付频率 |
| `algorithm_configuration` | C++17 | 是 | 是 | 使用已有骨长配置和算法参数覆盖文件 |
| `quick_start` | C++17 | 是 | 否 | 零参数读取单只手套姿态 |
| `time_sync` | C++17 | 是 | 否 | 在后台运行并定期使用外部时间源提供的 Unix 毫秒时间同步 Glove 时钟后读取姿态 |
| `imu` | C++17 | 是 | 否 | 读取有线 IMU 原始计数 |
| `manual_runtime` | C++17 | 是 | 是 | 由应用控制每次更新 |
| `foreground_runtime` | C++17 | 是 | 是 | 在当前线程持续运行 |
| `background_runtime` | C++17 | 是 | 是 | 在后台运行并主动停止 |
| `async_runtime` | C++20 | 是 | 是 | 使用 coroutine 异步读取输出 |
| `debug_trace` | C++17 | 是 | 是 | 生成诊断日志 |
| `multiple_outputs` | C++17 | 是 | 是 | 从一只手套读取多路目标输出 |
| `device_lifecycle` | C++17 | 是，两只 | 是 | 分别管理左右手设备 |
| `haptics` | C++17 | 是 | 否 | 设置并清除触觉反馈 |
| `host_capture_calibration` | C++17 | 是，两只 | 是 | 完成左右手采集并保存、应用标定结果 |

两个 external input 示例都需要显式提供算法包，默认由 SDK 在终端提示并接收采集确认。后台示例启动后会返回，示例继续提供手套帧；手动更新示例适合由应用控制每次更新的循环。标定完成后才会出现对应的目标手输出。需要由应用处理提示和确认时，添加 `--capture-interaction external`；默认值为 `auto_terminal`。使用算法包的示例必须显式传入 `--package-id`，两个 external input 示例也不例外。

手动更新示例会等待每次更新完成，再读取当前输出。

手动更新示例不会用 `has_next_task` 判断是否需要等待：`update()` 之后直接调用 `update.sleep()`；在 `update.sleep()` 返回 `no_tasks`、没有可等待的工作时，示例会短暂让出（10 毫秒）以避免忙轮询，然后继续下一轮 `update()`。外部输入示例在 `update()` 之前先 `push_frame()`，因此不会在应用还欠下一帧输入时阻塞。

当手套帧由应用提供时（两个 external input 示例），`update()` 之后调用 `sleep()` 可能在应用尚未提供下一帧时就返回 `qnbot::SleepResult::no_tasks`：此时仍有进行中的采集或标定阶段需要应用自己推送的下一帧，SDK 无法代替应用推进。应用循环必须处理该返回值（示例中短暂休眠后再推送下一帧），不能假设 `sleep()` 一定阻塞，也不能用 `update.has_next_task` 作为是否调用 `sleep()` 的门禁。

输入快于算法处理能力时，目标手输出按最新状态收敛：过载不会持续积压，也不会逐帧交付每一个中间结果。SDK 默认不限制推进频率或 CPU 占用，推进速度由输入到达和算法处理能力决定；需要限制节奏时由应用自己的循环控制 `update()` 的调用频率。

## 标定策略

普通示例（`quick_start`、`foreground_runtime`、`background_runtime`、`async_runtime`、
`manual_runtime` 以及 `device_lifecycle`、`multiple_outputs`、`debug_trace`）使用默认的自动标定。
`glove.start()` 不传参数时，SDK 会在首次推进（`update()`、`run_forever()` 或
`run_background()`）时自动完成所需的采集与标定，并在需要用户操作时给出终端提示；这些
示例不调用 `start_capture` 或 `start_calibration`。`manual_runtime` 里的“手动”只表示由应用
控制每次更新，标定仍由 SDK 自动组织。

需要完全不标定的原始数据流程时，显式选择 `qnbot::CalibrationMode::none`，例如
`device.start(qnbot::CalibrationMode::none)` 或 `glove.start(qnbot::CalibrationMode::none)`：
它允许原始 `start_capture`，但会拒绝标定以及需要标定的目标输出。不加载算法包、也不创建
标定的示例（`discover_gloves`、`skeleton`、`imu`、`haptics`）使用默认策略运行即可。

`host_capture_calibration` 演示由上位机显式控制同一流程，启动顺序为：`glove.start()`、
`device.start(qnbot::CalibrationMode::manual)`、`start_capture`、`start_calibration`。对每个
要手动管理的设备都选择 `qnbot::CalibrationMode::manual`，其余设备保持默认策略。未启动设备
就调用 `start_capture` 的旧顺序会失败，并提示先启动设备；SDK 不会替应用启动硬件或改变已
选择的标定策略。

## 常用命令

无需连接手套：

```bash
./build/external_input_run_background --package-id <package-id>
./build/external_input_manual_update --package-id <package-id>
```

真实手套：

```bash
./build/discover_gloves
./build/skeleton --port /dev/ttyUSB0 --side left
./build/quick_start
./build/time_sync
./build/imu --port /dev/ttyUSB0 --side left
./build/manual_runtime --port /dev/ttyUSB0 --side left --package-id <package-id> --updates 100
./build/foreground_runtime --port /dev/ttyUSB0 --side left --package-id <package-id>
./build/background_runtime --port /dev/ttyUSB0 --side left --package-id <package-id> --seconds 10
./build/async_runtime --port /dev/ttyUSB0 --side left --package-id <package-id> --samples 10
./build/debug_trace --port /dev/ttyUSB0 --side left --package-id <package-id> --detail full --sample-rate 10
./build/multiple_outputs --port /dev/ttyUSB0 --side left --package-id <package-id>
./build/device_lifecycle --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1 --package-id <package-id> --updates 10
./build/haptics --port /dev/ttyUSB0 --side left --hold 1
./build/host_capture_calibration --package-id <package-id>
```

`time_sync` 使用后台运行模式，每隔 1 秒重新获取一次外部时间源的 Unix 毫秒时间戳并调用
`glove.sync_time(reference_time_ms)`。传入参数的单位是毫秒，不是纳秒；按 `Ctrl+C` 停止。

`host_capture_calibration` 可同时接收多个包，完成左右手采集后选择一个已配置 target，保存并
应用标定结果：

```bash
./build/host_capture_calibration \
  --package-id <package-id-1> \
  --package-id <package-id-2>
```

程序会分别选择左右手设备，并查询当前设备的采集计划是否已有兼容、可复用的已保存采集
数据。需要忽略这些可复用数据并重新采集时增加 `--force`。只标定 SDK 内置 Skeleton 时
使用固定 ID：

```bash
./build/host_capture_calibration \
  --package-id qnbot_hand.dynamic_openxr_hand
```

使用多配置生成器时，将命令中的 `./build/` 替换为 `./build/Release/`。

## 参数说明

- `--port`：手套串口；使用 `--port` 的参数化示例必须显式提供串口。
- `--side`：手套物理侧，取值为 `left` 或 `right`。
- `--package-id`：已经安装的目标手算法包 ID；该参数不会安装算法包。
- `--force`：`host_capture_calibration` 忽略可复用采集并重新采集。
- `--detail`、`--sample-rate`：控制 `debug_trace` 的诊断日志详细程度和采样率。

`debug_trace` 生成的诊断日志固定写入 `<QNBOT_SDK_HOME>/logs`。如需自定义 SDK 数据根目录，优先设置 `QNBOT_SDK_HOME`；旧变量 `QNBOT_HOME` 仍兼容。

缺少标定时按终端提示操作，完成后示例会开始输出目标关节结果。

使用自动终端提示的示例默认跟随系统语言。运行前设置 `QNBOT_SDK_LANGUAGE=en` 可选择英文，
设置 `QNBOT_SDK_LANGUAGE=zh` 可选择中文；英文提示缺失时显示原始提示。手动展示采集进度的
示例会优先显示英文提示，缺失时使用原始提示。

当 SDK 能将该端口识别为受支持的有线直连手套，且同时显式提供 `--port`（配置字段
`port`）和 `--side`（配置字段 `side`）时，无法报告设备身份的旧版有线直连手套仍可
连接，并发出“身份未验证”告警。断线恢复只重试该串口，不能确认仍是同一只物理手套。
自动发现、只提供其中一个参数、按 `serial_number` 选择、无线 Receiver 以及
无法识别端口类型时仍要求设备身份校验。

`skeleton` 在 SDK 创建后、`start()` 前配置 Skeleton；启动前重复配置是幂等的。

`imu` 先注册有线 IMU callback 再调用 `start()`；`subscribe()` 可以在启动前或启动后调用。数据在 `start()` 之后才可能产出：`start()` 返回后需要等第一条有效帧，`latest()` 在该帧发布前返回 `std::nullopt`，而不是启动失败。它输出有符号 16 位 `gyroscope_raw` 和 `accelerometer_raw` 原始计数，SDK 不做单位换算、校准、滤波、偏置修正或融合。示例只支持 USB 串口直连手套的有线 IMU 原始数据，不支持无线 Receiver 手套的 IMU 数据；有线 IMU 原始数据不需要目标手算法包。异步程序可使用 `imu.next()` 等待样本。

## 订阅交付配置

`subscription_config` 默认以最新值模式读取姿态，最高每秒交付 30 次，运行 10 秒后停止。无需算法包。

```bash
./build/subscription_config --port /dev/ttyUSB0 --side left
./build/subscription_config --port /dev/ttyUSB0 --side left --mode fifo --capacity 128 --max-rate-hz 30 --seconds 10
```

- `--mode latest`：处理不及时只保留最新待处理值，适合界面显示。
- `--mode fifo`：按顺序交付，最多保留 `--capacity` 个待处理值；满时丢弃最旧值，不保证每帧都交付。
- `--max-rate-hz`：最高交付频率，默认 `30`；不会改变算法计算频率。
- `--seconds`：后台运行秒数，默认 `10`。

程序持续打印姿态和样本序号；最新值模式下序号可能跳跃。实际输出频率还取决于设备数据到达速度。

## 骨长与算法参数配置

`algorithm_configuration` 将已有骨长配置的使用策略与算法参数覆盖文件放在同一示例中。
先通过 QnBot CLI 安装产品交付的兼容算法包；准备骨长配置及覆盖文件时遵循该算法包的产品说明。
该示例使用配置，不生成骨长配置，也不执行骨长标定。

```bash
./build/algorithm_configuration --port /dev/ttyUSB0 --side left --package-id '<package-id>' --use-bone-config false
./build/algorithm_configuration --port /dev/ttyUSB0 --side left --package-id '<package-id>' --operator-id default --use-bone-config true --algorithm-config-path config/override.yaml
```

- `--use-bone-config auto`：默认值，存在对应操作者、手侧的骨长配置时使用，否则采用算法默认值。
- `--use-bone-config true`：要求对应配置存在且有效；`false` 忽略该配置。
- `--operator-id`：骨长配置对应的操作者，默认 `default`；无需传入骨长文件路径。
- `--algorithm-config-path`：可选的算法参数覆盖文件，相对路径从程序启动目录解析；字段按算法包说明填写。省略时使用算法默认参数。
- `--seconds`：启动及所需自动标定完成后的读取秒数，默认 `10`。

缺少动作采集时按终端提示操作，完成后输出目标关节数据。骨长配置不等同于该动作采集与标定流程。
配置文件修改后重新运行示例；是否需要重新标定以算法包说明为准。要求使用配置却找不到文件时，先确认操作者及手侧与准备的配置一致。

## 常见问题

- CMake 找不到 `qnbot_glove`：确认 `CMAKE_PREFIX_PATH` 指向解压后的匹配 SDK 包。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并运行 `discover_gloves`。
- 找不到算法包：通过 CLI 安装正确的产品算法包，并检查 `--package-id`。
- 没有目标手输出：确认已完成终端提示的标定，并保持手套连接。
- 没有 IMU 输出：确认使用支持有线 IMU 的 USB 直连手套，并在 `start()` 后等待第一条有效帧。

仍无法解决时，请保留构建命令、运行命令和完整错误信息并联系 QnBot 对接人员。
