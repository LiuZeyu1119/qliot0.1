# GUCDS Qt 重构工程

这是 `General Upper Computer Debugging Software5.5` 的 Qt 6 重构工程。目标是把 LabVIEW 上位机逐项迁移到 Qt Widgets/C++，并以 `docs/labview-analysis` 中已经求证的文档、前面板截图、控件清单、控件值和协议字符串作为迁移基准。

## 当前状态

- 已建立 Qt 6.8 CMake 工程骨架。
- 已按 LabVIEW 主前面板截图重排主窗口：蓝色外框、设备配置表、设备库、串口侧栏、右侧 Tab、测试数据表、最新值列表和曲线图。
- 已迁移主窗口标题、主 Tab、核心按钮、常用枚举、默认服务器、AT/DTU 命令模板、设备/测量/标定/总线设备表格模型。
- 已实现独立产品管理窗口：权限校验、搜索和类别筛选、新建/复制/修改/删除、动态参数定义、未保存修改保护，并将结果持久化到 SQLite 后刷新主界面设备库。
- 正式界面使用 `DeviceCommunicationController` 接入真实串口，不注入假设备、假测量值或假回包；AT、DTU、Modbus 扫描、参数读写和测量均异步执行。
- DTU 配置弹窗按协议分别提供 MQTT、HTTP、TCP、UDP 和 WebSocket 表单，只显示当前协议相关参数；字段通过校验后依次写入网络通道并执行保存命令。
- 已支持简体中文和英文，可在“设置 (Settings) -> 语言 (Language)”中选择；确认后程序保存语言并退出，下次启动应用新语言。协议键、数据库键和对象名不随界面语言变化。
- 已对照 DTU、蓝牙倾角传感器和 LoRa-RS485 网关三份固件归档核对 Modbus/AT 协议，修正 MCU 波特率字节序和 LoRa 工作模式映射，详见 `docs/FIRMWARE_PROTOCOL_AUDIT.md`。
- 已增加 QtTest 测试入口，覆盖协议命令、状态码解析、产品/标定/总线 SQLite CRUD、产品管理 UI、主窗口操作 UI、串口失败路径、可选 com0com 链路和虚拟 Modbus。
- `SerialSession` 在 Windows 使用 Win32 COM API，在 Linux/macOS 使用 POSIX `termios`；支持定长 Modbus 帧和静默超时结束的不定长文本回包。
- 曲线图先用自绘 `QWidget` 替代 `QtCharts`；当前本机 Qt 组件未发现 `Qt6Charts`。
- 后续可以切到 QML/Qt Quick 做现代化界面，现有 `core` 模型和传输接口已与 Widgets 解耦，路线见 `docs/EXTENSION_AND_QML.md`。

说明：英文界面只翻译软件文案。SQLite 中从 LabVIEW 导入的产品名、类别、曲线名和其他业务数据保留原文，避免自动翻译改变用户数据或匹配键。

## 构建

推荐使用本机 Qt SDK：

```powershell
$env:PATH='C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;' + $env:PATH
& 'C:\Qt\6.8.3\mingw_64\bin\qt-cmake.bat' -S . -B .\build\win-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build .\build\win-debug --parallel 4
& 'C:\Qt\Tools\CMake_64\bin\ctest.exe' --test-dir .\build\win-debug --output-on-failure
```

macOS（Apple Silicon + Homebrew QtBase 6.11 或更高版本）：

```bash
cmake --preset macos-homebrew
cmake --build --preset macos-homebrew
```

当前 macOS SDK 已不再提供 `AGL.framework`，不要使用仍依赖它的 Homebrew Qt 6.9；本项目预设使用 `/opt/homebrew/opt/qtbase`。

## 迁移原则

1. 任何界面、协议、状态、默认值必须能追溯到 LabVIEW 文档或官方导出。
2. 不臆测硬件接口；串口、Modbus、DTU 行为先建抽象和测试，再接真实设备。
3. UI 以 LabVIEW 前面板截图和 461 项 Controls/Indicators 清单为准逐步还原。
4. 每个迁移模块必须有验收条目或测试。

## SQLite 数据

- `data/gucds.sqlite` 由 `gucds_import_labview_data.exe` 从 LabVIEW 原始 `table/`、`files/` 和 TDMS 文件生成。
- 主界面启动时会加载该库，设备配置表、设备库树、设备标定表、总线设备表均使用真实导入数据。
- 产品管理对 `device_records` 执行事务化增删改，并检查名称、类别、型号重复；重新运行全量 LabVIEW 导入工具仍会重建设备库，正式使用前应备份 SQLite。
- 详细表结构、来源映射和验证样本见 `docs/SQLITE_DATA_IMPORT.md`。

## 便携版打包

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\package_portable.ps1
```

输出目录为 `dist/GUCDSQt-portable-win-x64`，压缩包为 `dist/GUCDSQt-portable-win-x64.zip`。当前本机只安装了 `mingw_64` Qt kit，因此产物是 Windows x86_64 便携版；严格 32 位 Windows 需要另装 32 位 Qt kit 后重新构建。

## Windows 安装包

安装 Inno Setup 6 并生成双语安装程序：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\package_installer.ps1 -InstallCompiler
```

后续已经安装编译器时，可以省略 `-InstallCompiler`。输出文件为
`dist/QL-IOT-App-5.7.4-win-x64-Setup.exe`。安装器默认执行当前用户安装，无需管理员权限；创建开始菜单入口，可选桌面快捷方式，并保留升级前和卸载后的 `data/gucds.sqlite` 用户数据库。
