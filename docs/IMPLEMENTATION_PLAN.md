# Qt 重构实施计划

## 已对齐事实

| 项目 | 已求证内容 |
| --- | --- |
| Qt 版本 | 本机存在 `C:\Qt\6.8.3\mingw_64\bin\qt-cmake.bat`。 |
| 构建工具 | 本机存在 `C:\Qt\Tools\CMake_64\bin\cmake.exe`、`C:\Qt\Tools\Ninja\ninja.exe`、`C:\Qt\Tools\mingw1310_64\bin\g++.exe`。 |
| 原始 UI | `GUCDS_main.vi` 标题为 `奇力智造上位机调试系统QL-IOT App5.7`，窗口范围 `1296 x 769`。 |
| 控件规模 | LabVIEW 官方导出 461 项 Controls/Indicators，其中输入控件 179 项、输出指示器 282 项。 |
| 通信 | VISA 串口、Modbus RTU、TTL/AT、LoRa、DTU 网络。 |
| 数据 | `table` 文本表、`files` TDMS 数据库、标定曲线、设备类别和频振索力参数。 |

## 当前技术选择

| 领域 | Qt 方案 | 状态 |
| --- | --- | --- |
| 主界面 | Qt Widgets，固定基准尺寸 1296x769，后续按截图逐项定位。 | 已建首版 |
| 表格 | `QAbstractTableModel` + `QTableView`。 | 已建首版 |
| 曲线 | 自绘 `QWidget`，避免依赖未安装的 `QtCharts`。 | 已建首版 |
| 串口 | `SerialSession` 使用 Win32/POSIX 原生 API，避免额外 QtSerialPort 组件依赖。 | 已实现 |
| Modbus | 内部 RTU 帧实现，按 F405 固件确认的特殊字节偏移和小端 float 解析。 | 已实现 |
| TDMS | 最小导入器读取工程现有字符串数组并持久化 SQLite。 | 已实现现有格式 |
| 测试 | QtTest。 | 已建首版 |

## 迭代顺序

1. 建立 Qt 工程、迁移标题、菜单、Tab、核心按钮和可测试核心逻辑。
2. 对照 `02a-main-vi-official-controls.md` 建立完整控件映射表，逐项迁移到 Qt Widgets。
3. 迁移 `table` 文本表读取、设备库表、标定表和总线管理表。
4. 接入串口组件后迁移 VISA/TTL/AT/LoRa 通信。
5. 明确 Modbus 实现方案后迁移寄存器读写、测量、配置、校准流程。
6. 明确 TDMS 策略后迁移设备库与标定曲线持久化。
7. 做 UI 像素级比对：截图、控件文案、默认值、Tab 层级、菜单、按钮行为。
8. 做硬件联调：真实串口、外部 Modbus 模拟器、网关、DTU、传感器。

## 当前阻塞点

- 目标硬件仍需验证不同固件版本的串口时序、主动上报和异常码。
- 未安装或未发现 `Qt6Charts`，曲线图暂用自绘替代。
- LabVIEW 程序使用 NI Modbus Master 和 TDMS；Qt 端对应库/授权/文件格式策略需要确认。
