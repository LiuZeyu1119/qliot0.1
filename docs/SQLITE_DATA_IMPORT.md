# SQLite 数据迁移说明

## 目标

Qt 重构版不再使用界面占位数据。启动时从 `qt-rewrite/data/gucds.sqlite` 读取 LabVIEW 上位机已有数据；如果该库不存在，并且能找到 `General Upper Computer Debugging Software5.5`，程序会自动导入一次。

## 导入来源

| LabVIEW 文件 | SQLite 去向 | Qt 可见位置 |
| --- | --- | --- |
| `table/名称` | `device_records`、`labview_table_cells` | 左侧设备配置表、设备库树 |
| `table/性能`、`table/传参`、`table/MCU`、`table/LoRa`、`table/DTU` | `labview_table_cells` | 作为后续参数页真实数据源保留 |
| `files/设备类别` | `device_categories`、`labview_table_cells` | 后续设备库分类扩展 |
| `files/频振索力传感器参数` | `frequency_tension_parameters`、`labview_table_cells` | 工具 → 频振索力传感器扩展参数 |
| `files/设备库.tdms` | `device_records`、`tdms_cells` | 左侧设备配置表、设备库树 |
| `files/标定曲线.tdms` | `calibration_records`、`tdms_cells` | 设备标定 Tab |
| `files/总线设备管理器.tdms` | `bus_device_records`、`tdms_cells` | 总线设备/网关 Tab |

`labview_table_cells` 和 `tdms_cells` 保留原始单元格，避免只做界面所需字段而丢失后续功能要用的数据。

## 运行方式

自动循环脚本会在编译后导入 SQLite：

```powershell
powershell -ExecutionPolicy Bypass -File .\qt-rewrite\tools\run_migration_cycle.ps1
```

也可以单独运行导入工具：

```powershell
.\qt-rewrite-build\gucds_import_labview_data.exe `
  ".\General Upper Computer Debugging Software5.5" `
  ".\qt-rewrite\data\gucds.sqlite"
```

如需指定其它数据库位置，可设置：

```powershell
$env:QLIOT_SQLITE_PATH="D:\path\gucds.sqlite"
```

如需指定其它 LabVIEW 项目目录，可设置：

```powershell
$env:QLIOT_LABVIEW_ROOT="D:\path\General Upper Computer Debugging Software5.5"
```

## 已验证样本

QtTest 会用临时 SQLite 库重新导入并校验：

- `device_records` 至少 140 条，包含 `QL-SPS-WNDUG-1` / `定位(卫星)传感器`。
- `calibration_records` 为 67 条，包含 `磁通量D120` 的第 1 点测量值 `300.3`。
- `bus_device_records` 为 10 条，包含地址 `10` 的 `测斜传感器` / `QL-ATIS-RxDP-Mx`。
- 原始 `labview_table_cells` 和 `tdms_cells` 均有数据，保证不是只导入了界面展示字段。
