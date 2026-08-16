<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US" sourcelanguage="zh_CN">
<context><name>AppConfig</name>
<message><source>AT指令</source><translation>AT Command</translation></message>
<message><source>主从模式</source><translation>Master/Slave Mode</translation></message>
<message><source>主机</source><translation>Host</translation></message>
<message><source>从机</source><translation>Slave</translation></message>
<message><source>传感器测试</source><translation>Sensor Test</translation></message>
<message><source>低功耗</source><translation>Low Power</translation></message>
<message><source>写频振扩展参数</source><translation>Write Frequency/Tension Extended Parameters</translation></message>
<message><source>北京奇力建通工程技术有限公司</source><translation>Beijing Qili Jiantong Engineering Technology Co., Ltd.</translation></message>
<message><source>复位和校准</source><translation>Reset and Calibrate</translation></message>
<message><source>奇力智造上位机调试系统QL-IOT App5.7.4</source><translation>Qili Intelligent Manufacturing Host Debugging System QL-IOT App 5.7.4</translation></message>
<message><source>定点模式</source><translation>Point-to-Point Mode</translation></message>
<message><source>总线网关RT</source><translation>Bus Gateway RT</translation></message>
<message><source>总线设备/网关</source><translation>Bus Devices/Gateway</translation></message>
<message><source>恢复出厂设置</source><translation>Restore Factory Settings</translation></message>
<message><source>拉线式位移传感器上位机</source><translation>Draw-Wire Displacement Sensor Host Software</translation></message>
<message><source>指令</source><translation>Command</translation></message>
<message><source>标定测量</source><translation>Calibration Measurement</translation></message>
<message><source>测试数据</source><translation>Test Data</translation></message>
<message><source>磁通量开发读取</source><translation>Read Magnetic Flux Development Data</translation></message>
<message><source>自动</source><translation>Automatic</translation></message>
<message><source>自定义-TTL通讯</source><translation>Custom TTL Communication</translation></message>
<message><source>设备标定</source><translation>Device Calibration</translation></message>
<message><source>设备配置</source><translation>Device Configuration</translation></message>
<message><source>读闪存数据</source><translation>Read Flash Data</translation></message>
<message><source>读频谱</source><translation>Read Spectrum</translation></message>
<message><source>默认</source><translation>Default</translation></message>
</context>
<context><name>AtProtocol</name>
<message><source>AT 回包解析失败</source><translation>Failed to parse AT response</translation></message>
<message><source>AT 数据写入 Flash 溢出</source><translation>AT data Flash write overflow</translation></message>
<message><source>AT 数据数量越界</source><translation>AT data count is out of range</translation></message>
<message><source>AT 数据追加数量越界</source><translation>AT appended data count is out of range</translation></message>
<message><source>二级参数数量错误</source><translation>Invalid secondary parameter count</translation></message>
<message><source>传感器数量错误</source><translation>Invalid sensor count</translation></message>
<message><source>写设备参数数量超过 U8 范围</source><translation>Device parameter write count exceeds the U8 range</translation></message>
<message><source>写设备参数数量错误</source><translation>Invalid device parameter write count</translation></message>
<message><source>恢复出厂/参考设置失败</source><translation>Failed to restore factory/reference settings</translation></message>
<message><source>恢复出厂/参考设置成功</source><translation>Factory/reference settings restored successfully</translation></message>
<message><source>扩展参数 Flash 写入错误</source><translation>Failed to write extended parameters to Flash</translation></message>
<message><source>扩展参数值错误</source><translation>Invalid extended parameter value</translation></message>
<message><source>扩展参数数量错误</source><translation>Invalid extended parameter count</translation></message>
<message><source>扩展参数类型错误</source><translation>Invalid extended parameter type</translation></message>
<message><source>扩展参数配置成功</source><translation>Extended parameters configured successfully</translation></message>
<message><source>未知回包状态</source><translation>Unknown response status</translation></message>
<message><source>校准/复位失败</source><translation>Calibration/reset failed</translation></message>
<message><source>校准/复位成功</source><translation>Calibration/reset completed successfully</translation></message>
<message><source>设备参数 Flash 溢出</source><translation>Device parameter Flash overflow</translation></message>
<message><source>设备参数数量错误</source><translation>Invalid device parameter count</translation></message>
<message><source>设备参数格式错误</source><translation>Invalid device parameter format</translation></message>
<message><source>设备参数追加溢出</source><translation>Device parameter append overflow</translation></message>
<message><source>设备参数配置成功</source><translation>Device parameters configured successfully</translation></message>
<message><source>连续采样启动失败</source><translation>Failed to start continuous sampling</translation></message>
<message><source>连续采样启动成功</source><translation>Continuous sampling started successfully</translation></message>
<message><source>连续采样正在进行</source><translation>Continuous sampling is in progress</translation></message>
</context>
<context><name>BusDeviceTableModel</name>
<message><source>传感器名</source><translation>Sensor Name</translation></message>
<message><source>信道</source><translation>Channel</translation></message>
<message><source>地址</source><translation>Address</translation></message>
<message><source>序号</source><translation>Index</translation></message>
<message><source>应答码</source><translation>Response Code</translation></message>
<message><source>数据数</source><translation>Data Count</translation></message>
<message><source>组号</source><translation>Group ID</translation></message>
<message><source>规格型号</source><translation>Model</translation></message>
</context>
<context><name>CalibrationTableModel</name>
<message><source>日期时间</source><translation>Date/Time</translation></message>
<message><source>曲线名</source><translation>Curve Name</translation></message>
<message><source>标定值</source><translation>Reference Value</translation></message>
<message><source>测量值</source><translation>Measured Value</translation></message>
<message><source>温度(℃)</source><translation>Temperature (°C)</translation></message>
<message><source>点号</source><translation>Point No.</translation></message>
</context>
<context><name>DeviceCommunicationController</name>
<message><source>%1 条配置命令执行成功</source><translation>%1 configuration command(s) completed successfully</translation></message>
<message><source>AT 测量完成</source><translation>AT measurement completed</translation></message>
<message><source>Modbus 从站 %1</source><translation>Modbus Slave %1</translation></message>
<message><source>Modbus 写命令回显与请求不一致</source><translation>Modbus write response does not match the request</translation></message>
<message><source>Modbus 响应缓冲区不能为空</source><translation>Modbus response buffer cannot be null</translation></message>
<message><source>Modbus 回包 CRC 校验失败：%1</source><translation>Modbus response CRC check failed: %1</translation></message>
<message><source>Modbus 回包从站不匹配：期望 %1，收到 %2</source><translation>Modbus response slave mismatch: expected %1, received %2</translation></message>
<message><source>Modbus 回包功能码不匹配：期望 0x%1，收到 0x%2</source><translation>Modbus response function mismatch: expected 0x%1, received 0x%2</translation></message>
<message><source>Modbus 回包长度不足：%1 字节</source><translation>Modbus response is too short: %1 byte(s)</translation></message>
<message><source>Modbus 异常响应：功能码 0x%1，异常码 0x%2</source><translation>Modbus exception response: function 0x%1, exception 0x%2</translation></message>
<message><source>Modbus 扫描 1..247（当前从站 %1）</source><translation>Modbus scan 1..247 (selected slave: %1)</translation></message>
<message><source>Modbus 数据长度不匹配：期望 %1，收到 %2</source><translation>Modbus data length mismatch: expected %1, received %2</translation></message>
<message><source>串口设备</source><translation>Serial Device</translation></message>
<message><source>参数写入并校验成功</source><translation>Parameters written and verified successfully</translation></message>
<message><source>参数已写入 RAM，但保存命令失败：%1</source><translation>Parameters were written to RAM, but the save command failed: %1</translation></message>
<message><source>发现 1 个 AT 设备</source><translation>Found 1 AT device</translation></message>
<message><source>命令 %1 执行失败：%2</source><translation>Command %1 failed: %2</translation></message>
<message><source>在线 AT 设备</source><translation>Online AT Device</translation></message>
<message><source>在线设备</source><translation>Online Device</translation></message>
<message><source>收到 %1 字节回包</source><translation>Received a %1-byte response</translation></message>
<message><source>收到主动测量帧</source><translation>Received an unsolicited measurement frame</translation></message>
<message><source>测量失败：0x%1 %2</source><translation>Measurement failed: 0x%1 %2</translation></message>
<message><source>测量已取消</source><translation>Measurement canceled</translation></message>
<message><source>测量已启动，但读取数据失败：%1</source><translation>Measurement started, but reading data failed: %1</translation></message>
<message><source>测量超时：状态仍为 0x%1 %2</source><translation>Measurement timed out: status remains 0x%1 %2</translation></message>
<message><source>监听串口 %1 ms</source><translation>Listen to serial port for %1 ms</translation></message>
<message><source>监听收到 %1 字节</source><translation>Listener received %1 byte(s)</translation></message>
<message><source>等待 Modbus 功能码 0x%1 回包超时；串口数据：%2</source><translation>Timed out waiting for Modbus function 0x%1; serial data: %2</translation></message>
<message><source>精确测量完成</source><translation>Precision measurement completed</translation></message>
<message><source>设备命令 0x%1 执行成功</source><translation>Device command 0x%1 completed successfully</translation></message>
<message><source>设备搜索完成：发现 %1 个 Modbus 从站</source><translation>Device scan completed: found %1 Modbus slave(s)</translation></message>
<message><source>设备搜索已取消，已发现 %1 个设备</source><translation>Device scan canceled after finding %1 device(s)</translation></message>
<message><source>读取到 %1 字节参数</source><translation>Read %1 byte(s) of parameters</translation></message>
<message><source>通信操作已取消</source><translation>Communication operation canceled</translation></message>
</context>
<context><name>DeviceTableModel</name>
<message><source>数据1</source><translation>Data 1</translation></message><message><source>数据2</source><translation>Data 2</translation></message><message><source>数据3</source><translation>Data 3</translation></message><message><source>数据4</source><translation>Data 4</translation></message><message><source>数据5</source><translation>Data 5</translation></message>
<message><source>未定义</source><translation>Undefined</translation></message><message><source>规格型号</source><translation>Model</translation></message><message><source>设备名称</source><translation>Device Name</translation></message><message><source>设备类别</source><translation>Device Category</translation></message>
</context>
<context><name>MainWindow</name>
<message><source>%1 测量完成：Pitch=%2 Roll=%3 Error=%4 Temp=%5</source><translation>%1 measurement completed: Pitch=%2 Roll=%3 Error=%4 Temp=%5</translation></message>
<message><source>%1：%2 / %3</source><translation>%1: %2 / %3</translation></message>
<message><source>AT 管理器</source><translation>AT Manager</translation></message><message><source>AT 管理器已清除</source><translation>AT Manager cleared</translation></message>
<message><source>AT回包</source><translation>AT Response</translation></message><message><source>AT指令</source><translation>AT Command</translation></message><message><source>AT次数</source><translation>AT Repetitions</translation></message><message><source>AT管理器</source><translation>AT Manager</translation></message><message><source>AT说明</source><translation>AT Description</translation></message>
<message><source>DTU 参数</source><translation>DTU Parameters</translation></message><message><source>DTU主卡</source><translation>DTU Main Module</translation></message><message><source>DTU测试</source><translation>Test DTU</translation></message><message><source>DTU网络</source><translation>DTU Network</translation></message>
<message><source>F405倾角传感器</source><translation>F405 Tilt Sensor</translation></message><message><source>Gap_MCU</source><translation>Interval_MCU</translation></message><message><source>IP地址</source><translation>IP Address</translation></message>
<message><source>LabVIEW 数据导入 SQLite 失败：%1</source><translation>Failed to import LabVIEW data into SQLite: %1</translation></message>
<message><source>LoRa设置</source><translation>LoRa Settings</translation></message><message><source>MCU卡</source><translation>MCU Module</translation></message><message><source>ModID_MCU</source><translation>Modbus ID_MCU</translation></message><message><source>Mode_MCU</source><translation>Mode_MCU</translation></message><message><source>RS485_MCU</source><translation>RS485_MCU</translation></message><message><source>SSL加密</source><translation>SSL Encryption</translation></message>
<message><source>上移</source><translation>Move Up</translation></message><message><source>下移</source><translation>Move Down</translation></message><message><source>不加密</source><translation>No Encryption</translation></message>
<message><source>串口 %1/%2</source><translation>Serial %1/%2</translation></message><message><source>串口列表已刷新</source><translation>Serial port list refreshed</translation></message><message><source>串口号</source><translation>Serial Port</translation></message><message><source>串口连接</source><translation>Serial Status</translation></message><message><source>串口配置</source><translation>Serial Configuration</translation></message>
<message><source>主/从_LR</source><translation>Host/Slave_LR</translation></message><message><source>产品库已更新，设备库候选列表已刷新</source><translation>Product catalog updated; device library selections refreshed</translation></message><message><source>产品管理</source><translation>Product Management</translation></message>
<message><source>从左侧设备库选择一个候选设备</source><translation>Select a device from the library on the left</translation></message><message><source>从站ID</source><translation>Slave ID</translation></message><message><source>从配置表删除“%1”？</source><translation>Remove “%1” from the configuration table?</translation></message>
<message><source>以空格分隔的十六进制字节显示接收数据</source><translation>Display received data as space-separated hexadecimal bytes</translation></message><message><source>传感器参数</source><translation>Sensor Parameters</translation></message>
<message><source>保存</source><translation>Save</translation></message><message><source>保存失败</source><translation>Save Failed</translation></message><message><source>信道</source><translation>Channel</translation></message><message><source>信道_LR</source><translation>Channel_LR</translation></message>
<message><source>修改</source><translation>Edit</translation></message><message><source>修改总线设备</source><translation>Edit Bus Device</translation></message><message><source>修改标定点</source><translation>Edit Calibration Point</translation></message><message><source>关</source><translation>Off</translation></message><message><source>关于我们</source><translation>About Us</translation></message>
<message><source>写DTU</source><translation>Write DTU</translation></message><message><source>写LoRa</source><translation>Write LoRa</translation></message><message><source>写MCU</source><translation>Write MCU</translation></message><message><source>切换语言</source><translation>Switch Language</translation></message>
<message><source>删除</source><translation>Delete</translation></message><message><source>删除失败</source><translation>Delete Failed</translation></message><message><source>删除总线设备</source><translation>Delete Bus Device</translation></message><message><source>删除标定点</source><translation>Delete Calibration Point</translation></message><message><source>删除设备</source><translation>Delete Device</translation></message><message><source>刷新</source><translation>Refresh</translation></message>
<message><source>剩余&#10;字长</source><translation>Remaining&#10;Bytes</translation></message><message><source>功率_LR</source><translation>Tx Power_LR</translation></message><message><source>十六进制</source><translation>Hexadecimal</translation></message><message><source>协议类型</source><translation>Protocol Type</translation></message><message><source>参数</source><translation>Parameter</translation></message>
<message><source>参数帧无效或已有通信任务正在运行</source><translation>Invalid parameter frame or another communication task is running</translation></message><message><source>发送</source><translation>Send</translation></message><message><source>发送数据</source><translation>Sent Data</translation></message><message><source>取消</source><translation>Cancel</translation></message><message><source>命令无效或已有通信任务正在运行</source><translation>Invalid command or another communication task is running</translation></message><message><source>地址</source><translation>Address</translation></message>
<message><source>复位和校准</source><translation>Reset and Calibrate</translation></message><message><source>密码</source><translation>Password</translation></message><message><source>工作模式_LR</source><translation>Operating Mode_LR</translation></message><message><source>工具</source><translation>Tools</translation></message>
<message><source>已更新配置表第 %1 行</source><translation>Updated row %1 of the configuration table</translation></message><message><source>已有通信任务正在运行，请稍后重试</source><translation>A communication task is already running. Try again later.</translation></message><message><source>已添加设备：%1</source><translation>Device added: %1</translation></message><message><source>已连接</source><translation>Connected</translation></message><message><source>帮助</source><translation>Help</translation></message><message><source>应用到配置表</source><translation>Apply to Configuration</translation></message><message><source>应答码</source><translation>Response Code</translation></message><message><source>开</source><translation>On</translation></message><message><source>开发者</source><translation>Developer</translation></message><message><source>开始</source><translation>Start</translation></message><message><source>开始测试</source><translation>Start Test</translation></message><message><source>当前设备参数</source><translation>Current Device Parameters</translation></message>
<message><source>总线设备</source><translation>Bus Device</translation></message><message><source>总线设备列表</source><translation>Bus Device List</translation></message><message><source>总线设备已保存</source><translation>Bus device saved</translation></message><message><source>总线设备已修改</source><translation>Bus device updated</translation></message><message><source>总线设备已删除</source><translation>Bus device deleted</translation></message><message><source>总线设备管理器</source><translation>Bus Device Manager</translation></message><message><source>恢复出厂设置</source><translation>Restore Factory Settings</translation></message><message><source>扩展参数不能为空。</source><translation>Extended parameters cannot be empty.</translation></message>
<message><source>接收&#10;字长</source><translation>Received&#10;Bytes</translation></message><message><source>接收数据</source><translation>Received Data</translation></message><message><source>搜索设备</source><translation>Scan Devices</translation></message><message><source>操作</source><translation>Operations</translation></message><message><source>数据%1</source><translation>Data %1</translation></message><message><source>数据手册</source><translation>Data Sheet</translation></message><message><source>数据手册：请查看项目 docs 目录</source><translation>Data sheet: see the project docs directory</translation></message><message><source>数据数</source><translation>Data Count</translation></message><message><source>文件</source><translation>File</translation></message><message><source>新建</source><translation>New</translation></message><message><source>新建：已创建空工程上下文</source><translation>New: empty project context created</translation></message>
<message><source>无法启动串口监听</source><translation>Unable to start serial listener</translation></message><message><source>无法启动精确测量任务</source><translation>Unable to start precision measurement</translation></message><message><source>无证书加密</source><translation>Encrypted Without Certificate</translation></message><message><source>曲线0</source><translation>Curve 0</translation></message><message><source>曲线名_标定</source><translation>Curve Name</translation></message><message><source>有证书加密</source><translation>Certificate-Based Encryption</translation></message>
<message><source>服务器 IP 不能为空。</source><translation>Server IP cannot be empty.</translation></message><message><source>服务器IP</source><translation>Server IP</translation></message><message><source>服务器端口</source><translation>Server Port</translation></message><message><source>未分类</source><translation>Uncategorized</translation></message><message><source>未命名设备</source><translation>Unnamed Device</translation></message><message><source>未定义</source><translation>Undefined</translation></message><message><source>未找到 LabVIEW 数据目录，无法生成 SQLite：General Upper Computer Debugging Software5.5</source><translation>LabVIEW data directory was not found; SQLite cannot be generated: General Upper Computer Debugging Software5.5</translation></message><message><source>未连接</source><translation>Disconnected</translation></message>
<message><source>本地地址_LR</source><translation>Local Address_LR</translation></message><message><source>本地组号_LR</source><translation>Local Group ID_LR</translation></message><message><source>标定</source><translation>Calibration</translation></message><message><source>标定值_标定</source><translation>Reference Value</translation></message><message><source>标定数据</source><translation>Calibration Data</translation></message><message><source>标定点已保存</source><translation>Calibration point saved</translation></message><message><source>标定点已修改</source><translation>Calibration point updated</translation></message><message><source>标定点已删除</source><translation>Calibration point deleted</translation></message><message><source>标定点数</source><translation>Calibration Points</translation></message>
<message><source>正在取消</source><translation>Canceling</translation></message><message><source>正在执行通信任务...</source><translation>Communication task in progress...</translation></message><message><source>此操作会清除设备通信和传感器参数，确定继续吗？</source><translation>This will clear the device communication and sensor parameters. Continue?</translation></message><message><source>波特率_LR</source><translation>Baud Rate_LR</translation></message><message><source>波特率_M</source><translation>Baud Rate_M</translation></message><message><source>波特率_MCU</source><translation>Baud Rate_MCU</translation></message><message><source>测量</source><translation>Measure</translation></message><message><source>测量值_标定</source><translation>Measured Value</translation></message><message><source>添加</source><translation>Add</translation></message><message><source>添加至设备</source><translation>Add to Configuration</translation></message><message><source>清空配置表</source><translation>Clear Configuration</translation></message><message><source>清除</source><translation>Clear</translation></message><message><source>温度_标定</source><translation>Temperature</translation></message><message><source>点号_标定</source><translation>Point No.</translation></message><message><source>用户名</source><translation>Username</translation></message>
<message><source>监听</source><translation>Listen</translation></message><message><source>监听串口数据三秒；收到一帧后立即显示</source><translation>Listen for serial data for 3 seconds and display the first received frame immediately</translation></message><message><source>目标地址_LR</source><translation>Target Address_LR</translation></message><message><source>目标组号_LR</source><translation>Target Group ID_LR</translation></message><message><source>确定删除曲线“%1”的第 %2 点吗？</source><translation>Delete point %2 from curve “%1”?</translation></message><message><source>确认清空当前设备配置表？</source><translation>Clear the current device configuration table?</translation></message>
<message><source>磁通量传感器开发</source><translation>Magnetic Flux Sensor Development</translation></message><message><source>磁通量传感器扩展参数</source><translation>Magnetic Flux Sensor Extended Parameters</translation></message><message><source>空速_LR</source><translation>Air Data Rate_LR</translation></message><message><source>端口</source><translation>Port</translation></message><message><source>简体中文</source><translation>Simplified Chinese</translation></message><message><source>组号</source><translation>Group ID</translation></message><message><source>网络选项卡</source><translation>Network Protocol</translation></message><message><source>网络配置</source><translation>Configure Network</translation></message><message><source>自定义LoRa</source><translation>Custom LoRa</translation></message><message><source>自定义TTL</source><translation>Custom TTL</translation></message><message><source>规格型号</source><translation>Model</translation></message>
<message><source>设备会立即重启并短暂断开串口，确定继续吗？</source><translation>The device will restart immediately and briefly disconnect from the serial port. Continue?</translation></message><message><source>设备参数</source><translation>Device Parameters</translation></message><message><source>设备名称</source><translation>Device Name</translation></message><message><source>设备名称不能为空。</source><translation>Device name cannot be empty.</translation></message><message><source>设备名称或规格型号不能为空。</source><translation>Device name or model cannot be empty.</translation></message><message><source>设备将重新计算零偏，测量期间请保持设备静止。确定继续吗？</source><translation>The device will recalculate its zero offset. Keep it stationary during measurement. Continue?</translation></message><message><source>设备已存在</source><translation>Device Already Exists</translation></message><message><source>设备库</source><translation>Device Library</translation></message><message><source>设备库候选</source><translation>Device Library Selection</translation></message><message><source>设备没有返回有效响应。</source><translation>The device did not return a valid response.</translation></message><message><source>设备类别</source><translation>Device Category</translation></message><message><source>设备配置表</source><translation>Device Configuration</translation></message>
<message><source>设置</source><translation>Settings</translation></message><message><source>设置日期时间</source><translation>Set Date/Time</translation></message><message><source>该设备没有可配置传感器参数</source><translation>This device has no configurable sensor parameters</translation></message><message><source>语言</source><translation>Language</translation></message><message><source>说明</source><translation>Notes</translation></message><message><source>说明：Qt 重构首版</source><translation>Notes: Initial Qt port</translation></message>
<message><source>请先在设备库或配置表选择一个设备</source><translation>Select a device from the library or configuration table first</translation></message><message><source>请先在设备库选择一个设备</source><translation>Select a device from the device library first</translation></message><message><source>请先在设备配置表选择一个设备</source><translation>Select a device from the configuration table first</translation></message><message><source>请先在设备配置表选择要修改的行</source><translation>Select the row to edit in the configuration table first</translation></message><message><source>请先填写 AT 指令。</source><translation>Enter an AT command first.</translation></message><message><source>请先选择要修改的标定点。</source><translation>Select the calibration point to edit first.</translation></message><message><source>请先选择要修改的设备。</source><translation>Select the device to edit first.</translation></message><message><source>请先选择要删除的标定点。</source><translation>Select the calibration point to delete first.</translation></message><message><source>请先选择要删除的设备。</source><translation>Select the device to delete first.</translation></message><message><source>请完整填写曲线名、点号、测量值、标定值和温度。</source><translation>Enter the curve name, point number, measured value, reference value, and temperature.</translation></message><message><source>请等待当前通信任务完成或先取消任务，再切换语言。</source><translation>Wait for the current communication task to finish, or cancel it before switching languages.</translation></message><message><source>请输入逗号分隔的扩展参数：</source><translation>Enter comma-separated extended parameters:</translation></message><message><source>请选择有效串口并填写正确波特率。</source><translation>Select a valid serial port and enter a valid baud rate.</translation></message>
<message><source>读LoRa</source><translation>Read LoRa</translation></message><message><source>读MCU</source><translation>Read MCU</translation></message><message><source>读取 SQLite 总线设备失败：%1</source><translation>Failed to read bus devices from SQLite: %1</translation></message><message><source>读取 SQLite 标定曲线失败：%1</source><translation>Failed to read calibration curves from SQLite: %1</translation></message><message><source>读取 SQLite 设备库失败：%1</source><translation>Failed to read the device library from SQLite: %1</translation></message><message><source>读取闪存传感器数据</source><translation>Read Sensor Flash Data</translation></message><message><source>读取频谱</source><translation>Read Spectrum</translation></message><message><source>读附加数据</source><translation>Read Additional Data</translation></message>
<message><source>连接中</source><translation>Connecting</translation></message><message><source>退出</source><translation>Exit</translation></message><message><source>通信任务正在进行，请等待完成或先取消</source><translation>A communication task is in progress. Wait for it to finish or cancel it first.</translation></message><message><source>通信协议</source><translation>Protocol</translation></message><message><source>通信失败</source><translation>Communication Failed</translation></message><message><source>通道ID</source><translation>Channel ID</translation></message><message><source>配置表中已有该设备。是否用右侧表单内容更新该行？</source><translation>This device is already in the configuration table. Update the row with the form values?</translation></message><message><source>配置表第 %1 行</source><translation>Configuration Table Row %1</translation></message><message><source>重启设备</source><translation>Restart Device</translation></message><message><source>重新扫描可用串口</source><translation>Rescan available serial ports</translation></message><message><source>频振索力传感器扩展参数</source><translation>Frequency/Tension Sensor Extended Parameters</translation></message>
</context>
<context><name>MeasurementTableModel</name>
<message><source>日期时间</source><translation>Date/Time</translation></message><message><source>测量值</source><translation>Measured Value</translation></message><message><source>温度</source><translation>Temperature</translation></message><message><source>电流(A)</source><translation>Current (A)</translation></message><message><source>索力(kN)</source><translation>Cable Force (kN)</translation></message>
</context>
<context><name>ProductManagementDialog</name>
<message><source> 副本</source><translation> Copy</translation></message><message><source> 点</source><translation> point(s)</translation></message><message><source>产品名称 *</source><translation>Product Name *</translation></message><message><source>产品管理</source><translation>Product Management</translation></message><message><source>产品管理权限</source><translation>Product Management Access</translation></message><message><source>产品类别 *</source><translation>Product Category *</translation></message><message><source>传感器参数定义</source><translation>Sensor Parameter Definitions</translation></message><message><source>保存产品</source><translation>Save Product</translation></message><message><source>全部类别</source><translation>All Categories</translation></message><message><source>关闭</source><translation>Close</translation></message><message><source>删除</source><translation>Delete</translation></message><message><source>删除产品</source><translation>Delete Product</translation></message>
<message><source>参数“%1”使用菜单输入，请至少填写一个菜单项。</source><translation>Parameter “%1” uses menu input. Enter at least one menu item.</translation></message><message><source>参数名称</source><translation>Parameter Name</translation></message><message><source>参数名称“%1”重复，请修改后再保存。</source><translation>Parameter name “%1” is duplicated. Change it before saving.</translation></message><message><source>取消</source><translation>Cancel</translation></message><message><source>可选扩展数据项</source><translation>Optional additional data item</translation></message><message><source>基本信息</source><translation>Basic Information</translation></message><message><source>复制</source><translation>Duplicate</translation></message><message><source>复制产品</source><translation>Duplicate Product</translation></message><message><source>多个菜单项用英文逗号分隔</source><translation>Separate menu items with commas</translation></message><message><source>字符</source><translation>Text</translation></message>
<message><source>密码不正确，请重试。</source><translation>Incorrect password. Try again.</translation></message><message><source>密码连续三次不正确，已取消进入产品管理。</source><translation>The password was incorrect three times. Product Management access was canceled.</translation></message><message><source>当前产品有未保存的修改。</source><translation>The current product has unsaved changes.</translation></message><message><source>搜索产品</source><translation>Search Products</translation></message><message><source>搜索名称、类别、型号或数据项</source><translation>Search by name, category, model, or data item</translation></message><message><source>撤销修改</source><translation>Revert Changes</translation></message><message><source>数据%1</source><translation>Data %1</translation></message><message><source>数据名称</source><translation>Data Names</translation></message><message><source>数据类型</source><translation>Data Type</translation></message><message><source>整数</source><translation>Integer</translation></message><message><source>新建</source><translation>New</translation></message><message><source>新建产品</source><translation>New Product</translation></message><message><source>显示 %1 / %2 项</source><translation>Showing %1 of %2</translation></message><message><source>未保存的产品修改</source><translation>Unsaved Product Changes</translation></message><message><source>标定点数</source><translation>Calibration Points</translation></message><message><source>浮点</source><translation>Floating-Point</translation></message><message><source>确定</source><translation>OK</translation></message>
<message><source>确定删除产品“%1 / %2”吗？&#10;主界面设备库中将不再提供该候选产品。</source><translation>Delete product “%1 / %2”?&#10;It will no longer be available in the main-window device library.</translation></message><message><source>编辑产品：%1</source><translation>Edit Product: %1</translation></message><message><source>菜单</source><translation>Menu</translation></message><message><source>规格型号 *</source><translation>Model *</translation></message><message><source>请先选择要复制的产品。</source><translation>Select a product to duplicate first.</translation></message><message><source>请填写产品名称、产品类别和规格型号。</source><translation>Enter the product name, category, and model.</translation></message><message><source>请输入产品管理密码：</source><translation>Enter the Product Management password:</translation></message><message><source>读取产品库失败：%1</source><translation>Failed to read the product catalog: %1</translation></message><message><source>输入方式</source><translation>Input Method</translation></message><message><source>选择一个产品进行编辑</source><translation>Select a product to edit</translation></message><message><source>通信与标定</source><translation>Communication and Calibration</translation></message><message><source>默认值</source><translation>Default Value</translation></message><message><source>默认值 / 菜单项</source><translation>Default Value / Menu Items</translation></message>
</context>
<context><name>ProductTableModel</name>
<message><source>%1&#10;%2&#10;%3&#10;数据项：%4</source><translation>%1&#10;%2&#10;%3&#10;Data items: %4</translation></message><message><source>、</source><translation>, </translation></message><message><source>产品名称</source><translation>Product Name</translation></message><message><source>产品类别</source><translation>Product Category</translation></message><message><source>参数数</source><translation>Parameter Count</translation></message><message><source>数据项</source><translation>Data Items</translation></message><message><source>无</source><translation>None</translation></message><message><source>标定点</source><translation>Calibration Points</translation></message><message><source>规格型号</source><translation>Model</translation></message><message><source>通信</source><translation>Communication</translation></message>
</context>
<context><name>SpectrumPlotWidget</name>
<message><source>幅值</source><translation>Amplitude</translation></message><message><source>数据曲线</source><translation>Data Curve</translation></message><message><source>时间</source><translation>Time</translation></message><message><source>等待测量数据</source><translation>Waiting for measurement data</translation></message>
</context>
<context><name>main</name><message><source>无法加载所选语言资源。</source><translation>Unable to load the selected language resources.</translation></message><message><source>语言切换失败</source><translation>Language Switch Failed</translation></message></context>
<context><name>MainWindow</name>
<message><source>%1（%2/%3）</source><translation>%1 (%2/%3)</translation></message>
<message><source>菜单</source><translation>Menu</translation></message>
<message><source>字符</source><translation>Text</translation></message>
<message><source>整数</source><translation>Integer</translation></message>
<message><source>浮点</source><translation>Floating-Point</translation></message>
</context>
<context><name>SerialSession</name>
<message><source>%1失败：%2</source><translation>%1 failed: %2</translation></message>
<message><source>读取串口参数</source><translation>Read serial port parameters</translation></message>
<message><source>配置串口参数</source><translation>Configure serial port parameters</translation></message>
<message><source>配置串口超时</source><translation>Configure serial port timeouts</translation></message>
<message><source>当前 POSIX 平台不支持波特率 %1</source><translation>Baud rate %1 is not supported on this POSIX platform</translation></message>
<message><source>串口号不能为空</source><translation>Serial port name cannot be empty</translation></message>
<message><source>波特率必须大于0</source><translation>Baud rate must be greater than 0</translation></message>
<message><source>打开串口 %1</source><translation>Open serial port %1</translation></message>
<message><source>当前操作系统不支持 POSIX 或 Win32 串口 API</source><translation>The current operating system does not support the POSIX or Win32 serial port API</translation></message>
<message><source>串口未连接</source><translation>Serial port is not connected</translation></message>
<message><source>发送帧不能为空</source><translation>The frame to send cannot be empty</translation></message>
<message><source>发送串口数据</source><translation>Send serial port data</translation></message>
<message><source>串口发送不完整：%1/%2 字节</source><translation>Incomplete serial write: %1/%2 bytes</translation></message>
<message><source>串口发送已取消</source><translation>Serial write canceled</translation></message>
<message><source>串口发送超时：%1/%2 字节</source><translation>Serial write timed out: %1/%2 bytes</translation></message>
<message><source>接收缓冲区不能为空</source><translation>Receive buffer is null</translation></message>
<message><source>期望接收字节数无效</source><translation>Invalid expected byte count</translation></message>
<message><source>串口读取已取消</source><translation>Serial read canceled</translation></message>
<message><source>读取串口数据</source><translation>Read serial port data</translation></message>
<message><source>串口读取超时：收到 %1/%2 字节</source><translation>Serial read timed out: received %1/%2 bytes</translation></message>
<message><source>响应缓冲区不能为空</source><translation>Response buffer is null</translation></message>
<message><source>AT/配置命令不能为空</source><translation>AT/configuration command cannot be empty</translation></message>
<message><source>串口读取超时参数无效</source><translation>Invalid serial read timeout</translation></message>
<message><source>读取串口文本回包</source><translation>Read text response from serial port</translation></message>
<message><source>串口文本回包超过 1 MiB 限制</source><translation>Serial text response exceeds the 1 MiB limit</translation></message>
<message><source>串口等待数据超时：%1 ms</source><translation>Timed out waiting for serial data after %1 ms</translation></message>
<message><source>已连接</source><translation>Connected</translation></message>
<message><source>未连接</source><translation>Not Connected</translation></message>
</context>
<context><name>LabviewDatabase</name>
<message><source>无法读取数据文件：%1</source><translation>Unable to read data file: %1</translation></message>
<message><source>无法打开 SQLite：%1</source><translation>Unable to open SQLite database: %1</translation></message>
<message><source>无法读取 TDMS：%1</source><translation>Unable to read TDMS file: %1</translation></message>
<message><source>不是可识别的 TDMS 文件：%1</source><translation>Unrecognized TDMS file: %1</translation></message>
<message><source>TDMS 原始数据越界：%1</source><translation>TDMS raw data is out of bounds: %1</translation></message>
<message><source>LabVIEW 数据目录不存在：%1</source><translation>LabVIEW data directory does not exist: %1</translation></message>
<message><source>无法覆盖 SQLite 文件：%1</source><translation>Unable to replace SQLite file: %1</translation></message>
<message><source>产品记录不能为空</source><translation>Product record cannot be empty</translation></message>
<message><source>产品名称、产品类别和规格型号不能为空</source><translation>Product name, category, and model cannot be empty</translation></message>
<message><source>相同名称、类别和型号的产品已经存在</source><translation>A product with the same name, category, and model already exists</translation></message>
<message><source>要修改的产品已不存在</source><translation>The product to update no longer exists</translation></message>
<message><source>产品记录 ID 无效</source><translation>Invalid product record ID</translation></message>
<message><source>要删除的产品已不存在</source><translation>The product to delete no longer exists</translation></message>
<message><source>标定记录的曲线名或点号无效</source><translation>Invalid curve name or point number in the calibration record</translation></message>
<message><source>要修改的标定记录已不存在</source><translation>The calibration record to update no longer exists</translation></message>
<message><source>标定记录 ID 无效</source><translation>Invalid calibration record ID</translation></message>
<message><source>要删除的标定记录已不存在</source><translation>The calibration record to delete no longer exists</translation></message>
<message><source>总线设备名称或地址无效</source><translation>Invalid bus device name or address</translation></message>
<message><source>要修改的总线设备已不存在</source><translation>The bus device to update no longer exists</translation></message>
<message><source>总线设备记录 ID 无效</source><translation>Invalid bus device record ID</translation></message>
<message><source>要删除的总线设备已不存在</source><translation>The bus device to delete no longer exists</translation></message>
<message><source>频振索力参数记录不能为空</source><translation>The frequency/tension parameter list cannot be null</translation></message>
<message><source>传感器名称不能为空</source><translation>Sensor name cannot be empty</translation></message>
<message><source>传感器名称“%1”重复</source><translation>Sensor name “%1” is duplicated</translation></message>
<message><source>频振索力参数包含非有限数值</source><translation>Frequency/tension parameters contain a non-finite value</translation></message>
<message><source>支座系数、单位质量、索长、截面积、弹性模量和截面惯性矩必须大于 0</source><translation>Support factor, unit mass, cable length, area, elastic modulus, and moment of inertia must be greater than 0</translation></message>
<message><source>要修改的频振索力参数已不存在</source><translation>The frequency/tension parameter record to update no longer exists</translation></message>
</context>
<context><name>VirtualDevice</name>
<message><source>测量完成，正在读取数据</source><translation>Measurement completed; reading data</translation></message>
<message><source>字符串数据读取成功</source><translation>Text data read successfully</translation></message>
<message><source>频谱数据读取成功</source><translation>Spectrum data read successfully</translation></message>
<message><source>DTU 网络配置保存成功</source><translation>DTU network configuration saved successfully</translation></message>
<message><source>扩展参数配置成功</source><translation>Extended parameters configured successfully</translation></message>
<message><source>传感器测试命令完成</source><translation>Sensor test command completed</translation></message>
<message><source>虚拟设备不识别该命令</source><translation>The virtual device does not recognize this command</translation></message>
<message><source>虚拟设备测量完成</source><translation>Virtual device measurement completed</translation></message>
<message><source>DTU 网络配置格式错误</source><translation>Invalid DTU network configuration format</translation></message>
<message><source>DTU 网络配置写入成功</source><translation>DTU network configuration written successfully</translation></message>
<message><source>设备参数数量错误</source><translation>Invalid device parameter count</translation></message>
<message><source>设备参数配置成功</source><translation>Device parameters configured successfully</translation></message>
</context>
<context><name>AppConfig</name>
<message><source>透传模式</source><translation>Transparent Mode</translation></message>
</context>
<context><name>AtProtocol</name>
<message><source>不支持该二级命令</source><translation>This secondary command is not supported</translation></message>
<message><source>二级命令无效</source><translation>Invalid secondary command</translation></message>
<message><source>命令参数无效</source><translation>Invalid command parameters</translation></message>
<message><source>模块执行命令失败</source><translation>The module failed to execute the command</translation></message>
<message><source>缺少 SET 命令类型</source><translation>Missing SET command type</translation></message>
<message><source>未定义的 SET 命令</source><translation>Undefined SET command</translation></message>
</context>
<context><name>VirtualModbusClient</name>
<message><source>CRC 错误或帧长度不足</source><translation>CRC error or frame is too short</translation></message>
<message><source>忽略从站 %1 的帧</source><translation>Ignoring frame from slave %1</translation></message>
<message><source>不支持功能码 0x%1</source><translation>Unsupported function code 0x%1</translation></message>
<message><source>测量成功，可以读取输入寄存器</source><translation>Measurement succeeded; input registers are ready</translation></message>
<message><source>MCU 参数保存成功</source><translation>MCU parameters saved successfully</translation></message>
<message><source>LoRa 参数保存成功</source><translation>LoRa parameters saved successfully</translation></message>
<message><source>复位/校准成功</source><translation>Reset/calibration succeeded</translation></message>
<message><source>恢复出厂设置成功</source><translation>Factory settings restored successfully</translation></message>
<message><source>设备重启完成</source><translation>Device restart completed</translation></message>
<message><source>B7 命令成功（设备搜索或电池读取）</source><translation>B7 command succeeded (device search or battery read)</translation></message>
<message><source>B8 命令成功（连续采样启动或附加数据读取）</source><translation>B8 command succeeded (continuous sampling start or additional data read)</translation></message>
<message><source>B9 命令成功（连续采样停止或 DTU 参数保存）</source><translation>B9 command succeeded (continuous sampling stop or DTU parameter save)</translation></message>
<message><source>测量失败</source><translation>Measurement failed</translation></message>
<message><source>MCU 参数保存失败</source><translation>Failed to save MCU parameters</translation></message>
<message><source>LoRa 参数保存失败</source><translation>Failed to save LoRa parameters</translation></message>
<message><source>复位/校准失败</source><translation>Reset/calibration failed</translation></message>
<message><source>恢复出厂设置失败</source><translation>Failed to restore factory settings</translation></message>
<message><source>B7 命令失败（电池读取失败）</source><translation>B7 command failed (battery read failed)</translation></message>
<message><source>连续采样正忙</source><translation>Continuous sampling is busy</translation></message>
<message><source>连续采样启动失败</source><translation>Failed to start continuous sampling</translation></message>
<message><source>连续采样忙</source><translation>Continuous sampling busy</translation></message>
<message><source>连续采样失败</source><translation>Continuous sampling failed</translation></message>
<message><source>等待设备处理</source><translation>Waiting for device processing</translation></message>
<message><source>无有效状态码</source><translation>No valid status code</translation></message>
</context>
<context><name>AtProtocol</name>
<message><source>命令无效</source><translation>Invalid command</translation></message>
</context>
<context><name>MainWindow</name>
<message><source>MCU模块</source><translation>MCU Module</translation></message>
<message><source>波特率</source><translation>Baud Rate</translation></message>
<message><source>采样间隔</source><translation>Sampling Interval</translation></message>
<message><source>工作模式</source><translation>Operating Mode</translation></message>
<message><source>发射功率</source><translation>Tx Power</translation></message>
<message><source>空中速率</source><translation>Air Data Rate</translation></message>
<message><source>主从模式</source><translation>Host/Slave Mode</translation></message>
<message><source>本地组号</source><translation>Local Group ID</translation></message>
<message><source>本地地址</source><translation>Local Address</translation></message>
<message><source>目标组号</source><translation>Target Group ID</translation></message>
<message><source>目标地址</source><translation>Target Address</translation></message>
<message><source>DTU模块</source><translation>DTU Module</translation></message>
<message><source>曲线名称</source><translation>Curve Name</translation></message>
<message><source>点号</source><translation>Point No.</translation></message>
<message><source>测量值</source><translation>Measured Value</translation></message>
<message><source>标定值</source><translation>Reference Value</translation></message>
<message><source>温度</source><translation>Temperature</translation></message>
<message><source>应用到配置表</source><translation>Apply Changes</translation></message>
<message><source>添加至设备</source><translation>Add to Config</translation></message>
<message><source>清空配置表</source><translation>Clear Table</translation></message>
<message><source>网络配置</source><translation>Configure</translation></message>
<message><source>重启设备</source><translation>Restart</translation></message>
<message><source>不加密</source><translation>None</translation></message>
<message><source>无证书加密</source><translation>No Certificate</translation></message>
<message><source>有证书加密</source><translation>Certificate</translation></message>
</context>
<context><name>DeviceTableModel</name>
<message><source>设备类别</source><translation>Category</translation></message>
</context>
<context><name>FrequencyTensionParameterDialog</name>
<message><source>频振索力传感器扩展参数管理器</source><translation>Frequency/Tension Sensor Extended Parameter Manager</translation></message>
<message><source>参数保存在本机数据库中，无需连接传感器即可添加、修改和保存。</source><translation>Parameters are stored in the local database. You can add, edit, and save them without connecting a sensor.</translation></message>
<message><source>参数操作</source><translation>Actions</translation></message>
<message><source>添加</source><translation>Add</translation></message>
<message><source>修改</source><translation>Edit</translation></message>
<message><source>保存</source><translation>Save</translation></message>
<message><source>关闭</source><translation>Close</translation></message>
<message><source>传感器名</source><translation>Sensor Name</translation></message>
<message><source>支座系数 μ</source><translation>Support Factor μ</translation></message>
<message><source>单位质量 G (kg/m)</source><translation>Unit Mass G (kg/m)</translation></message>
<message><source>索长 L (m)</source><translation>Cable Length L (m)</translation></message>
<message><source>截面积 A (cm²)</source><translation>Area A (cm²)</translation></message>
<message><source>弹性模量 E (MPa)</source><translation>Elastic Modulus E (MPa)</translation></message>
<message><source>截面惯性矩 I (cm⁴)</source><translation>Moment of Inertia I (cm⁴)</translation></message>
<message><source>水平夹角 θ (°)</source><translation>Horizontal Angle θ (°)</translation></message>
<message><source>参数编辑</source><translation>Parameter Editor</translation></message>
<message><source>例如 Sen01</source><translation>For example, Sen01</translation></message>
<message><source>选择表格中的参数可进行修改。</source><translation>Select a table row to edit its parameters.</translation></message>
<message><source>读取参数失败</source><translation>Failed to Load Parameters</translation></message>
<message><source>已加载 %1 组本地参数。</source><translation>Loaded %1 local parameter set(s).</translation></message>
<message><source>正在编辑：%1</source><translation>Editing: %1</translation></message>
<message><source>参数校验</source><translation>Parameter Validation</translation></message>
<message><source>传感器名称不能为空。</source><translation>Sensor name cannot be empty.</translation></message>
<message><source>单位质量 G</source><translation>Unit Mass G</translation></message>
<message><source>索长 L</source><translation>Cable Length L</translation></message>
<message><source>截面积 A</source><translation>Area A</translation></message>
<message><source>弹性模量 E</source><translation>Elastic Modulus E</translation></message>
<message><source>截面惯性矩 I</source><translation>Moment of Inertia I</translation></message>
<message><source>水平夹角 θ</source><translation>Horizontal Angle θ</translation></message>
<message><source>%1 必须是大于 0 的有效数字。</source><translation>%1 must be a valid number greater than 0.</translation></message>
<message><source>%1 必须是有效数字。</source><translation>%1 must be a valid number.</translation></message>
<message><source>添加参数</source><translation>Add Parameters</translation></message>
<message><source>传感器名称“%1”已经存在。请修改现有记录或使用其他名称。</source><translation>Sensor name “%1” already exists. Edit the existing record or use another name.</translation></message>
<message><source>已添加参数“%1”，请点击保存写入数据库。</source><translation>Added parameters for “%1”. Click Save to write them to the database.</translation></message>
<message><source>修改参数</source><translation>Edit Parameters</translation></message>
<message><source>请先选择要修改的参数。</source><translation>Select the parameter set to edit first.</translation></message>
<message><source>传感器名称“%1”已经存在。请使用其他名称。</source><translation>Sensor name “%1” already exists. Use another name.</translation></message>
<message><source>已修改参数“%1”，请点击保存写入数据库。</source><translation>Updated parameters for “%1”. Click Save to write them to the database.</translation></message>
<message><source>保存参数失败</source><translation>Failed to Save Parameters</translation></message>
<message><source>参数已保存到本机数据库。</source><translation>Parameters were saved to the local database.</translation></message>
<message><source>未保存的参数修改</source><translation>Unsaved Parameter Changes</translation></message>
<message><source>本地参数列表有未保存的修改。</source><translation>The local parameter list has unsaved changes.</translation></message>
</context>
<context><name>MeasurementTableModel</name>
<message><source>测量值</source><translation>Value</translation></message>
<message><source>索力(kN)</source><translation>Force (kN)</translation></message>
</context>
<context><name>CalibrationTableModel</name>
<message><source>标定值</source><translation>Reference</translation></message>
<message><source>温度(℃)</source><translation>Temp. (°C)</translation></message>
</context>
<context><name>ProductManagementDialog</name>
<message><source>参数名称</source><translation>Parameter</translation></message>
<message><source>输入方式</source><translation>Input</translation></message>
<message><source>默认值 / 菜单项</source><translation>Default / Options</translation></message>
</context>
<context><name>ProductTableModel</name>
<message><source>产品名称</source><translation>Name</translation></message>
<message><source>产品类别</source><translation>Category</translation></message>
<message><source>参数数</source><translation>Parameters</translation></message>
<message><source>通信</source><translation>Interfaces</translation></message>
</context>
<context><name>DtuConfigDialog</name>
<message><source>DTU 网络配置</source><translation>DTU Network Configuration</translation></message>
<message><source>连接参数</source><translation>Connection</translation></message>
<message><source>网络协议</source><translation>Protocol</translation></message>
<message><source>服务器地址</source><translation>Server Address</translation></message>
<message><source>服务器端口</source><translation>Server Port</translation></message>
<message><source>通道 ID</source><translation>Channel ID</translation></message>
<message><source>IP 版本</source><translation>IP Version</translation></message>
<message><source>SSL 加密</source><translation>SSL Encryption</translation></message>
<message><source>不加密</source><translation>None</translation></message>
<message><source>无证书加密</source><translation>TLS Without Certificate</translation></message>
<message><source>有证书加密</source><translation>TLS With Certificate</translation></message>
<message><source>HTTP 参数</source><translation>HTTP</translation></message>
<message><source>请求方法</source><translation>Method</translation></message>
<message><source>URL 路径</source><translation>URL Path</translation></message>
<message><source>响应超时</source><translation>Response Timeout</translation></message>
<message><source> 秒</source><translation> s</translation></message>
<message><source>MQTT 参数</source><translation>MQTT</translation></message>
<message><source>用户名</source><translation>Username</translation></message>
<message><source>密码</source><translation>Password</translation></message>
<message><source>显示密码</source><translation>Show Password</translation></message>
<message><source>心跳时间</source><translation>Keep Alive</translation></message>
<message><source>MQTT 版本</source><translation>MQTT Version</translation></message>
<message><source>订阅 Topic</source><translation>Subscribe Topic</translation></message>
<message><source>发布 Topic</source><translation>Publish Topic</translation></message>
<message><source>订阅 QoS</source><translation>Subscribe QoS</translation></message>
<message><source>发布 QoS</source><translation>Publish QoS</translation></message>
<message><source>清除会话</source><translation>Clean Session</translation></message>
<message><source>保留发布消息</source><translation>Retain Published Message</translation></message>
<message><source>TCP/UDP 参数</source><translation>TCP/UDP</translation></message>
<message><source>写入并保存</source><translation>Write and Save</translation></message>
<message><source>取消</source><translation>Cancel</translation></message>
<message><source>DTU 参数</source><translation>DTU Parameters</translation></message>
<message><source>参数不完整或包含逗号、换行等非法字符，命令总长度不能超过 254 字节。</source><translation>Parameters are incomplete or contain commas or line breaks. The command must not exceed 254 bytes.</translation></message>
<message><source>DTU 网络通道配置</source><translation>DTU Network Channel</translation></message>
<message><source>是否启动</source><translation>Enabled</translation></message>
<message><source>启动</source><translation>Enabled</translation></message>
<message><source>停止</source><translation>Disabled</translation></message>
<message><source>网络通信协议</source><translation>Network Protocol</translation></message>
<message><source>绑定通讯串口</source><translation>Serial Channel</translation></message>
<message><source>心跳包发送间隔时间</source><translation>Heartbeat Interval</translation></message>
<message><source>登录客户端 ID</source><translation>Client ID</translation></message>
<message><source>登录用户名</source><translation>Username</translation></message>
<message><source>登录密码</source><translation>Password</translation></message>
<message><source>协议版本</source><translation>Protocol Version</translation></message>
<message><source>离线自动销毁</source><translation>Clean Session</translation></message>
<message><source>持久会话</source><translation>Persistent Session</translation></message>
<message><source>持久消息</source><translation>Retained Message</translation></message>
<message><source>否</source><translation>No</translation></message>
<message><source>是</source><translation>Yes</translation></message>
<message><source>订阅消息主题</source><translation>Subscribe Topic</translation></message>
<message><source>发布消息主题</source><translation>Publish Topic</translation></message>
<message><source>设置遗嘱</source><translation>Last Will</translation></message>
<message><source>遗嘱 QoS</source><translation>Will QoS</translation></message>
<message><source>遗嘱持久消息</source><translation>Retain Will</translation></message>
<message><source>遗嘱 Topic</source><translation>Will Topic</translation></message>
<message><source>遗嘱内容</source><translation>Will Payload</translation></message>
<message><source>登录注册信息</source><translation>Registration Packet</translation></message>
<message><source>登录注册数据</source><translation>Registration Data</translation></message>
<message><source>不发送</source><translation>Do Not Send</translation></message>
<message><source>固定格式</source><translation>Fixed Format</translation></message>
<message><source>16进制</source><translation>HEX</translation></message>
<message><source>字符串</source><translation>String</translation></message>
<message><source>自定义函数</source><translation>Custom Function</translation></message>
<message><source>支持 IPv6</source><translation>IPv6</translation></message>
<message><source>支持 SSL</source><translation>SSL</translation></message>
<message><source>请求 URL</source><translation>Request URL</translation></message>
<message><source>等待超时时间</source><translation>Request Timeout</translation></message>
<message><source>是否自定义头部</source><translation>Custom Header</translation></message>
<message><source>不添加</source><translation>Do Not Add</translation></message>
<message><source>添加</source><translation>Add</translation></message>
<message><source>自定义头部数据</source><translation>Header Data</translation></message>
<message><source>返回数据过滤</source><translation>Response Filter</translation></message>
<message><source>不过滤</source><translation>Disabled</translation></message>
<message><source>过滤</source><translation>Enabled</translation></message>
<message><source>心跳包开关</source><translation>Heartbeat</translation></message>
<message><source>开</source><translation>On</translation></message>
<message><source>关</source><translation>Off</translation></message>
<message><source>心跳包数据类型</source><translation>Heartbeat Data Type</translation></message>
<message><source>心跳包数据</source><translation>Heartbeat Data</translation></message>
<message><source>数据前置字段</source><translation>Data Prefix</translation></message>
<message><source>前置字段数据</source><translation>Prefix Data</translation></message>
<message><source>数据后置字段</source><translation>Data Suffix</translation></message>
<message><source>后置字段数据</source><translation>Suffix Data</translation></message>
<message><source>保存参数</source><translation>Save Parameters</translation></message>
<message><source>返回</source><translation>Back</translation></message>
<message><source>停用通道</source><translation>Disable Channel</translation></message>
<message><source>通道 1 | 已停用</source><translation>Channel 1 | Disabled</translation></message>
<message><source>请完整填写当前协议的必要参数；字段不能包含逗号或换行，命令总长度不能超过 254 字节。</source><translation>Complete all required fields. Commas and line breaks are not allowed, and the command must not exceed 254 bytes.</translation></message>
</context>
<context><name>MainWindow</name>
<message><source>检测状态</source><translation>Check Status</translation></message>
<message><source>配置摘要</source><translation>Configuration Summary</translation></message>
<message><source>DTU 配置命令无效。</source><translation>The DTU configuration command is invalid.</translation></message>
<message><source>设备数据</source><translation>Device Data</translation></message>
<message><source>模块功能</source><translation>Module Features</translation></message>
<message><source>先选择设备，再按模块完成参数读取、编辑和写入。</source><translation>Select a device, then read, edit, and write its parameters by module.</translation></message>
<message><source>通信控制</source><translation>Communication</translation></message>
<message><source>可用设备</source><translation>Available Devices</translation></message>
<message><source>通信状态</source><translation>Communication Status</translation></message>
<message><source>串口参数</source><translation>Serial Parameters</translation></message>
<message><source>集中查看采集数据、最新状态和趋势曲线。</source><translation>Review acquired data, current status, and trends in one place.</translation></message>
<message><source>接收概览</source><translation>Receive Overview</translation></message>
<message><source>接收游标</source><translation>Receive Cursor</translation></message>
<message><source>接收字长</source><translation>Received Length</translation></message>
<message><source>剩余字长</source><translation>Remaining Length</translation></message>
<message><source>测量数据</source><translation>Measurement Data</translation></message>
<message><source>最新数据</source><translation>Latest Data</translation></message>
<message><source>曲线</source><translation>Curve</translation></message>
<message><source>管理标定点，并在右侧完成测量与保存。</source><translation>Manage calibration points and measure or save them in the panel on the right.</translation></message>
<message><source>标定记录</source><translation>Calibration Records</translation></message>
<message><source>维护总线设备，并集中处理 DTU 与 AT 指令。</source><translation>Maintain bus devices and handle DTU and AT commands in one place.</translation></message>
<message><source>设备管理</source><translation>Devices</translation></message>
</context>
<context><name>AtProtocol</name>
<message><source>DTU 命令执行成功</source><translation>DTU command completed successfully</translation></message>
<message><source>DTU 命令执行失败（错误码 %1）</source><translation>DTU command failed (error code %1)</translation></message>
<message><source>未知</source><translation>Unknown</translation></message>
</context>
<context><name>MainWindow</name>
<message><source>测试数据</source><translation>Test Data</translation></message>
<message><source>等待测量数据</source><translation>Waiting for measurement data</translation></message>
<message><source>数据曲线</source><translation>Data Curve</translation></message>
<message><source>设备配置</source><translation>Device Configuration</translation></message>
<message><source>设备标定</source><translation>Device Calibration</translation></message>
<message><source>总线设备/网关</source><translation>Bus Devices/Gateway</translation></message>
<message><source>索力 %1 kN，fn %2 Hz</source><translation>Cable force %1 kN, fn %2 Hz</translation></message>
<message><source>请先选择频振索力传感器，再读取频谱。</source><translation>Select a frequency/tension sensor before reading the spectrum.</translation></message>
<message><source>频谱数据</source><translation>Spectrum Data</translation></message>
<message><source>索力测量数据</source><translation>Cable Force Measurements</translation></message>
<message><source>峰值 %1 Hz / %2</source><translation>Peak %1 Hz / %2</translation></message>
<message><source>频谱回包未包含有效数据。</source><translation>The spectrum response contains no valid data.</translation></message>
<message><source>频谱回包未包含可配对的频率和幅值数据。</source><translation>The spectrum response does not contain matching frequency and amplitude data.</translation></message>
<message><source>索力测量回包缺少索力、fn、阶数或收敛误差。</source><translation>The cable-force response is missing force, fn, order, or convergence error.</translation></message>
<message><source>索力测量完成：%1 kN，fn=%2 Hz，n=%3，误差=%4%</source><translation>Cable-force measurement complete: %1 kN, fn=%2 Hz, n=%3, error=%4%</translation></message>
<message><source>删除选中记录</source><translation>Delete Selected Records</translation></message>
<message><source>保存测量数据为 CSV…</source><translation>Save Measurement Data as CSV…</translation></message>
<message><source>串口配置无效，参数仅保存在本机。</source><translation>Invalid serial configuration; the parameters were saved locally only.</translation></message>
<message><source>已有通信任务正在运行，请稍后重试。</source><translation>A communication task is already running. Try again later.</translation></message>
<message><source>无法启动扩展参数写入任务。</source><translation>Could not start the extended-parameter write operation.</translation></message>
<message><source>删除测量数据</source><translation>Delete Measurement Data</translation></message>
<message><source>请先选择要删除的测量记录。</source><translation>Select the measurement records to delete first.</translation></message>
<message><source>确定删除选中的 %1 条测量记录吗？</source><translation>Delete the selected %1 measurement record(s)?</translation></message>
<message><source>已删除 %1 条测量记录</source><translation>Deleted %1 measurement record(s)</translation></message>
<message><source>保存测量数据</source><translation>Save Measurement Data</translation></message>
<message><source>当前没有可保存的测量数据。</source><translation>There is no measurement data to save.</translation></message>
<message><source>CSV 文件 (*.csv)</source><translation>CSV Files (*.csv)</translation></message>
<message><source>保存测量数据失败</source><translation>Failed to Save Measurement Data</translation></message>
<message><source>测量数据已保存：%1</source><translation>Measurement data saved: %1</translation></message>
<message><source>采样频率</source><translation>Sampling Frequency</translation></message>
<message><source>采样点数</source><translation>Sample Count</translation></message>
<message><source>采样频率和采样点数请在 MCU 模块中读取和配置。</source><translation>Read and configure the sampling frequency and sample count in the MCU module.</translation></message>
<message><source>保存数据…</source><translation>Save Data…</translation></message>
<message><source>删除选中</source><translation>Delete Selected</translation></message>
<message><source>将当前测量列表保存为 CSV；也可在表格内右键保存。</source><translation>Save the current measurement list as a CSV file. You can also save it from the table's context menu.</translation></message>
<message><source>删除表格中选中的测量记录；也可按 Delete 键。</source><translation>Delete the selected measurement records. You can also press Delete.</translation></message>
<message><source>采样参数无效或已有通信任务正在运行</source><translation>The sampling parameters are invalid, or another communication task is already running.</translation></message>
<message><source>停止测量</source><translation>Stop Measurement</translation></message>
</context>
<context><name>MeasurementTableModel</name>
<message><source>序号</source><translation>Index</translation></message>
<message><source>设备名</source><translation>Device Name</translation></message>
<message><source>n(阶数)</source><translation>n (Order)</translation></message>
<message><source>收敛误差(%)</source><translation>Convergence Error (%)</translation></message>
<message><source>点号</source><translation>Point No.</translation></message>
<message><source>频率(Hz)</source><translation>Frequency (Hz)</translation></message>
<message><source>幅值</source><translation>Amplitude</translation></message>
<message><source>CSV 文件路径不能为空</source><translation>The CSV file path cannot be empty.</translation></message>
<message><source>无法打开 CSV 文件：%1</source><translation>Could not open the CSV file: %1</translation></message>
<message><source>保存 CSV 文件失败：%1</source><translation>Failed to save the CSV file: %1</translation></message>
</context>
<context><name>FrequencyTensionParameterDialog</name>
<message><source>参数可离线保存到本机数据库；连接传感器后，保存时可选择写入并回读校验。</source><translation>Parameters can be saved offline to the local database. When a sensor is connected, Save can also write them to the sensor and verify the readback.</translation></message>
<message><source>写入传感器失败：%1</source><translation>Failed to write to the sensor: %1</translation></message>
<message><source>设备已确认写入，但 AT,get,FVCFexppar 未返回完整的 7 个参数，无法完成回读校验。</source><translation>The device confirmed the write, but AT,get,FVCFexppar did not return all seven parameters, so readback verification could not be completed.</translation></message>
<message><source>传感器回读值与设置值不一致：第 %1 项期望 %2，回读 %3。</source><translation>Sensor readback does not match: parameter %1 expected %2, but the sensor returned %3.</translation></message>
<message><source>扩展参数已写入传感器，且 AT,get,FVCFexppar 回读校验一致。</source><translation>The extended parameters were written to the sensor and verified by AT,get,FVCFexppar readback.</translation></message>
<message><source>写入传感器</source><translation>Write to Sensor</translation></message>
<message><source>本地参数已保存。是否将当前参数写入传感器？&#10;&#10;将发送 AT,set,exppar,...，随后使用 AT,get,FVCFexppar 回读校验。</source><translation>The local parameters were saved. Write the current parameters to the sensor?&#10;&#10;The app will send AT,set,exppar,... and then verify the values by reading them back with AT,get,FVCFexppar.</translation></message>
<message><source>正在写入传感器并回读校验，请稍候……</source><translation>Writing to the sensor and verifying the readback…</translation></message>
<message><source>“保存到本机”仅更新本地数据库；“写入传感器并校验”会发送 AT,set,exppar,...，再用 AT,get,FVCFexppar 回读确认。</source><translation>“Save Locally” updates only the local database. “Write to Sensor and Verify” sends AT,set,exppar,... and then confirms the values by reading them back with AT,get,FVCFexppar.</translation></message>
<message><source>保存到本机</source><translation>Save Locally</translation></message>
<message><source>写入传感器并校验</source><translation>Write to Sensor and Verify</translation></message>
<message><source>请先选择要写入传感器的参数。</source><translation>Select the parameters to write to the sensor first.</translation></message>
</context>
<context><name>DeviceCommunicationController</name>
<message><source>测量超时：等待 %1 秒后状态仍为 0x%2 %3</source><translation>Measurement timed out: status remained 0x%2 %3 after waiting %1 seconds</translation></message>
<message><source>Modbus 参数写入回包与请求地址或数量不一致</source><translation>The Modbus parameter-write response does not match the requested address or register count.</translation></message>
<message><source>MCU 与采样参数已写入 RAM，但保存命令失败：%1</source><translation>The MCU and sampling parameters were written to RAM, but the save command failed: %1</translation></message>
<message><source>MCU 参数保存命令回显不一致</source><translation>The MCU parameter save-command echo does not match the request.</translation></message>
<message><source>参数已保存，但 MCU 参数回读失败：%1</source><translation>The parameters were saved, but the MCU parameter readback failed: %1</translation></message>
<message><source>参数已保存，但采样参数回读失败：%1</source><translation>The parameters were saved, but the sampling-parameter readback failed: %1</translation></message>
<message><source>参数已保存，但 MCU 或采样参数回读不完整</source><translation>The parameters were saved, but the MCU or sampling-parameter readback is incomplete.</translation></message>
<message><source>MCU 参数回读不一致：第 %1 项期望 %2，回读 %3</source><translation>MCU parameter readback mismatch: item %1 expected %2 but returned %3.</translation></message>
<message><source>采样参数回读不一致：期望频率索引 %1 / 点数索引 %2，回读 %3 / %4</source><translation>Sampling-parameter readback mismatch: expected rate index %1 / sample-count index %2, but returned %3 / %4.</translation></message>
<message><source>MCU 与采样参数写入、保存并回读成功</source><translation>The MCU and sampling parameters were written, saved, and verified successfully.</translation></message>
<message><source>读取 MCU 运行模式失败：%1</source><translation>Failed to read the MCU operating mode: %1</translation></message>
<message><source>读取频振索力采样配置失败：%1</source><translation>Failed to read the frequency/tension sampling configuration: %1</translation></message>
<message><source>设备返回的采样配置索引无效：频率 %1，点数 %2</source><translation>The device returned invalid sampling configuration indices: rate %1, sample count %2.</translation></message>
<message><source>当前 MCU 运行模式 %1 不支持频振索力测量</source><translation>MCU operating mode %1 does not support frequency/tension measurement.</translation></message>
<message><source>频振索力测量启动命令回显不一致</source><translation>The frequency/tension measurement start-command echo does not match the request.</translation></message>
<message><source>频振索力测量已取消</source><translation>The frequency/tension measurement was canceled.</translation></message>
<message><source>频振索力测量失败：0x%1 %2</source><translation>Frequency/tension measurement failed: 0x%1 %2</translation></message>
<message><source>频振索力测量超时：按 %1 Hz / %2 点等待 %3 秒后状态仍为 0x%4 %5</source><translation>Frequency/tension measurement timed out: after waiting %3 seconds for %1 Hz / %2 samples, the status remained 0x%4 %5.</translation></message>
<message><source>指令模式频振索力测量完成</source><translation>Command-mode frequency/tension measurement completed.</translation></message>
<message><source>自动模式收到一条频振索力测量数据</source><translation>Received one frequency/tension measurement in automatic mode.</translation></message>
<message><source>低功耗模式收到一条频振索力测量数据</source><translation>Received one frequency/tension measurement in low-power mode.</translation></message>
<message><source>自动模式连续测量已停止</source><translation>Continuous automatic measurement has stopped.</translation></message>
<message><source>自动模式收到无效测量状态：0x%1 %2</source><translation>Automatic mode received an invalid measurement status: 0x%1 %2</translation></message>
</context>
<context><name>SpectrumPlotWidget</name>
<message><source>测量序号</source><translation>Sample No.</translation></message>
<message><source>测量值</source><translation>Measured Value</translation></message>
<message><source>索力趋势</source><translation>Cable Force Trend</translation></message>
<message><source>索力(kN)</source><translation>Cable Force (kN)</translation></message>
<message><source>频谱曲线</source><translation>Spectrum</translation></message>
<message><source>频率(Hz)</source><translation>Frequency (Hz)</translation></message>
</context>
<context><name>AppConfig</name>
<message><source>CAN总线</source><translation>CAN Bus</translation></message>
</context>
<context><name>CanFrameTableModel</name>
<message><source>时间</source><translation>Time</translation></message>
<message><source>方向</source><translation>Direction</translation></message>
<message><source>帧类型</source><translation>Frame Type</translation></message>
<message><source>数据 (HEX)</source><translation>Data (HEX)</translation></message>
<message><source>发送</source><translation>TX</translation></message>
<message><source>接收</source><translation>RX</translation></message>
</context>
<context><name>gucds::PcanBasicSession</name>
<message><source>PCAN-USB 通道必须在 1 到 16 之间</source><translation>The PCAN-USB channel must be between 1 and 16</translation></message>
<message><source>不支持的 CAN 波特率：%1 bit/s</source><translation>Unsupported CAN bit rate: %1 bit/s</translation></message>
<message><source>打开 PCAN-USB 失败：%1</source><translation>Failed to open PCAN-USB: %1</translation></message>
<message><source>PCAN 通道实际波特率为 %1 bit/s，与请求的 %2 bit/s 不一致</source><translation>The actual PCAN channel bit rate is %1 bit/s, not the requested %2 bit/s</translation></message>
<message><source>PCAN 通道已打开，但驱动报告警告：%1</source><translation>The PCAN channel is open, but the driver reported a warning: %1</translation></message>
<message><source>关闭 PCAN 通道时驱动返回：%1</source><translation>The driver returned an error while closing the PCAN channel: %1</translation></message>
<message><source>PCAN 通道尚未连接</source><translation>The PCAN channel is not connected</translation></message>
<message><source>CAN 帧发送失败：%1</source><translation>Failed to send CAN frame: %1</translation></message>
<message><source>CAN 帧已提交，但驱动报告警告：%1</source><translation>The CAN frame was submitted, but the driver reported a warning: %1</translation></message>
<message><source>CAN ID 超出%1帧范围</source><translation>CAN ID exceeds the %1 frame range</translation></message>
<message><source>扩展</source><translation>extended</translation></message>
<message><source>标准</source><translation>standard</translation></message>
<message><source>CAN DLC 不能超过 8</source><translation>CAN DLC cannot exceed 8</translation></message>
<message><source>RTR 远程帧不能携带数据</source><translation>An RTR frame cannot carry data</translation></message>
<message><source>经典 CAN 数据不能超过 8 字节</source><translation>Classic CAN data cannot exceed 8 bytes</translation></message>
<message><source>CAN DLC 与数据长度不一致</source><translation>CAN DLC does not match the data length</translation></message>
<message><source>读取 PCAN 接收队列失败：%1</source><translation>Failed to read the PCAN receive queue: %1</translation></message>
<message><source>收到 PCAN 状态/错误帧（类型 0x%1）</source><translation>Received a PCAN status/error frame (type 0x%1)</translation></message>
<message><source>收到 PCAN 状态帧：%1</source><translation>Received a PCAN status frame: %1</translation></message>
<message><source>收到 CAN 错误帧（ID 0x%1）</source><translation>Received a CAN error frame (ID 0x%1)</translation></message>
<message><source>驱动返回了无效 CAN DLC：%1</source><translation>The driver returned an invalid CAN DLC: %1</translation></message>
<message><source>CAN 总线错误：%1</source><translation>CAN bus error: %1</translation></message>
<message><source>PCAN-Basic 后端当前仅支持 Windows</source><translation>The PCAN-Basic backend currently supports Windows only</translation></message>
<message><source>未找到 PCANBasic.dll；请安装 PCAN-Basic x64 驱动组件</source><translation>PCANBasic.dll was not found; install the PCAN-Basic x64 driver component</translation></message>
<message><source>%1: 缺少必要的 PCAN-Basic 导出函数</source><translation>%1: required PCAN-Basic exports are missing</translation></message>
<message><source>无法加载 PCAN-Basic API：%1</source><translation>Unable to load the PCAN-Basic API: %1</translation></message>
</context>
<context><name>gucds::CanMonitorWidget</name>
<message><source>CAN 总线调试</source><translation>CAN Bus Debugger</translation></message>
<message><source>通过 PCAN-USB 监视和发送经典 CAN 帧，并可一键验证 STM32F103 开发板。</source><translation>Monitor and transmit classic CAN frames through PCAN-USB, with one-click STM32F103 board verification.</translation></message>
<message><source>PCAN 连接与开发板状态</source><translation>PCAN Connection and Board Status</translation></message>
<message><source>连接</source><translation>Connect</translation></message>
<message><source>开发板自检</source><translation>Board Self-Test</translation></message>
<message><source>通道</source><translation>Channel</translation></message>
<message><source>波特率</source><translation>Bit Rate</translation></message>
<message><source>未连接</source><translation>Disconnected</translation></message>
<message><source>未知</source><translation>Unknown</translation></message>
<message><source>等待心跳</source><translation>Waiting for heartbeat</translation></message>
<message><source>总线</source><translation>Bus</translation></message>
<message><source>接收帧</source><translation>RX Frames</translation></message>
<message><source>发送帧</source><translation>TX Frames</translation></message>
<message><source>尚未加载</source><translation>Not loaded</translation></message>
<message><source>操作</source><translation>Operation</translation></message>
<message><source>就绪</source><translation>Ready</translation></message>
<message><source>发送经典 CAN 帧</source><translation>Send Classic CAN Frame</translation></message>
<message><source>最多 8 字节，例如 11 22 33 44</source><translation>Up to 8 bytes, for example 11 22 33 44</translation></message>
<message><source>扩展帧</source><translation>Extended Frame</translation></message>
<message><source>RTR 远程帧</source><translation>RTR Frame</translation></message>
<message><source>发送</source><translation>Send</translation></message>
<message><source>数据</source><translation>Data</translation></message>
<message><source>CAN 帧监视器</source><translation>CAN Frame Monitor</translation></message>
<message><source>最多保留最近 5000 帧</source><translation>Keeps the latest 5,000 frames</translation></message>
<message><source>清空</source><translation>Clear</translation></message>
<message><source>错误：%1</source><translation>Error: %1</translation></message>
<message><source>正常</source><translation>Normal</translation></message>
<message><source>FAIL：3 秒内未收到 F103 心跳帧 0x123</source><translation>FAIL: no F103 heartbeat frame 0x123 within 3 seconds</translation></message>
<message><source>FAIL：1 秒内未收到匹配的 0x322 回显</source><translation>FAIL: no matching 0x322 echo within 1 second</translation></message>
<message><source>经典 CAN 数据不能超过 8 字节</source><translation>Classic CAN data cannot exceed 8 bytes</translation></message>
<message><source>HEX 数据格式错误；每个字节必须包含两位十六进制数</source><translation>Invalid HEX data; each byte must contain two hexadecimal digits</translation></message>
<message><source>连接失败</source><translation>Connection failed</translation></message>
<message><source>断开</source><translation>Disconnect</translation></message>
<message><source>已连接</source><translation>Connected</translation></message>
<message><source>FAIL：PCAN 通道已断开</source><translation>FAIL: the PCAN channel was disconnected</translation></message>
<message><source>等待连接</source><translation>Waiting for connection</translation></message>
<message><source>PCAN 通道已连接</source><translation>PCAN channel connected</translation></message>
<message><source>心跳超时，开发板离线</source><translation>Heartbeat timed out; board is offline</translation></message>
<message><source>CAN ID 格式错误</source><translation>Invalid CAN ID</translation></message>
<message><source>在线，心跳 #%1</source><translation>Online, heartbeat #%1</translation></message>
<message><source>FAIL：回显正确，但 CAN 控制器报告总线错误</source><translation>FAIL: echo matched, but the CAN controller reported a bus error</translation></message>
<message><source>FAIL：回显正确，但 PCAN 状态为 0x%1</source><translation>FAIL: echo matched, but PCAN status is 0x%1</translation></message>
<message><source>已发送 0x%1，DLC %2</source><translation>Sent 0x%1, DLC %2</translation></message>
<message><source>PASS：500 kbit/s 心跳与 0x321 → 0x322 双向回显通过</source><translation>PASS: 500 kbit/s heartbeat and bidirectional 0x321 → 0x322 echo verified</translation></message>
<message><source>FAIL：无法打开 PCAN-USB</source><translation>FAIL: unable to open PCAN-USB</translation></message>
<message><source>FAIL：开发板自检必须选择 500 kbit/s</source><translation>FAIL: select 500 kbit/s for the board self-test</translation></message>
<message><source>自检：等待 0x123 心跳</source><translation>Self-test: waiting for 0x123 heartbeat</translation></message>
<message><source>FAIL：0x321 发送失败：%1</source><translation>FAIL: unable to send 0x321: %1</translation></message>
<message><source>自检：等待 0x322 回显</source><translation>Self-test: waiting for 0x322 echo</translation></message>
</context>
<context><name>QPlatformTheme</name>
<message><source>Cancel</source><translation>Cancel</translation></message>
</context>
</TS>
