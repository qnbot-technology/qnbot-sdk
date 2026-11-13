# QnBot SDK C++ 示例

`src/` 目录存放可以直接运行的示例源码，不存放 SDK 自身的实现代码。

本目录提供可直接构建的 C++ 客户示例，覆盖设备发现、手套姿态、IMU 原始数据、
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

## 无硬件设备选择示例

`device_selection` 使用 Glove **1.5.0** 及配套 Core **1.5.0** 交付包。
请通过产品交付渠道获取与平台匹配的版本；不要将不同版本的头文件和库混用。
1.4.0 不支持本示例使用的名称与左右手联合选择。使用 1.4.0 时构建会跳过此示例和两个配置示例，
其他示例仍按上面的步骤构建。取得兼容交付包后，单独构建并运行：

```bash
cmake -S . -B build-selection -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/compatible/qnbot
cmake --build build-selection --config Release --target device_selection
./build-selection/device_selection
```

无需连接手套、安装算法包或传入参数。程序配置 `primary` 左右手和 `backup` 左手，
分别输出三次成功选择得到的 `source_id`，接着输出 `expected ambiguity: name=primary`
和 `expected ambiguity: side=left`，最后输出 `device selection complete` 并退出。
这些歧义提示是预期结果，不表示运行失败。

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
| `device_selection` | C++17 | 否 | 否 | 按名称和左右手选择三台外部输入设备，观察歧义提示 |
| `discover_gloves` | C++17 | 是 | 否 | 查看当前可用手套 |
| `device_info` | C++17 | 是 | 否 | 读取手套设备信息和连接类型 |
| `external_input_run_background` | C++17 | 否 | 是 | 由应用提供手套帧，SDK 在后台运行 |
| `external_input_manual_update` | C++17 | 否 | 是 | 由应用提供手套帧，并由应用手动更新 |
| `skeleton` | C++17 | 是 | 否 | 读取手部骨骼数据 |
| `subscription_config` | C++17 | 是 | 否 | 选择最新值或有界 FIFO，并限制订阅交付频率 |
| `algorithm_configuration` | C++17 | 是 | 是 | 使用已有骨长配置和算法参数覆盖文件 |
| `quick_start` | C++17 | 是 | 否 | 零参数读取单只手套姿态 |
| `time_sync` | C++17 | 是 | 否 | 在后台运行并定期使用外部时间源提供的 Unix 毫秒时间同步 Glove 时钟后读取姿态 |
| `imu` | C++17 | 是 | 否 | 读取手套的带单位 IMU 数据 |
| `emf` | C++17 | 是，支持 EMF 的手套 | 否 | 读取与 pose 配对的原始 EMF 矩阵和实际通道信息 |
| `manual_runtime` | C++17 | 是 | 是 | 由应用控制每次更新 |
| `foreground_runtime` | C++17 | 是 | 是 | 在当前线程持续运行 |
| `background_runtime` | C++17 | 是 | 是 | 在后台运行并主动停止 |
| `async_runtime` | C++20 | 是 | 是 | 使用 coroutine 异步读取输出 |
| `debug_trace` | C++17 | 是 | 是 | 生成诊断日志 |
| `multiple_outputs` | C++17 | 是 | 是 | 从一只手套读取多路目标输出 |
| `device_lifecycle` | C++17 | 是，两只 | 否 | 多只手套的独立启停：停止、重启其中一只，其他手套继续输出 |
| `haptics` | C++17 | 是 | 否 | 设置并清除触觉反馈 |
| `host_capture_calibration` | C++17 | 是，两只 | 否，可选目标包 | 完成左右手采集并保存、应用标定结果 |
| `pressure` | C++17 | 是，HMP-MX | 否 | 读取整帧压力摘要和所选点 |

两个 external input 示例都需要显式提供 `--package-id`，默认由 SDK 在可交互终端提示并接收采集确认（`auto_terminal`）。
后台示例启动后会返回并继续推帧，**返回不代表标定完成**；按终端提示完成动作期间也要持续提供帧，
完成标定后才会出现目标手输出。手动更新示例由应用循环持续推帧并更新，仍可使用 SDK 的自动终端引导。
需要由应用处理提示和确认时，添加 `--capture-interaction external`；该选项不会代替应用供帧。
使用算法包的示例必须显式传入 `--package-id`，两个 external input 示例也不例外。

手动更新示例会等待每次更新完成，再读取当前输出。

手动更新示例不会用 `has_next_task` 判断是否需要等待：`update()` 之后直接调用 `update.sleep()`；在 `update.sleep()` 返回 `no_tasks`、没有可等待的工作时，示例会短暂让出（10 毫秒）以避免忙轮询，然后继续下一轮 `update()`。外部输入示例在 `update()` 之前先 `push_frame()`，因此不会在应用还欠下一帧输入时阻塞。

当手套帧由应用提供时（两个 external input 示例），`update()` 之后调用 `sleep()` 可能在应用尚未提供下一帧时就返回 `qnbot::SleepResult::no_tasks`：此时仍有进行中的采集或标定阶段需要应用自己推送的下一帧，SDK 无法代替应用推进。应用循环必须处理该返回值（示例中短暂休眠后再推送下一帧），不能假设 `sleep()` 一定阻塞，也不能用 `update.has_next_task` 作为是否调用 `sleep()` 的门禁。

输入快于算法处理能力时，目标手输出按最新状态收敛：过载不会持续积压，也不会逐帧交付每一个中间结果。SDK 默认不限制推进频率或 CPU 占用，推进速度由输入到达和算法处理能力决定；需要限制节奏时由应用自己的循环控制 `update()` 的调用频率。

## 标定策略

普通示例（`quick_start`、`foreground_runtime`、`background_runtime`、`async_runtime`、
`manual_runtime` 以及 `multiple_outputs`、`debug_trace`）使用默认的自动标定。
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

`device_selection` 按上面的专用步骤构建后，也可将构建目录改为 `build` 并运行
`./build/device_selection`。其他无需连接手套的示例：

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
./build/device_lifecycle --left-port /dev/ttyUSB0 --right-port /dev/ttyUSB1 --updates 10
./build/haptics --port /dev/ttyUSB0 --side left --hold 1
./build/host_capture_calibration --package-id <package-id>
```

`async_runtime` 持续打印收到的目标关节输出；完成指定样本后停止并释放资源。后台运行提前失败时，示例输出错误并退出，不再等待下一条数据；暂时没有数据时仍继续等待。读取或处理输出失败时也会完成清理。

`time_sync` 使用后台运行模式，每隔 1 秒重新获取一次外部时间源的 Unix 毫秒时间戳并调用
`glove.sync_time(reference_time_ms)`。传入参数的单位是毫秒，不是纳秒；按 `Ctrl+C` 停止。

`host_capture_calibration` 不传包参数时使用内置骨骼，无需另外安装算法包：

```bash
./build/host_capture_calibration
```

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
- `--package-id`：已经安装的目标手算法包 ID；该参数不会安装算法包。`host_capture_calibration` 不传此参数时使用内置骨骼，其他需要目标包的示例仍须显式提供。
- `--capture-interaction`：两个外部输入示例的交互方式，默认 `auto_terminal`；自带界面或无交互终端时选择 `external`。
- `--force`：`host_capture_calibration` 忽略可复用采集并重新采集。
- `--detail`、`--sample-rate`：控制 `debug_trace` 的诊断日志详细程度和采样率。

`debug_trace` 生成的诊断日志固定写入 `<QNBOT_SDK_HOME>/logs`。如需自定义 SDK 数据根目录，优先设置 `QNBOT_SDK_HOME`；旧变量 `QNBOT_HOME` 仍兼容。

缺少标定时按终端提示操作，完成后示例会开始输出目标关节结果。

以下八手势池流程适用于包含该更新的 SDK。当前安装示例指定的 1.5.0 不包含此更新，其动作仍由算法包声明；首个包含八手势池的发布版本尚未确定。

首次按提示完成八个动作，每个动作保持到采满 30 帧：五指自然并拢、五指自然张开、四指根关节弯曲约 90 度、自然握拳，以及拇指分别与食指、中指、无名指、小指指腹轻触。八步全部完成后保存采集。

同一操作者、手套和手侧下，再次运行或切换目标包会复用兼容采集，不重复摆已采动作；新目标仍会计算自己的标定结果。使用 `--force` 才重新采集全部八步。升级前保存的采集若不兼容，程序会重新请求这八步；完成一次后恢复正常复用。`host_capture_calibration` 示例逐个确认计划中的动作。

包含步骤选择更新的 SDK 支持在该示例的等待确认提示中输入 `stage <stage_id>`，例如 `stage pinch_little`。步骤 ID 和当前状态会列在提示上方；可选择待采动作或重采已完成动作，按 Enter 确认采集，输入 `n` 取消。跳转不算采集完成，重采会替换该动作的数据；其他已完成动作保留，全部必采动作仍需采齐。首次包含此更新的发布版本未定，使用这项交互前升级到包含该更新的匹配 SDK。

完整复用当前采集数据时，已有的兼容标定结果保持不变。包含取消结果修复的 SDK 在标定会话被取消后丢弃本次新结果，保留上一份兼容的有效标定；没有旧结果时，下次运行仍需完成标定。首个包含该修复的发布版本尚未确定，Core/Glove 1.5.0 仍可能保留已取消会话的新结果。
使用 `--force` 成功完成新采集后，需要基于新数据完成标定才能恢复目标输出。

使用自动终端提示的示例默认跟随系统语言。运行前设置 `QNBOT_SDK_LANGUAGE=en` 可选择英文，
设置 `QNBOT_SDK_LANGUAGE=zh` 可选择中文；英文提示缺失时显示原始提示。手动展示采集进度的
示例会优先显示英文提示，缺失时使用原始提示。

当 SDK 能将该端口识别为受支持的有线直连手套，且同时显式提供 `--port`（配置字段
`port`）和 `--side`（配置字段 `side`）时，无法报告设备身份的旧版有线直连手套仍可
连接，并发出“身份未验证”告警。断线恢复只重试该串口，不能确认仍是同一只物理手套。
自动发现、只提供其中一个参数、按 `serial_number` 选择、无线 Receiver 以及
无法识别端口类型时仍要求设备身份校验。

`skeleton` 在 SDK 创建后、`start()` 前配置 Skeleton；启动前重复配置是幂等的。

在 macOS 上，首次请求 Skeleton 可能需要一次性准备时间；请像示例一样在启动设备前完成，
再进入需要及时响应的循环。仅做设备选择或读取原始姿态时无需请求 Skeleton。

`imu` 需要一只支持 IMU 输出的手套，不需要目标手算法包。程序按通道输出 id、位置、有效性、fresh，以及 raw_counts 或 gyro_mdps/accel_mg 数值；无效与缺失值分别显示，按 `Ctrl+C` 停止。启动成功后仍须等待第一条有效样本。

### Helix USB CDC

连接 Helix，安装支持 Helix 的配套 SDK 构建，再运行设备信息示例。将 `/dev/ttyACM0` 和 `left` 替换为实际端口和手别。

```bash
./build/device_info --port /dev/ttyACM0 --side left
```

终端显示设备序列号、型号、固件版本、手别和连接类型。只连接这一只手套，再运行 `./build/quick_start`。终端持续显示姿态序列号和 `fingertips=5`，按 Ctrl+C 停止。

使用 Helix 时，`imu()` 通道直接输出多路带单位数据。

### 无线 IMU（待发布）

无线 IMU 需要包含此功能的 SDK 和支持同手姿态/IMU 组合输出的配套手套、Receiver 固件；首个 SDK 发布版本未定，1.7.0 不支持。安装产品交付说明指定的匹配版本后，将端口替换为 Receiver 端口，并选择在线手套：

```bash
./build/imu --port /dev/ttyACM0 --side left
```

预期持续输出所选手套的 IMU 数据和单位标签。没有 IMU 输出时，确认固件支持组合输出、所选手套在线，并检查启动错误；只有姿态的设备不会产生 IMU。不要同时运行多个示例进程争用同一 Receiver 端口。

### 触觉ENF定位手套（VetraGlove HMP-MX）

安装同一版本且支持触觉ENF定位手套的 Core 和 Glove SDK。连接一只按出厂设置主动上报压感的 VetraGlove HMP-MX 手套，在交互式终端运行，并将示例端口替换为实际串口。

**460 点压感是 HMP-MX 独有的通道：只有这只手套提供 `pressure()`，Nano、Pro 和 Receiver 手套没有该通道。**

```bash
cmake -S . -B build-pressure -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/qnbot \
  -DQNBOT_BUILD_PRESSURE_EXAMPLE=ON
cmake --build build-pressure --config Release --target pressure
./build-pressure/pressure --port /dev/ttyACM0
```

程序只读取压感，每秒最多显示一帧 `pressure`。`points=460` 是测点数；`raw_min`、`raw_mean`、`raw_max` 汇总全部测点的原始计数（0–255），`peak_index` 是最大原始值首次出现的索引。`point_index`、`point_raw`、`point_estimated_kpa` 是选中测点的索引、原始计数和同一帧的 kPa 估计，默认选择峰值点。指定 `--point 459` 可持续观察第 459 点；索引范围为 0–459，按设备上报顺序排列，按下表定位区域。

`point_row`、`point_column` 分别为 `point_index / 20` 向下取整和 `point_index % 20`。行号 0–22、列号 0–19；映射表中第 0 行在下方，第 0 列在左侧。这是索引网格，不是毫米坐标。按实际手套侧别查表：

| 区域 | 行 | 右手列 | 左手列 |
| --- | --- | --- | --- |
| 小指 | 17–22 | 0–3 | 16–19 |
| 无名指 | 17–22 | 4–7 | 12–15 |
| 中指 | 17–22 | 8–11 | 8–11 |
| 食指 | 17–22 | 12–15 | 4–7 |
| 拇指 | 17–22 | 16–19 | 0–3 |
| 掌心 | 0–14 | 0–15 | 4–19 |

五指各 24 点，掌心 240 点。第 15–16 行及掌心行之外的 4 列共 100 点在映射表中未分区；示例仍保留并汇总这些值，不把它们当作无效数据或零值。例如索引 459 为第 22 行第 19 列，对应右手拇指或左手小指。

kPa 是依据手掌汇总气压测量得到的估计，未逐点标定，不是力（N），也没有已验证的逐点精度。原始值 107–255 的估计均截顶到约 40.01 kPa，`capped_points` 是落在这个截顶区间的测点数；它不表示传感器已物理饱和或实际压力等于 40.01 kPa。示例保留原始值，kPa 仅显示一位小数。

启动后先显示等待提示。启动完成后约 5 秒仍没有首帧压感时，程序报错退出；检查 HMP-MX 型号、串口和设备是否上报压感。460 点全零仍是有效帧。收到首帧后持续运行，缺少压感块时不生成新帧；1 Hz 显示可能跳过短促按压，观察时保持按压片刻。按 `Ctrl+C` 停止。姿态、IMU 和 Skeleton 使用对应的通用示例。

### 使用完整标定密钥

安装包含 `TargetAlgorithm.calibration_key` 的匹配 Core、Glove SDK 和 CLI；首次发布版本尚未确定。C++ 项目使用整套新 SDK 重新构建，原有调用代码可继续使用。先通过 CLI 完成采集和标定，使用 `calibration run --key-output` 保存一个 `.qnbotkey` 文件。将整个文件复制到数据批次中，不需要查找或复制其他标定文件。

使用导出结果中的相同算法包及版本、操作者、手套和手侧。CLI 采集使用设备名 `primary`，通过 `--source-name primary` 保持一致。消费其他 SDK 程序导出的文件时，使用原程序的设备名；设备未命名时省略 `--source-name`。将下面的 `TARGET_NAME` 替换为标定结果 `target.name`；该值为空时省略 `--target-name`。

```bash
./build/algorithm_configuration --port /dev/ttyUSB0 --side left \
  --package-id '<package-id>' --operator-id default \
  --source-name primary --target-name TARGET_NAME --calibration-key /path/to/batch.qnbotkey \
  --key-output /path/to/new-batch/effective.qnbotkey
```

程序读取所选目标的输出，并在运行结束前将实际使用的完整密钥保存到 `--key-output`。预期看到 `retargeting` 数据和 `effective calibration key saved`，后者显示文件路径和 `sha256`；该值应与 CLI 导出结果的 `calibration_key.digest` 相同。将该文件与本次原始数据一起保存。输出目录须已存在，输出文件须不存在。没有成功输出、密钥不匹配或文件内容损坏时，程序报告错误。省略 `--key-output` 只读取数据；省略 `--calibration-key` 保持原来的标定流程。密钥已包含所需骨长和参数配置，使用密钥时无需再次准备骨长或指定参数覆盖文件。

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

程序结束时输出 `samples`（接收数）、`observed_hz`（首末样本之间的平均交付频率）和 `last_sequence`，随后输出 `subscription complete`。回调只记录计数和时间；最新值模式下序号可能跳跃。至少收到两条样本才计算频率，否则显示 `0`。实际频率还取决于输入速度、处理耗时和系统调度；单帧间隔会随调度抖动变化，不能把配置理解为每帧严格等间隔。

## 骨长与算法参数配置

`algorithm_configuration` 将已有骨长配置的使用策略与算法参数覆盖文件放在同一示例中。
先通过 QnBot CLI 安装产品交付的兼容算法包；准备骨长配置及覆盖文件时遵循该算法包的产品说明。
该示例默认使用已有配置，也可显式选择扫码准备或导入网页下载的单手配置；准备成功后再创建 SDK。新增准备选项需要包含 `BoneConfigSession` 接口的配套 SDK 安装包。

```bash
./build/algorithm_configuration --port /dev/ttyUSB0 --side left --package-id '<package-id>' --use-bone-config false
./build/algorithm_configuration --port /dev/ttyUSB0 --side left --package-id '<package-id>' --operator-id default --use-bone-config true --algorithm-config-path config/override.yaml
```

- `--use-bone-config auto`：默认值，存在对应操作者、手侧的骨长配置时使用，否则采用算法默认值。
- `--use-bone-config true`：要求对应配置存在且有效；`false` 忽略该配置。
- `--operator-id`：骨长配置对应的操作者，默认 `default`；无需指定配置安装目录。
- `--algorithm-config-path`：可选的算法参数覆盖文件，相对路径从程序启动目录解析；字段按算法包说明填写。省略时使用算法默认参数。
- `--seconds`：启动及所需自动标定完成后的读取秒数，默认 `10`。


需要先测量时，按 [标定网页](https://calibrator.qnbot.com) 的要求准备 A4 标定板，在原命令中增加 `--prepare-bone-config`：

```sh
./build/algorithm_configuration --port /dev/ttyUSB0 --side left --package-id '<package-id>' --operator-id default --use-bone-config true --prepare-bone-config
```

程序显示二维码或链接；手机完成指定手侧测量后，电脑输出安装路径并继续运行。会话有效期为 15 分钟，到期后重新运行命令。也可以在网页下载“上位机配置 YAML”并导入：

```sh
./build/algorithm_configuration --port /dev/ttyUSB0 --side left --package-id '<package-id>' --operator-id default --use-bone-config true --bone-config-file /path/to/left.yaml
```

`--prepare-bone-config` 与 `--bone-config-file` 二选一；“详细 YAML”不能作为配置导入。已有配置默认保留，确认要替换时增加 `--replace-bone-config`。准备失败时程序不会继续创建 SDK。取消时按终端提示操作；若已经显示配置安装成功，清理未确认不影响已保存的配置。

缺少动作采集时按终端提示操作，完成后输出目标关节数据。骨长配置不等同于该动作采集与标定流程。
配置文件修改后重新运行示例；是否需要重新标定以算法包说明为准。要求使用配置却找不到文件时，先确认操作者及手侧与准备的配置一致。

配置示例正常结束时分别输出 `subscription complete` 和 `algorithm configuration complete`。
完整配置说明见[权威 SDK 指南](https://docs.qnbot.com/zh-CN/docs/vetra-glove-pro/sdk/cpp/latest/configuration-reference)。

`time_sync` 同时输出 `source_time_ms` 和 `observed_at_ms`：
前者反映数据源时间，后者用于观察 SDK 接收的先后，不能直接把两个不同时间基准相减。

## 常见问题

- CMake 找不到 `qnbot_glove`：确认 `CMAKE_PREFIX_PATH` 指向解压后的匹配 SDK 包。
- 找不到手套：确认设备已连接、串口未被其他程序占用，并运行 `discover_gloves`。
- 找不到算法包：通过 CLI 安装正确的产品算法包，并检查 `--package-id`。
- 没有目标手输出：确认已完成终端提示的标定；外部输入示例在标定期间也要持续推帧。
- 后台自动引导启动报终端错误：在可交互终端运行，或选择 `--capture-interaction external`。
- 采集被取消或采集/标定失败：示例会报告错误并清理资源，不会继续等待目标输出；保留完整错误信息后排查。
- 没有 IMU 输出：确认 USB 直连手套支持 IMU 输出，并在 `start()` 后等待第一条有效帧。

仍无法解决时，请保留构建命令、运行命令和完整错误信息并联系 QnBot 对接人员。

## 有线和无线共用示例

以下示例使用同一套配置，支持 USB 直连、Receiver 无线连接以及左右手混合连接。
连接 Receiver 时，打开本次使用的手套。只读取一只手时，无需另一只手在线。

查看可用设备及连接类型：

```bash
./build/discover_gloves
```

读取指定手套的设备信息（无需算法包或启动数据流）：

```bash
./build/device_info --side left
./build/device_info --side right
./build/device_info --port <port> --side left
```

只连接一只手套时可省略参数；有多个同侧手套时用 `--port` 明确串口。
输出 SN、手别、型号、固件及 `transport_type`。有线为 `serial`，无线手套为 `receiver_peer`；
设备未提供的字段显示 `unknown`。

`device_lifecycle` 无需算法包，依次展示双手输出、停止右手时左手继续输出、重启右手后恢复输出。省略串口参数时自动发现；两只手套使用同一 Receiver 时，`--left-port` 和 `--right-port` 可以填写同一串口。

## 多路 IMU 与原始 EMF

使用配套新版 Core、Glove 包重新构建或安装示例；目标正式版本尚未确定。IMU 示例直接逐项输出通道 id、位置、有效性、fresh 和数值，无需额外参数。普通手套输出 raw_counts；Helix 输出 gyro_mdps 和 accel_mg，缺失值保持缺失。 同一 IMU 示例同时订阅 pose，输出独立 hand/wrist 的 fresh/held、timestamp_ms 和 received_ms；缺失组件显示 absent，无效腕部显示 valid=false 和空向量。腕部 position_m 为米，quaternion_xyzw 按 x/y/z/w 排列。held 组件保留自己的时间，不能当作与本次新组件同时采样。

读取支持设备的原始 EMF，不需要目标手算法包：

```bash
./build/emf --port /dev/your-glove --side right
```

程序输出 session、实际存在的帧序号或 sync、channel_id 及四组 3×3 整数矩阵，单位为 demodulated_integral。SDK 自动准备配套采集，客户无需设置采样频率。没有数据时等待首条观测；设备不支持时返回能力错误。按 Ctrl+C 停止。
