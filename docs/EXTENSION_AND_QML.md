# 扩展接口与 QML 现代化路线

## 当前扩展点

| 扩展点 | 位置 | 用途 |
| --- | --- | --- |
| 正式通信控制器 | `gucds::DeviceCommunicationController` | 串行调度 AT、DTU、Modbus 扫描、寄存器读写和测量任务。 |
| 串口后端 | `gucds::SerialSession` | Windows Win32 COM API 与 POSIX `termios` 的枚举、打开、收发和超时。 |
| 测试后端 | `gucds::VirtualTransport`、`VirtualDevice`、`VirtualModbusClient` | 仅用于 QtTest 自动验证，不作为正式界面默认数据源。 |
| 协议构造/解析 | `gucds::AtProtocol`、`VirtualModbusClient` 的静态帧方法 | 统一命令字符串、CRC、寄存器帧与传感器数据解析。 |
| 数据模型 | `DeviceTableModel`、`MeasurementTableModel`、`CalibrationTableModel`、`BusDeviceTableModel` | 均为 `QAbstractTableModel`，Widgets 和 QML 都可以复用。 |

## 后续增加功能的建议规则

1. 新功能先加到 `src/core` 的模型、协议或后端接口，不直接写死在界面里。
2. 真实硬件不可用时，正式界面返回明确错误；测试需要数据时使用测试后端。
3. 每个新增协议命令都补 QtTest，覆盖命令生成、成功回包、错误回包和超时。
4. 每个新增页面都复用已有模型或新增独立模型，避免把业务状态塞进控件文本。

## QML/Qt Quick 现代化可行性

可以做。当前核心模型是 `QAbstractTableModel`，通信边界是 `DeviceCommunicationController`，两者都可以被 QML 前端复用。

推荐路线：

1. 保留当前 Qt Widgets 版本作为 LabVIEW 一比一迁移基准。
2. 新增独立 Qt Quick 前端目标或页面模块，例如 `src/quick`，用 `qt_add_qml_module()` 管理 QML。
3. 将 `DeviceTableModel`、`MeasurementTableModel` 等注册给 QML，上层用 `TableView`、`ListView`、弹窗、状态芯片做现代化界面。
4. 现代化界面不要直接重写协议逻辑，只调用 `src/core` 中的模型和后端接口。
5. 在真实设备联调稳定后，再做视觉升级；否则容易把“业务未确认”和“界面美化”混在一起。
