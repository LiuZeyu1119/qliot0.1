#include "gucds/core/virtualmodbusclient.h"

#include <QCoreApplication>
#include <QStringList>
#include <QtGlobal>

#include <cstring>

namespace {

QString modbusText(const char *source)
{
    return QCoreApplication::translate("VirtualModbusClient", source);
}

} // namespace

namespace gucds {

VirtualModbusClient::VirtualModbusClient()
    : m_holdingBytes(256, char(0))
    , m_inputBytes(128, char(0))
{
    seedDefaults();
}

void VirtualModbusClient::setSlaveId(int slaveId)
{
    m_slaveId = qBound(1, slaveId, 247);
}

int VirtualModbusClient::slaveId() const
{
    return m_slaveId;
}

void VirtualModbusClient::writeHoldingRegister(quint16 address, quint16 value)
{
    m_holdingRegisters.insert(address, value);
    const int byteOffset = address * 2;
    if (byteOffset + 1 < m_holdingBytes.size()) {
        m_holdingBytes[byteOffset] = char((value >> 8) & 0xFF);
        m_holdingBytes[byteOffset + 1] = char(value & 0xFF);
    }

    if (address == 0)
        executeCommand(value);

    m_lastFrame = QStringLiteral("S%1 W 0x%2=%3")
                      .arg(m_slaveId)
                      .arg(address, 4, 16, QLatin1Char('0'))
                      .arg(value);
}

QVector<quint16> VirtualModbusClient::readHoldingRegisters(quint16 startAddress, int count) const
{
    const QVector<quint16> values = readRegistersFromBytes(m_holdingBytes, startAddress, count);
    m_lastFrame = QStringLiteral("S%1 R 0x%2[%3]")
                      .arg(m_slaveId)
                      .arg(startAddress, 4, 16, QLatin1Char('0'))
                      .arg(count);
    return values;
}

QVector<quint16> VirtualModbusClient::readInputRegisters(quint16 startAddress, int count) const
{
    const QVector<quint16> values = readRegistersFromBytes(m_inputBytes, startAddress, count);
    m_lastFrame = QStringLiteral("S%1 IR 0x%2[%3]")
                      .arg(m_slaveId)
                      .arg(startAddress, 4, 16, QLatin1Char('0'))
                      .arg(count);
    return values;
}

quint16 VirtualModbusClient::holdingRegister(quint16 address) const
{
    return readHoldingRegisters(address * 2, 1).value(0, 0);
}

quint8 VirtualModbusClient::holdingByte(quint16 byteOffset) const
{
    if (byteOffset >= m_holdingBytes.size())
        return 0;
    return quint8(m_holdingBytes.at(byteOffset));
}

quint8 VirtualModbusClient::inputByte(quint16 byteOffset) const
{
    if (byteOffset >= m_inputBytes.size())
        return 0;
    return quint8(m_inputBytes.at(byteOffset));
}

SensorModbusStatus VirtualModbusClient::statusByte(quint16 byteOffset) const
{
    return describeStatus(holdingByte(byteOffset));
}

SensorModbusSample VirtualModbusClient::sample() const
{
    SensorModbusSample result;
    result.pitch = readLeFloat(m_inputBytes, 0);
    result.roll = readLeFloat(m_inputBytes, 4);
    result.error = readLeFloat(m_inputBytes, 8);
    result.temperature = readLeFloat(m_inputBytes, 12);
    result.battery = readLeFloat(m_inputBytes, 64);
    return result;
}

bool VirtualModbusClient::transactFrame(const QByteArray &request, QByteArray *response)
{
    if (response)
        response->clear();
    if (request.size() < 4 || !hasValidCrc(request)) {
        m_lastFrame = modbusText("CRC 错误或帧长度不足");
        return false;
    }

    const quint8 slave = quint8(request.at(0));
    const quint8 function = quint8(request.at(1));
    if (slave != m_slaveId) {
        m_lastFrame = modbusText("忽略从站 %1 的帧").arg(slave);
        return false;
    }

    auto be16 = [&request](int offset) -> quint16 {
        return (quint16(quint8(request.at(offset))) << 8) | quint8(request.at(offset + 1));
    };

    QByteArray payload;
    switch (function) {
    case 0x03:
    case 0x04:
    {
        if (request.size() != 8)
            return false;
        const quint16 byteOffset = be16(2);
        const quint16 count = be16(4);
        if (function == 0x03 && byteOffset <= 2 && byteOffset + count * 2 > 2)
            advancePreciseMeasurement();
        const QByteArray data = readBytes(function == 0x03 ? m_holdingBytes : m_inputBytes, byteOffset, count);
        if (data.size() != count * 2)
            return false;

        payload.append(char(slave));
        payload.append(char(function));
        payload.append(char(data.size()));
        payload.append(data);
        if (response)
            *response = appendCrc(payload);
        m_lastFrame = QStringLiteral("S%1 F%2 R bytes 0x%3[%4]")
                          .arg(slave)
                          .arg(function, 2, 16, QLatin1Char('0'))
                          .arg(byteOffset, 4, 16, QLatin1Char('0'))
                          .arg(count);
        return true;
    }
    case 0x06:
    {
        if (request.size() != 8)
            return false;
        const quint16 address = be16(2);
        const quint16 value = be16(4);
        writeHoldingRegister(address, value);
        if (response)
            *response = request;
        return true;
    }
    case 0x10:
    {
        if (request.size() < 11)
            return false;
        const quint16 address = be16(2);
        const quint16 count = be16(4);
        const quint8 byteCount = quint8(request.at(6));
        if (byteCount != count * 2 || request.size() != byteCount + 9)
            return false;
        const int byteOffset = address * 2;
        if (byteOffset < 0 || byteOffset + byteCount > m_holdingBytes.size())
            return false;
        for (int i = 0; i < byteCount; ++i)
            m_holdingBytes[byteOffset + i] = request.at(7 + i);
        if (address == 0 && byteCount >= 2)
            executeCommand((quint16(quint8(request.at(7))) << 8) | quint8(request.at(8)));
        payload = request.left(6);
        if (response)
            *response = appendCrc(payload);
        return true;
    }
    default:
        payload.append(char(slave));
        payload.append(char(function | 0x80));
        payload.append(char(0x01));
        if (response)
            *response = appendCrc(payload);
        m_lastFrame = modbusText("不支持功能码 0x%1").arg(function, 2, 16, QLatin1Char('0'));
        return false;
    }
}

QString VirtualModbusClient::lastFrame() const
{
    return m_lastFrame;
}

quint16 VirtualModbusClient::crc(const QByteArray &payload)
{
    quint16 value = 0xFFFF;
    for (char ch : payload) {
        value ^= quint8(ch);
        for (int bit = 0; bit < 8; ++bit)
            value = (value & 0x0001) ? quint16((value >> 1) ^ 0xA001) : quint16(value >> 1);
    }
    return value;
}

bool VirtualModbusClient::hasValidCrc(const QByteArray &frame)
{
    if (frame.size() < 4)
        return false;
    const quint16 expected = crc(frame.left(frame.size() - 2));
    const quint8 firstCrcByte = quint8(frame.at(frame.size() - 2));
    const quint8 secondCrcByte = quint8(frame.at(frame.size() - 1));
    return (firstCrcByte == (expected >> 8) && secondCrcByte == (expected & 0xFF))
        || (firstCrcByte == (expected & 0xFF) && secondCrcByte == (expected >> 8));
}

QByteArray VirtualModbusClient::appendCrc(const QByteArray &payload)
{
    const quint16 value = crc(payload);
    QByteArray frame = payload;
    frame.append(char(value & 0xFF));
    frame.append(char(value >> 8));
    return frame;
}

QByteArray VirtualModbusClient::buildReadHoldingRegisters(int slaveId, quint16 byteOffset, quint16 count)
{
    QByteArray payload;
    payload.append(char(qBound(1, slaveId, 247)));
    payload.append(char(0x03));
    payload.append(char(byteOffset >> 8));
    payload.append(char(byteOffset & 0xFF));
    payload.append(char(count >> 8));
    payload.append(char(count & 0xFF));
    return appendCrc(payload);
}

QByteArray VirtualModbusClient::buildReadInputRegisters(int slaveId, quint16 byteOffset, quint16 count)
{
    QByteArray payload;
    payload.append(char(qBound(1, slaveId, 247)));
    payload.append(char(0x04));
    payload.append(char(byteOffset >> 8));
    payload.append(char(byteOffset & 0xFF));
    payload.append(char(count >> 8));
    payload.append(char(count & 0xFF));
    return appendCrc(payload);
}

QByteArray VirtualModbusClient::buildWriteSingleHoldingRegister(int slaveId, quint16 address, quint16 value)
{
    QByteArray payload;
    payload.append(char(qBound(1, slaveId, 247)));
    payload.append(char(0x06));
    payload.append(char(address >> 8));
    payload.append(char(address & 0xFF));
    payload.append(char(value >> 8));
    payload.append(char(value & 0xFF));
    return appendCrc(payload);
}

QByteArray VirtualModbusClient::buildWriteMultipleHoldingRegisters(int slaveId, quint16 startAddress, const QByteArray &bytes)
{
    if (bytes.isEmpty() || (bytes.size() % 2) != 0 || bytes.size() > 246)
        return {};

    const quint16 count = quint16(bytes.size() / 2);
    QByteArray payload;
    payload.append(char(qBound(1, slaveId, 247)));
    payload.append(char(0x10));
    payload.append(char(startAddress >> 8));
    payload.append(char(startAddress & 0xFF));
    payload.append(char(count >> 8));
    payload.append(char(count & 0xFF));
    payload.append(char(bytes.size()));
    payload.append(bytes);
    return appendCrc(payload);
}

QByteArray VirtualModbusClient::encodeMcuBaud(quint32 baud)
{
    QByteArray bytes;
    bytes.reserve(4);
    bytes.append(char(baud & 0xFF));
    bytes.append(char((baud >> 16) & 0xFF));
    bytes.append(char((baud >> 8) & 0xFF));
    bytes.append(char((baud >> 24) & 0xFF));
    return bytes;
}

quint32 VirtualModbusClient::decodeMcuBaud(const QByteArray &bytes, qsizetype offset)
{
    if (offset < 0 || offset + 4 > bytes.size())
        return 0;
    return quint32(quint8(bytes.at(offset)))
        | (quint32(quint8(bytes.at(offset + 1))) << 16)
        | (quint32(quint8(bytes.at(offset + 2))) << 8)
        | (quint32(quint8(bytes.at(offset + 3))) << 24);
}

quint16 VirtualModbusClient::loraModuleModeToUpper(quint16 moduleMode)
{
    switch (moduleMode) {
    case 2:
        return 0; // Fixed-address mode.
    case 4:
        return 1; // Master/slave mode.
    case 1:
        return 2; // Transparent mode.
    default:
        return 0x00FF;
    }
}

quint16 VirtualModbusClient::loraUpperModeToModule(quint16 upperMode)
{
    switch (upperMode) {
    case 0:
        return 2;
    case 1:
        return 4;
    case 2:
        return 1;
    default:
        return 0;
    }
}

QString VirtualModbusClient::formatHex(const QByteArray &frame)
{
    QStringList parts;
    parts.reserve(frame.size());
    for (char ch : frame)
        parts.append(QStringLiteral("%1").arg(quint8(ch), 2, 16, QLatin1Char('0')).toUpper());
    return parts.join(QLatin1Char(' '));
}

int VirtualModbusClient::expectedReadResponseBytes(quint16 count)
{
    return 3 + int(count) * 2 + 2;
}

SensorModbusStatus VirtualModbusClient::parseStatusResponse(const QByteArray &frame)
{
    if (frame.size() < expectedReadResponseBytes(1)
        || !hasValidCrc(frame)
        || (quint8(frame.at(1)) != 0x03 && quint8(frame.at(1)) != 0x04)
        || quint8(frame.at(2)) < 1) {
        return describeStatus(0);
    }
    return describeStatus(quint8(frame.at(3)));
}

SensorModbusSample VirtualModbusClient::parseMeasurementResponse(const QByteArray &frame)
{
    SensorModbusSample result;
    if (frame.size() < 21 || !hasValidCrc(frame) || quint8(frame.at(1)) != 0x04 || quint8(frame.at(2)) < 16)
        return result;

    const QByteArray data = frame.mid(3, quint8(frame.at(2)));
    result.pitch = readLeFloat(data, 0);
    result.roll = readLeFloat(data, 4);
    result.error = readLeFloat(data, 8);
    result.temperature = readLeFloat(data, 12);
    return result;
}

void VirtualModbusClient::seedDefaults()
{
    m_holdingBytes.fill(char(0));
    m_inputBytes.fill(char(0));
    m_holdingRegisters.clear();
    m_pendingPrecisePolls = 0;

    constexpr quint32 baud = 9600;
    const QByteArray mcuBaud = encodeMcuBaud(baud);
    for (qsizetype index = 0; index < mcuBaud.size(); ++index)
        m_holdingBytes[4 + index] = mcuBaud.at(index);
    m_holdingBytes[8] = char(0);
    m_holdingBytes[9] = char(1);  // Gap index.
    m_holdingBytes[10] = char(0); // Command mode.
    m_holdingBytes[12] = char(1); // Modbus id.
    m_holdingBytes[13] = char(1); // RS485 enabled.
    m_holdingBytes[20] = char((baud >> 24) & 0xFF);
    m_holdingBytes[21] = char((baud >> 16) & 0xFF);
    m_holdingBytes[22] = char((baud >> 8) & 0xFF);
    m_holdingBytes[23] = char(baud & 0xFF);

    putLeFloat(&m_inputBytes, 0, 0.0f);
    putLeFloat(&m_inputBytes, 4, 0.0f);
    putLeFloat(&m_inputBytes, 8, 0.0f);
    putLeFloat(&m_inputBytes, 12, 25.0f);
    putLeFloat(&m_inputBytes, 64, 100.0f);
}

void VirtualModbusClient::executeCommand(quint16 value)
{
    const quint8 command = value & 0xFF;
    switch (command) {
    case 0xB1:
    {
        m_pendingPrecisePolls = 2;
        m_holdingBytes[1] = char(0xFF);
        m_holdingBytes[2] = char(0xFF);
        break;
    }
    case 0xB2:
        m_holdingBytes[0] = char(0xA2);
        m_holdingBytes[1] = char(0xFF);
        break;
    case 0xB3:
        m_holdingBytes[0] = char(0xA3);
        m_holdingBytes[1] = char(0xFF);
        break;
    case 0xB4:
        m_holdingBytes[0] = char(0xA4);
        m_holdingBytes[1] = char(0xFF);
        break;
    case 0xB5:
        seedDefaults();
        m_holdingBytes[0] = char(0xA5);
        m_holdingBytes[1] = char(0xFF);
        break;
    case 0xB6:
        seedDefaults();
        m_holdingBytes[0] = char(0xA6);
        m_holdingBytes[1] = char(0xFF);
        break;
    case 0xB7:
        m_holdingBytes[0] = char(0xA7);
        m_holdingBytes[1] = char(0xFF);
        break;
    case 0xB8:
        m_holdingBytes[0] = char(0xA8);
        m_holdingBytes[1] = char(0xFF);
        break;
    case 0xB9:
        m_holdingBytes[0] = char(0xA9);
        m_holdingBytes[1] = char(0xFF);
        break;
    default:
        break;
    }
}

void VirtualModbusClient::advancePreciseMeasurement()
{
    if (m_pendingPrecisePolls <= 0)
        return;

    --m_pendingPrecisePolls;
    if (m_pendingPrecisePolls == 0)
        finishPreciseMeasurement();
}

void VirtualModbusClient::finishPreciseMeasurement()
{
    ++m_measurementCounter;
    const float pitch = 1.25f + float(m_measurementCounter % 7) * 0.05f;
    const float roll = -0.85f + float(m_measurementCounter % 5) * 0.03f;
    const float error = 0.001f * float((m_measurementCounter % 4) + 1);
    const float temperature = 25.0f + float(m_measurementCounter % 6) * 0.2f;
    putLeFloat(&m_inputBytes, 0, pitch);
    putLeFloat(&m_inputBytes, 4, roll);
    putLeFloat(&m_inputBytes, 8, error);
    putLeFloat(&m_inputBytes, 12, temperature);
    m_holdingBytes[2] = char(0xA1);
}

QVector<quint16> VirtualModbusClient::readRegistersFromBytes(const QByteArray &bytes, quint16 byteOffset, int count) const
{
    QVector<quint16> values;
    if (count <= 0)
        return values;

    values.reserve(count);
    for (int offset = 0; offset < count; ++offset) {
        const int index = byteOffset + offset * 2;
        if (index + 1 >= bytes.size()) {
            values.append(0);
        } else {
            values.append((quint16(quint8(bytes.at(index))) << 8) | quint8(bytes.at(index + 1)));
        }
    }
    return values;
}

QByteArray VirtualModbusClient::readBytes(const QByteArray &bytes, quint16 byteOffset, quint16 count) const
{
    const int byteCount = count * 2;
    if (count == 0 || byteOffset >= bytes.size() || byteOffset + byteCount > bytes.size())
        return {};
    return bytes.mid(byteOffset, byteCount);
}

void VirtualModbusClient::putLeFloat(QByteArray *bytes, int offset, float value)
{
    if (!bytes || offset < 0 || offset + int(sizeof(float)) > bytes->size())
        return;
    quint32 word = 0;
    std::memcpy(&word, &value, sizeof(word));
    (*bytes)[offset] = char(word & 0xFF);
    (*bytes)[offset + 1] = char((word >> 8) & 0xFF);
    (*bytes)[offset + 2] = char((word >> 16) & 0xFF);
    (*bytes)[offset + 3] = char((word >> 24) & 0xFF);
}

float VirtualModbusClient::readLeFloat(const QByteArray &bytes, int offset)
{
    if (offset < 0 || offset + int(sizeof(float)) > bytes.size())
        return 0.0f;
    const quint32 word = quint32(quint8(bytes.at(offset)))
        | (quint32(quint8(bytes.at(offset + 1))) << 8)
        | (quint32(quint8(bytes.at(offset + 2))) << 16)
        | (quint32(quint8(bytes.at(offset + 3))) << 24);
    float value = 0.0f;
    std::memcpy(&value, &word, sizeof(value));
    return value;
}

SensorModbusStatus VirtualModbusClient::describeStatus(quint8 code)
{
    SensorModbusStatus status;
    status.code = code;
    switch (code) {
    case 0xA1:
        status.description = modbusText("测量成功，可以读取输入寄存器");
        status.success = true;
        break;
    case 0xA2:
        status.description = modbusText("MCU 参数保存成功");
        status.success = true;
        break;
    case 0xA3:
        status.description = modbusText("LoRa 参数保存成功");
        status.success = true;
        break;
    case 0xA4:
        status.description = modbusText("复位/校准成功");
        status.success = true;
        break;
    case 0xA5:
        status.description = modbusText("恢复出厂设置成功");
        status.success = true;
        break;
    case 0xA6:
        status.description = modbusText("设备重启完成");
        status.success = true;
        break;
    case 0xA7:
        status.description = modbusText("B7 命令成功（设备搜索或电池读取）");
        status.success = true;
        break;
    case 0xA8:
        status.description = modbusText("B8 命令成功（连续采样启动或附加数据读取）");
        status.success = true;
        break;
    case 0xA9:
        status.description = modbusText("B9 命令成功（连续采样停止或 DTU 参数保存）");
        status.success = true;
        break;
    case 0xC1:
        status.description = modbusText("测量失败");
        break;
    case 0xC2:
        status.description = modbusText("MCU 参数保存失败");
        break;
    case 0xC3:
        status.description = modbusText("LoRa 参数保存失败");
        break;
    case 0xC4:
        status.description = modbusText("复位/校准失败");
        break;
    case 0xC5:
        status.description = modbusText("恢复出厂设置失败");
        break;
    case 0xC7:
        status.description = modbusText("B7 命令失败（电池读取失败）");
        break;
    case 0xC8:
        status.description = modbusText("连续采样正忙");
        break;
    case 0xD8:
        status.description = modbusText("连续采样启动失败");
        break;
    case 0xC9:
        status.description = modbusText("连续采样忙");
        break;
    case 0xD9:
        status.description = modbusText("连续采样失败");
        break;
    case 0xFF:
        status.description = modbusText("等待设备处理");
        break;
    default:
        status.description = modbusText("无有效状态码");
        break;
    }
    return status;
}

} // namespace gucds
