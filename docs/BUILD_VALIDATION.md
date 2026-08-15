# 构建与验证记录

## 本机 Qt 环境

| 项目 | 路径 |
| --- | --- |
| Qt SDK | `C:\Qt\6.8.3\mingw_64` |
| qt-cmake | `C:\Qt\6.8.3\mingw_64\bin\qt-cmake.bat` |
| CMake | `C:\Qt\Tools\CMake_64\bin\cmake.exe` |
| Ninja | `C:\Qt\Tools\Ninja\ninja.exe` |
| MinGW g++ | `C:\Qt\Tools\mingw1310_64\bin\g++.exe` |

## 已执行验证

| 验证项 | 结果 |
| --- | --- |
| CMake 配置 | 通过，当前 Windows 构建目录 `build/win-debug`。 |
| Ninja 编译 | 通过，生成 `gucds_app.exe` 和 `gucds_core_tests.exe`。 |
| QtTest | 通过，核心、产品管理 UI、主窗口 UI 共 `3/3 tests passed`。 |
| 应用启动烟测 | 通过，启动 3 秒未崩溃，随后主动关闭进程。 |
| 主界面截图 | 通过，中英文各 4 个页签和产品管理页共 10 张，保存于 `build/win-debug/ui-screenshots`。 |

## 当前构建命令

```powershell
$env:PATH='C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;' + $env:PATH
& 'C:\Qt\6.8.3\mingw_64\bin\qt-cmake.bat' -S . -B .\build\win-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build .\build\win-debug --parallel 4
& 'C:\Qt\Tools\CMake_64\bin\ctest.exe' --test-dir .\build\win-debug --output-on-failure
```

## 2026-07-13 固件协议、双语和 UI 循环验证

| 验证项 | 结果 |
| --- | --- |
| 固件归档审计 | 通过，已解包并只读核对 DTU、蓝牙倾角传感器和 LoRa-RS485 网关三份归档。 |
| 协议测试 | 通过，覆盖 Modbus 字节偏移、CRC、MCU 波特率特殊字节序、LoRa 模式映射、AT/蓝牙命令和 B7/B8/B9 状态。 |
| 双语 | 通过，简体中文/英文切换、设置持久化和稳定内部键均有 UI 测试。 |
| 无界面 QtTest | 通过，`3/3 tests passed`，耗时约 1.16 s。 |
| Windows 原生字体 UI 测试 | 通过，主窗口 `6 passed`（含英文→中文→英文往返切换），产品管理 `3 passed`；按钮和参数表头防截断断言通过。 |
| 截图复核 | 通过，修正英文 MCU/总线/网络按钮、SSL 选项、串口侧栏和产品表头拥挤；中文不再显示 `_MCU`/`_LR` 内部后缀。 |

## 2026-07-10 真实通信闭环验证

| 验证项 | 结果 |
| --- | --- |
| 全量构建 | 通过，正式应用只链接真实串口通信控制器。 |
| QtTest | 通过，核心、产品管理 UI、主窗口 UI 共 `3/3`。 |
| com0com 定长帧 | 通过，`COM4 <-> COM5` 完成 Modbus 请求/回显。 |
| com0com 文本帧 | 通过，AT 命令以静默超时正确结束不定长回包。 |
| F405 轮询测量 | 通过，B1、状态轮询、04 数据帧均完成。 |
| F405 主动上报 | 通过，B1 回显后优先识别 `01 04 10` 的 21 字节主动帧。 |
| 应用启动烟测 | 通过，启动 3 秒无异常退出。 |

## 2026-07-02 自动循环记录

执行命令：

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\run_migration_cycle.ps1
```

结果：

| 验证项 | 结果 |
| --- | --- |
| Qt CMake 配置 | 通过，当时构建目录已生成。 |
| Ninja 构建 | 通过，`gucds_app.exe` 和 `gucds_core_tests.exe` 成功链接。 |
| QtTest | 通过，`1/1 tests passed`。 |
| 应用启动烟测 | 通过，隐藏窗口启动 3 秒后由脚本关闭。 |
| UI 截图刷新 | 通过，使用 Qt/Mingw 运行时 `PATH` 启动后按窗口句柄截图。 |

## 2026-07-03 SQLite 数据迁移验证

执行命令：
```powershell
powershell -ExecutionPolicy Bypass -File .\tools\run_migration_cycle.ps1
```

结果：
| 验证项 | 结果 |
| --- | --- |
| LabVIEW -> SQLite 导入 | 通过，生成 `data/gucds.sqlite`。 |
| 可见数据统计 | 通过，`devices=147`、`calibrations=67`、`busDevices=10`。 |
| QtTest | 通过，`labviewDatabaseImportsExistingData` 校验真实样本和原始单元格表。 |
| 应用启动烟测 | 通过，设置 `QLIOT_SQLITE_PATH` 后启动 3 秒未退出。 |
| 主界面截图 | 通过，设备配置表和设备库树可见旧传感器数据；临时截图已在验证后清理。 |

## 2026-07-03 便携版打包验证

执行命令：
```powershell
powershell -ExecutionPolicy Bypass -File .\tools\package_portable.ps1
```

结果：
| 验证项 | 结果 |
| --- | --- |
| 便携目录 | 通过，生成 `dist/GUCDSQt-portable-win-x64`。 |
| 便携 zip | 通过，生成 `dist/GUCDSQt-portable-win-x64.zip`，大小约 24 MB。 |
| Qt 运行库部署 | 通过，包含 `Qt6Core.dll`、`Qt6Gui.dll`、`Qt6Widgets.dll`、`Qt6Sql.dll`、`platforms/qwindows.dll`、`sqldrivers/qsqlite.dll` 和 MinGW runtime。 |
| SQLite 数据 | 通过，包内包含 `data/gucds.sqlite`，导入统计 `devices=147`、`calibrations=67`、`busDevices=10`。 |
| 无 Qt PATH 启动烟测 | 通过，脚本临时仅保留系统 PATH 后启动 `gucds_app.exe` 3 秒未退出。 |
| 包内导入工具烟测 | 通过，包内 `gucds_import_labview_data.exe` 在无 Qt PATH 环境下可生成 SQLite。 |

说明：当前 Qt kit 为 `C:\Qt\6.8.3\mingw_64`，所以便携版是 Windows x86_64；若目标是严格 32 位 Windows，需要安装 32 位 Qt/MinGW kit 后另打 32 位包。

本轮 `gucds_core_tests` 覆盖：

- AT 命令生成和状态码解析。
- UTF-8 文本表读取。
- 测试专用虚拟设备搜索、测量、频谱、DTU 配置和未知命令。
- 标定表增删清空、总线设备应答码更新。
- 真实通信控制器打开失败、串口空参数、无效端口和关闭状态。
- 主窗口 MCU、LoRa、DTU、标定、总线和 AT 管理器 20 个操作入口。
- 测试专用虚拟 Modbus 保持寄存器读写。

脚本防误报规则：

- `qt-cmake` 非 0 退出码会立即失败。
- `ninja` 非 0 退出码会立即失败。
- `ctest` 非 0 退出码会立即失败。
- `gucds_app.exe` 若 3 秒内退出，会记录退出码并失败。

## 仍需外部环境验证的内容

- 目标硬件串口：Win32/POSIX 后端已经实现，仍需用目标批次传感器验证设备处理时长和断线恢复。
- QtCharts：本机 Qt 组件目录未发现 `Qt6Charts`，当前使用自绘曲线控件。
- TDMS：当前导入器支持工程现有字符串数组格式；新增 TDMS 数据类型需要样本驱动扩展。
- 目标 Modbus 设备：软件已实现固件确认的地址、大小端、CRC 和超时，仍需实机确认不同固件版本的异常码。
