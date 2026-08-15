# 需求追踪矩阵

| LabVIEW 功能 | 证据来源 | Qt 模块 | 当前状态 | 验收方式 |
| --- | --- | --- | --- | --- |
| 主窗口标题与描述 | `02a-main-vi-official-controls.md` | `MainWindow` | 已迁移 | 窗口标题一致。 |
| 主 Tab：测试数据、设备配置、设备标定、总线设备/网关 | 官方控件清单 `主选项卡` | `MainWindow` | 已迁移 | Tab 文案与顺序一致。 |
| 跳转指令 21 项 | 官方控件清单 `跳转指令` | `AppConfig` | 已迁移 | 枚举列表一致。 |
| 波特率枚举 | 官方控件清单 `波特率_M` | `AppConfig` | 已迁移 | ComboBox 选项一致。 |
| AT 测量/频谱/配置命令 | `04-communication-protocols.md` | `AtProtocol` | 已迁移首版 | QtTest 覆盖命令生成。 |
| AT/设备参数错误码 | `04-communication-protocols.md` | `AtProtocol` | 已迁移首版 | QtTest 覆盖状态码解析。 |
| 设备表 | `02-ui-control-map.md`、前面板截图 | `DeviceTableModel` | 已迁移 | 默认空表，真实扫描成功后填充。 |
| 产品管理 | `modules/product-management.md`、`modules/product-management-auth.md` | `ProductManagementDialog` + `ProductTableModel` + `LabviewDatabase` | 已迁移 SQLite 版 | UI 测试覆盖加载、筛选、新建和保存；核心测试覆盖新增、修改、防重复和删除。 |
| 测量数据表 | `02-ui-control-map.md`、前面板截图 | `MeasurementTableModel` | 已迁移 | 默认空表，真实 `AT,set,meastart` 成功后追加。 |
| 数据曲线 | 前面板截图 | `SpectrumPlotWidget` | 已迁移 | 默认空曲线，真实频谱读取成功后刷新。 |
| 设备标定表 | 前面板截图、控件清单 | `CalibrationTableModel` + `LabviewDatabase` | 已迁移 | 按选中行增改删并持久化 SQLite；QtTest 覆盖模型和数据库。 |
| 总线设备表 | 前面板截图、控件清单 | `BusDeviceTableModel` + `LabviewDatabase` | 已迁移 | 按选中行增改删、真实测试命令和应答码更新。 |
| VISA 串口 | 工程依赖和控件清单 | `SerialSession` + `DeviceCommunicationController` | 已接入 | Win32/POSIX 串口支持定长/不定长收发、超时、取消和异步任务；com0com 测试由环境变量启用。 |
| Modbus RTU | 固件源码和协议文档 | `DeviceCommunicationController` + `VirtualModbusClient` 帧方法 | 已接入 | 实现 1..247 扫描、B1..B7、03/04/06/10、CRC、MCU/LoRa 参数映射和测量解析。 |
| TDMS 设备库/标定曲线/总线设备管理器 | `05-data-files.md`、LabVIEW `files/*.tdms` | `LabviewDatabase` + SQLite | 已迁移 | `labviewDatabaseImportsExistingData` 重新导入并校验行数和真实样本。 |
| DTU 网络配置 | 协议文档 `config,set,%s,%s` | `AtProtocol` + `DeviceCommunicationController` | 已接入 | 表单生成协议键值，依次发送配置和 `config,set,save`，错误中止后续命令。 |

## 八荣八耻落地检查

| 原则 | 项目动作 |
| --- | --- |
| 以查档求证为荣 | UI 和协议以 LabVIEW 官方导出与分析文档为证据。 |
| 以对齐需求为荣 | 本矩阵记录每项功能的来源、状态、验收方式。 |
| 以请示规则为荣 | 串口、Modbus 未确认前不臆造完整实现；TDMS 仅按已解析出的现有文件格式导入。 |
| 以复用存量为荣 | 复用 `docs/labview-analysis`、`table`、`files` 作为迁移基准。 |
| 以完备测例为荣 | QtTest 覆盖协议、文件读取、虚拟设备、标定/总线 CRUD、串口失败路径、主窗口操作和虚拟 Modbus。 |
| 以恪守规范为荣 | 使用 Qt 6 CMake API，不写 qmake/Qt5 宏。 |
| 以坦诚存疑为荣 | 硬件差异继续通过真实设备联调记录，QtCharts 仍使用自绘替代。 |
| 以分步迭代为荣 | 首版只建可编译骨架和可验证核心，后续逐模块迁移。 |

## 2026-07-02 迁移闭环补充

| 功能片段 | 当前 Qt 实现 | 自动验证 |
| --- | --- | --- |
| 测试专用设备搜索 | `VirtualDevice::scanDevices()` 返回测试设备，仅用于 QtTest，不作为主界面默认数据。 | `virtualDeviceScansDevices` |
| 测试专用启动测量 | `AT,set,meastart` 通过 `VirtualTransport` 生成测试测量记录，仅用于 QtTest。 | `virtualDeviceMeasures` |
| 测试专用读取频谱 | `AT,get,spec` 返回测试频谱，仅用于 QtTest。 | `virtualDeviceReadsSpectrum` |
| 测试专用 DTU 配置 | `config,set,%s,%s` 写入测试配置，仅用于 QtTest。 | `virtualDeviceStoresDtuConfig` |
| 正式真实通信 | `DeviceCommunicationController` 只连接用户选择的串口，不生成模拟数据。 | `communicationControllerReportsOpenFailure`、可选 com0com 测试 |
| 未知命令 | 未识别命令返回 `AT_PARSINGFAILED`。 | `virtualTransportRejectsUnknownCommand` |
| 标定模型 | `添加/删除/清空` 改变标定表。 | `calibrationTableModelMutates` |
| 总线设备模型 | 更新最后一行应答码。 | `busDeviceTableModelUpdatesResponse` |
| 串口错误路径 | `SerialSession` 对空端口、无效端口、超时和收发错误返回具体原因。 | `serialSessionTransitions`、`communicationControllerReportsOpenFailure` |
| 测试专用虚拟 Modbus | 保持寄存器读写和帧记录，仅用于自动测试。 | `virtualModbusReadsWrites` |

真实串口和 Modbus 软件链路已实现；仍需用目标批次硬件核对设备特有回包、时序和 DTU 固件键值。TDMS 已按 LabVIEW 现有字符串数组格式导入 SQLite，若出现其它 TDMS 数据类型再按样本扩展。
