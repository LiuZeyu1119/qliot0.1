#include "gucds/core/devicecommunicationcontroller.h"

#include "gucds/core/atprotocol.h"
#include "gucds/core/serialsession.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMetaObject>
#include <QRegularExpression>
#include <QThread>

#include <algorithm>
#include <iterator>

namespace gucds {

namespace {

QString hex(const QByteArray &bytes)
{
    return VirtualModbusClient::formatHex(bytes);
}

bool validModbusResponse(const QByteArray &response, int slaveId, quint8 function, QString *error)
{
    if (response.size() < 5) {
        if (error)
            *error = QCoreApplication::translate("DeviceCommunicationController", "Modbus 回包长度不足：%1 字节").arg(response.size());
        return false;
    }
    if (!VirtualModbusClient::hasValidCrc(response)) {
        if (error)
            *error = QCoreApplication::translate("DeviceCommunicationController", "Modbus 回包 CRC 校验失败：%1").arg(hex(response));
        return false;
    }
    if (quint8(response.at(0)) != quint8(slaveId)) {
        if (error)
            *error = QCoreApplication::translate("DeviceCommunicationController", "Modbus 回包从站不匹配：期望 %1，收到 %2")
                         .arg(slaveId)
                         .arg(quint8(response.at(0)));
        return false;
    }
    const quint8 responseFunction = quint8(response.at(1));
    if (responseFunction == quint8(function | 0x80)) {
        if (error)
            *error = QCoreApplication::translate("DeviceCommunicationController", "Modbus 异常响应：功能码 0x%1，异常码 0x%2")
                         .arg(function, 2, 16, QLatin1Char('0'))
                         .arg(quint8(response.at(2)), 2, 16, QLatin1Char('0'))
                         .toUpper();
        return false;
    }
    if (responseFunction != function) {
        if (error)
            *error = QCoreApplication::translate("DeviceCommunicationController", "Modbus 回包功能码不匹配：期望 0x%1，收到 0x%2")
                         .arg(function, 2, 16, QLatin1Char('0'))
                         .arg(responseFunction, 2, 16, QLatin1Char('0'))
                         .toUpper();
        return false;
    }
    return true;
}

QVector<double> parseNumbers(const QString &text)
{
    static const QRegularExpression numberPattern(
        QStringLiteral(R"([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)"));
    QVector<double> values;
    auto match = numberPattern.globalMatch(text);
    while (match.hasNext()) {
        bool ok = false;
        const double value = match.next().captured().toDouble(&ok);
        if (ok)
            values.append(value);
    }
    return values;
}

bool parseFrequencySamplingIndexes(const QVector<double> &values,
                                   int *sampleRateIndex,
                                   int *samplePointIndex)
{
    if (!sampleRateIndex || !samplePointIndex)
        return false;

    // Frequency/tension firmware returns only the two sampling indexes.  Keep
    // accepting the longer sensor-parameter form used by older products.
    if (values.size() == 2) {
        *sampleRateIndex = qRound(values.at(0));
        *samplePointIndex = qRound(values.at(1));
        return true;
    }
    if (values.size() >= 4) {
        *sampleRateIndex = qRound(values.at(2));
        *samplePointIndex = qRound(values.at(3));
        return true;
    }
    return false;
}

QVector<double> normalizedFrequencySensorParameters(const QVector<double> &values)
{
    if (values.size() != 2)
        return values;

    // Preserve the existing combined MCU/sensor layout: sampling indexes are
    // exposed at positions 7 and 8 after the five MCU values.
    return {0.0, 0.0, values.at(0), values.at(1)};
}

CommunicationResult textCommand(SerialSession &session,
                                const QString &request,
                                const QString &context,
                                int repeatCount)
{
    CommunicationResult result;
    result.context = context;
    result.request = request;
    QByteArray combined;
    for (int index = 0; index < repeatCount; ++index) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            result.message = QCoreApplication::translate("DeviceCommunicationController", "通信操作已取消");
            return result;
        }
        QByteArray response;
        QString error;
        if (!session.transactText(request, &response, &error)) {
            result.message = error;
            return result;
        }
        if (!combined.isEmpty())
            combined.append('\n');
        combined.append(response);
    }

    result.responseBytes = combined;
    result.responseText = QString::fromUtf8(combined).trimmed();
    result.numericValues = parseNumbers(result.responseText);
    const ParsedStatus status = AtProtocol::parseStatus(result.responseText);
    result.success = status.code == QStringLiteral("UNKNOWN") ? !combined.isEmpty() : status.success;
    result.message = status.code == QStringLiteral("UNKNOWN")
        ? QCoreApplication::translate("DeviceCommunicationController", "收到 %1 字节回包").arg(combined.size())
        : status.message;
    return result;
}

bool transactModbus(SerialSession &session,
                     const QByteArray &request,
                     int responseSize,
                     int slaveId,
                     quint8 function,
                     QByteArray *response,
                     QString *error,
                     int timeoutMs = 1500)
{
    Q_UNUSED(responseSize)
    if (!response) {
        if (error)
            *error = QCoreApplication::translate("DeviceCommunicationController", "Modbus 响应缓冲区不能为空");
        return false;
    }

    response->clear();
    if (!session.writeFrame(request, error))
        return false;

    QByteArray received;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs) {
        QByteArray chunk;
        QString readError;
        const int remainingMs = timeoutMs - int(timer.elapsed());
        const int idleTimeoutMs = qMin(80, qMax(20, remainingMs));
        if (!session.readUntilIdle(&chunk, &readError, remainingMs, idleTimeoutMs)) {
            if (error)
                *error = readError;
            return false;
        }

        received.append(chunk);
        for (int offset = 0; offset + 5 <= received.size(); ++offset) {
            const quint8 responseFunction = quint8(received.at(offset + 1));
            int frameLength = 0;
            if (responseFunction == 0x03 || responseFunction == 0x04) {
                frameLength = 5 + quint8(received.at(offset + 2));
            } else if (responseFunction == 0x06 || responseFunction == 0x10) {
                frameLength = 8;
            } else if (responseFunction == quint8(function | 0x80)) {
                frameLength = 5;
            } else {
                continue;
            }
            if (offset + frameLength > received.size())
                continue;

            const QByteArray candidate = received.mid(offset, frameLength);
            if (quint8(candidate.at(0)) != quint8(slaveId)
                || !VirtualModbusClient::hasValidCrc(candidate)) {
                continue;
            }
            if (quint8(candidate.at(1)) != function
                && quint8(candidate.at(1)) != quint8(function | 0x80)) {
                continue;
            }

            *response = candidate;
            return validModbusResponse(*response, slaveId, function, error);
        }
    }

    if (error) {
        *error = QCoreApplication::translate("DeviceCommunicationController", "等待 Modbus 功能码 0x%1 回包超时；串口数据：%2")
                     .arg(function, 2, 16, QLatin1Char('0'))
                     .arg(hex(received))
                     .toUpper();
    }
    return false;
}

} // namespace

DeviceCommunicationController::DeviceCommunicationController(QObject *parent)
    : QObject(parent)
    , m_serialSession(std::make_unique<SerialSession>())
{
    qRegisterMetaType<CommunicationResult>();
}

DeviceCommunicationController::~DeviceCommunicationController()
{
    if (!m_thread)
        return;
    m_thread->requestInterruption();
    m_thread->wait();
    delete m_thread;
    m_thread = nullptr;
}

void DeviceCommunicationController::configure(const QString &portName,
                                               int baudRate,
                                               int slaveId,
                                               DeviceWireProtocol protocol)
{
    if (m_thread)
        return;
    m_portName = portName.trimmed();
    m_baudRate = baudRate;
    m_slaveId = (std::clamp)(slaveId, 1, 247);
    m_protocol = protocol;
}

bool DeviceCommunicationController::sendTextCommand(const QString &request,
                                                    const QString &context,
                                                    int repeatCount)
{
    const QString trimmedRequest = request.trimmed();
    if (trimmedRequest.isEmpty() || repeatCount < 1 || repeatCount > 1000)
        return false;
    return startOperation(context, [trimmedRequest, context, repeatCount](SerialSession &session) {
        return textCommand(session, trimmedRequest, context, repeatCount);
    });
}

bool DeviceCommunicationController::sendTextCommands(const QStringList &requests,
                                                      const QString &context,
                                                      int intervalMs)
{
    QStringList commands;
    for (const QString &request : requests) {
        if (!request.trimmed().isEmpty())
            commands.append(request.trimmed());
    }
    if (commands.isEmpty() || intervalMs < 0 || intervalMs > 60000)
        return false;
    return startOperation(context, [commands, context, intervalMs](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        QStringList sent;
        QStringList received;
        for (qsizetype index = 0; index < commands.size(); ++index) {
            const QString &command = commands.at(index);
            CommunicationResult item = textCommand(session, command, context, 1);
            sent.append(command);
            received.append(item.responseText);
            result.responseBytes += item.responseBytes;
            if (!item.success) {
                result.request = sent.join(QLatin1Char('\n'));
                result.responseText = received.join(QLatin1Char('\n'));
                result.message = QCoreApplication::translate("DeviceCommunicationController", "命令 %1 执行失败：%2").arg(command, item.message);
                return result;
            }
            if (index + 1 < commands.size() && intervalMs > 0) {
                int waitedMs = 0;
                while (waitedMs < intervalMs) {
                    if (QThread::currentThread()->isInterruptionRequested()) {
                        result.request = sent.join(QLatin1Char('\n'));
                        result.responseText = received.join(QLatin1Char('\n'));
                        result.message = QCoreApplication::translate("DeviceCommunicationController", "通信操作已取消");
                        return result;
                    }
                    const int sleepMs = (std::min)(100, intervalMs - waitedMs);
                    QThread::msleep(static_cast<unsigned long>(sleepMs));
                    waitedMs += sleepMs;
                }
            }
        }
        result.request = sent.join(QLatin1Char('\n'));
        result.responseText = received.join(QLatin1Char('\n'));
        result.numericValues = parseNumbers(result.responseText);
        result.success = true;
        result.message = QCoreApplication::translate("DeviceCommunicationController", "%1 条配置命令执行成功").arg(commands.size());
        return result;
    });
}

bool DeviceCommunicationController::sendModbusCommand(SensorModbusCommand command, const QString &context)
{
    const int slave = m_slaveId;
    return startOperation(context, [slave, command, context](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        const QByteArray request = VirtualModbusClient::buildWriteSingleHoldingRegister(
            slave,
            0x0000,
            static_cast<quint16>(command));
        result.request = hex(request);
        QString error;
        if (!transactModbus(session, request, request.size(), slave, 0x06, &result.responseBytes, &error)) {
            result.message = error;
            return result;
        }
        if (result.responseBytes != request) {
            result.message = QCoreApplication::translate("DeviceCommunicationController", "Modbus 写命令回显与请求不一致");
            return result;
        }
        result.responseText = hex(result.responseBytes);
        result.success = true;
        result.message = QCoreApplication::translate("DeviceCommunicationController", "设备命令 0x%1 执行成功")
                             .arg(static_cast<quint16>(command), 4, 16, QLatin1Char('0'))
                             .toUpper();
        return result;
    });
}

bool DeviceCommunicationController::writeModbusRegisters(quint16 startAddress,
                                                         const QByteArray &bytes,
                                                         const QString &context,
                                                         SensorModbusCommand saveCommand)
{
    if (bytes.isEmpty() || bytes.size() % 2 != 0)
        return false;
    const int slave = m_slaveId;
    return startOperation(context, [slave, startAddress, bytes, context, saveCommand](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        const QByteArray request = VirtualModbusClient::buildWriteMultipleHoldingRegisters(slave, startAddress, bytes);
        result.request = hex(request);
        QString error;
        if (!transactModbus(session, request, 8, slave, 0x10, &result.responseBytes, &error)) {
            result.message = error;
            return result;
        }
        result.responseText = hex(result.responseBytes);

        if (static_cast<quint16>(saveCommand) != 0) {
            const QByteArray saveRequest = VirtualModbusClient::buildWriteSingleHoldingRegister(
                slave,
                0x0000,
                static_cast<quint16>(saveCommand));
            QByteArray saveResponse;
            if (!transactModbus(session, saveRequest, 8, slave, 0x06, &saveResponse, &error)) {
                result.message = QCoreApplication::translate("DeviceCommunicationController", "参数已写入 RAM，但保存命令失败：%1").arg(error);
                return result;
            }
            result.request += QStringLiteral("\n") + hex(saveRequest);
            result.responseText += QStringLiteral("\n") + hex(saveResponse);
            result.responseBytes += saveResponse;
        }
        result.success = true;
        result.message = QCoreApplication::translate("DeviceCommunicationController", "参数写入并校验成功");
        return result;
    });
}

bool DeviceCommunicationController::writeFrequencyMcuParameters(const QByteArray &mcuBytes,
                                                                 quint8 sampleRateIndex,
                                                                 quint8 samplePointIndex,
                                                                 const QString &context)
{
    constexpr quint8 maximumSampleRateIndex = 10;
    constexpr quint8 maximumSamplePointIndex = 3;
    if (mcuBytes.size() != 10
        || sampleRateIndex > maximumSampleRateIndex
        || samplePointIndex > maximumSamplePointIndex) {
        return false;
    }

    QByteArray samplingBytes(8, char(0));
    samplingBytes[3] = char(sampleRateIndex);
    samplingBytes[7] = char(samplePointIndex);

    const int slave = m_slaveId;
    return startOperation(context,
                          [slave,
                           mcuBytes,
                           samplingBytes,
                           sampleRateIndex,
                           samplePointIndex,
                           context](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        QStringList requests;
        QStringList responses;
        QString error;

        const auto writeBlock = [&](quint16 startAddress, const QByteArray &bytes) {
            const QByteArray request = VirtualModbusClient::buildWriteMultipleHoldingRegisters(
                slave,
                startAddress,
                bytes);
            QByteArray response;
            if (!transactModbus(session, request, 8, slave, 0x10, &response, &error))
                return false;

            const QByteArray expectedResponse = VirtualModbusClient::appendCrc(request.left(6));
            if (response != expectedResponse) {
                error = QCoreApplication::translate(
                    "DeviceCommunicationController",
                    "Modbus 参数写入回包与请求地址或数量不一致");
                return false;
            }
            requests.append(hex(request));
            responses.append(hex(response));
            result.responseBytes += response;
            return true;
        };

        if (!writeBlock(0x0002, mcuBytes)) {
            result.message = error;
            return result;
        }
        if (!writeBlock(0x0012, samplingBytes)) {
            result.request = requests.join(QLatin1Char('\n'));
            result.responseText = responses.join(QLatin1Char('\n'));
            result.message = error;
            return result;
        }

        const QByteArray saveRequest = VirtualModbusClient::buildWriteSingleHoldingRegister(
            slave,
            0x0000,
            static_cast<quint16>(SensorModbusCommand::SaveMcuParameters));
        QByteArray saveResponse;
        if (!transactModbus(session, saveRequest, 8, slave, 0x06, &saveResponse, &error)) {
            result.request = requests.join(QLatin1Char('\n'));
            result.responseText = responses.join(QLatin1Char('\n'));
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "MCU 与采样参数已写入 RAM，但保存命令失败：%1")
                                 .arg(error);
            return result;
        }
        if (saveResponse != saveRequest) {
            result.request = requests.join(QLatin1Char('\n'));
            result.responseText = responses.join(QLatin1Char('\n'));
            result.message = QCoreApplication::translate(
                "DeviceCommunicationController",
                "MCU 参数保存命令回显不一致");
            return result;
        }
        requests.append(hex(saveRequest));
        responses.append(hex(saveResponse));
        result.responseBytes += saveResponse;

        // The firmware commits both blocks on B2. Read both AT views back in the
        // same serial session so callers can verify the values actually persisted.
        QThread::msleep(200);
        const QString mcuRequest = AtProtocol::command(AtCommand::GetMcuParameters);
        CommunicationResult mcuReadback = textCommand(session, mcuRequest, context, 1);
        requests.append(mcuRequest);
        responses.append(mcuReadback.responseText);
        result.responseBytes += mcuReadback.responseBytes;
        if (!mcuReadback.success) {
            result.request = requests.join(QLatin1Char('\n'));
            result.responseText = responses.join(QLatin1Char('\n'));
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "参数已保存，但 MCU 参数回读失败：%1")
                                 .arg(mcuReadback.message);
            return result;
        }

        const QString sensorRequest = AtProtocol::command(AtCommand::GetSensorParameters);
        CommunicationResult sensorReadback = textCommand(session, sensorRequest, context, 1);
        requests.append(sensorRequest);
        responses.append(sensorReadback.responseText);
        result.responseBytes += sensorReadback.responseBytes;
        result.request = requests.join(QLatin1Char('\n'));
        result.responseText = responses.join(QLatin1Char('\n'));
        if (!sensorReadback.success) {
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "参数已保存，但采样参数回读失败：%1")
                                 .arg(sensorReadback.message);
            return result;
        }

        int actualRateIndex = -1;
        int actualPointIndex = -1;
        if (mcuReadback.numericValues.size() < 5
            || !parseFrequencySamplingIndexes(sensorReadback.numericValues,
                                              &actualRateIndex,
                                              &actualPointIndex)) {
            result.message = QCoreApplication::translate(
                "DeviceCommunicationController",
                "参数已保存，但 MCU 或采样参数回读不完整");
            return result;
        }
        result.numericValues = mcuReadback.numericValues;
        result.numericValues += normalizedFrequencySensorParameters(sensorReadback.numericValues);

        const quint32 expectedBaud = VirtualModbusClient::decodeMcuBaud(mcuBytes);
        const quint16 expectedGap = (quint16(quint8(mcuBytes.at(4))) << 8)
            | quint8(mcuBytes.at(5));
        const int expectedMcuValues[] = {
            int(expectedBaud),
            int(expectedGap),
            int(quint8(mcuBytes.at(6))),
            int(quint8(mcuBytes.at(8))),
            int(quint8(mcuBytes.at(9))),
        };
        for (int index = 0; index < int(std::size(expectedMcuValues)); ++index) {
            const int actual = qRound(mcuReadback.numericValues.at(index));
            if (actual != expectedMcuValues[index]) {
                result.message = QCoreApplication::translate(
                                     "DeviceCommunicationController",
                                     "MCU 参数回读不一致：第 %1 项期望 %2，回读 %3")
                                     .arg(index + 1)
                                     .arg(expectedMcuValues[index])
                                     .arg(actual);
                return result;
            }
        }
        if (actualRateIndex != int(sampleRateIndex)
            || actualPointIndex != int(samplePointIndex)) {
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "采样参数回读不一致：期望频率索引 %1 / 点数索引 %2，回读 %3 / %4")
                                 .arg(sampleRateIndex)
                                 .arg(samplePointIndex)
                                 .arg(actualRateIndex)
                                 .arg(actualPointIndex);
            return result;
        }
        result.success = true;
        result.message = QCoreApplication::translate(
            "DeviceCommunicationController",
            "MCU 与采样参数写入、保存并回读成功");
        return result;
    });
}

bool DeviceCommunicationController::readModbusRegisters(quint16 byteOffset,
                                                        quint16 registerCount,
                                                        const QString &context)
{
    if (registerCount == 0 || registerCount > 125)
        return false;
    const int slave = m_slaveId;
    return startOperation(context, [slave, byteOffset, registerCount, context](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        const QByteArray request = VirtualModbusClient::buildReadHoldingRegisters(slave, byteOffset, registerCount);
        result.request = hex(request);
        QString error;
        if (!transactModbus(session,
                            request,
                            VirtualModbusClient::expectedReadResponseBytes(registerCount),
                            slave,
                            0x03,
                            &result.responseBytes,
                            &error)) {
            result.message = error;
            return result;
        }
        const int byteCount = quint8(result.responseBytes.at(2));
        if (byteCount != registerCount * 2) {
            result.message = QCoreApplication::translate("DeviceCommunicationController", "Modbus 数据长度不匹配：期望 %1，收到 %2")
                                 .arg(registerCount * 2)
                                 .arg(byteCount);
            return result;
        }
        result.dataBytes = result.responseBytes.mid(3, byteCount);
        result.responseText = hex(result.responseBytes);
        result.success = true;
        result.message = QCoreApplication::translate("DeviceCommunicationController", "读取到 %1 字节参数").arg(byteCount);
        return result;
    });
}

bool DeviceCommunicationController::scanDevices(const QString &context)
{
    const DeviceWireProtocol wireProtocol = m_protocol;
    const int selectedSlave = m_slaveId;
    if (wireProtocol == DeviceWireProtocol::Text) {
        return startOperation(context, [context](SerialSession &session) {
            CommunicationResult result = textCommand(
                session,
                AtProtocol::command(AtCommand::GetSensorParameters),
                context,
                1);
            if (result.success) {
                DeviceRecord device;
                device.name = QCoreApplication::translate("DeviceCommunicationController", "在线 AT 设备");
                device.category = QCoreApplication::translate("DeviceCommunicationController", "串口设备");
                device.model = result.responseText.left(120);
                device.protocol = QStringLiteral("TTL/AT");
                result.devices.append(device);
                result.message = QCoreApplication::translate("DeviceCommunicationController", "发现 1 个 AT 设备");
            }
            return result;
        });
    }

    return startOperation(context, [selectedSlave, context](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        QStringList traces;
        const int firstSlave = 1;
        const int lastSlave = 247;
        for (int slave = firstSlave; slave <= lastSlave; ++slave) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                result.message = QCoreApplication::translate("DeviceCommunicationController", "设备搜索已取消，已发现 %1 个设备").arg(result.devices.size());
                return result;
            }
            const QByteArray request = VirtualModbusClient::buildReadHoldingRegisters(slave, 0x0000, 0x0001);
            QByteArray response;
            QString ignoredError;
            if (!transactModbus(session, request, 7, slave, 0x03, &response, &ignoredError, 70))
                continue;

            DeviceRecord device;
            device.name = QCoreApplication::translate("DeviceCommunicationController", "Modbus 从站 %1").arg(slave);
            device.category = QCoreApplication::translate("DeviceCommunicationController", "在线设备");
            device.model = QStringLiteral("RTU-%1").arg(slave, 3, 10, QLatin1Char('0'));
            device.protocol = QStringLiteral("Modbus RTU");
            device.modbus = QStringLiteral("开");
            device.parameterValue1 = QString::number(slave);
            result.devices.append(device);
            traces.append(QStringLiteral("S%1 RX %2").arg(slave).arg(hex(response)));
        }
        result.request = QCoreApplication::translate("DeviceCommunicationController", "Modbus 扫描 1..247（当前从站 %1）").arg(selectedSlave);
        result.responseText = traces.join(QLatin1Char('\n'));
        result.success = true;
        result.message = QCoreApplication::translate("DeviceCommunicationController", "设备搜索完成：发现 %1 个 Modbus 从站").arg(result.devices.size());
        return result;
    });
}

bool DeviceCommunicationController::measure(const QString &context)
{
    const DeviceWireProtocol wireProtocol = m_protocol;
    const int slave = m_slaveId;
    if (wireProtocol == DeviceWireProtocol::Text) {
        return startOperation(context, [context](SerialSession &session) {
            CommunicationResult result = textCommand(
                session,
                AtProtocol::command(AtCommand::MeasureStart),
                context,
                1);
            if (!result.success)
                return result;
            CommunicationResult data = textCommand(
                session,
                AtProtocol::command(AtCommand::GetStringData),
                context,
                1);
            result.responseBytes += data.responseBytes;
            result.responseText += QStringLiteral("\n") + data.responseText;
            result.numericValues = data.numericValues;
            bool finished = false;
            const QVector<SampleStreamPoint> points = AtProtocol::parseSampleStream(data.responseText, &finished);
            if (!points.isEmpty()) {
                const SampleStreamPoint &point = points.last();
                result.sample.pitch = point.pitch;
                result.sample.roll = point.roll;
                result.hasSample = true;
            }
            result.success = data.success;
            result.message = data.success
                ? QCoreApplication::translate("DeviceCommunicationController", "AT 测量完成")
                : QCoreApplication::translate("DeviceCommunicationController", "测量已启动，但读取数据失败：%1").arg(data.message);
            return result;
        });
    }

    return startOperation(context, [slave, context](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        QStringList trace;
        QString error;

        const QByteArray startRequest = VirtualModbusClient::buildWriteSingleHoldingRegister(
            slave,
            0x0000,
            static_cast<quint16>(SensorModbusCommand::MeasureStart));
        QByteArray response;
        if (!transactModbus(session, startRequest, 8, slave, 0x06, &response, &error)) {
            result.message = error;
            return result;
        }
        trace.append(QStringLiteral("B1 TX: %1\nB1 RX: %2").arg(hex(startRequest), hex(response)));

        QByteArray activeFrame;
        QString activeError;
        if (session.readUntilIdle(&activeFrame, &activeError, 240, 30)
            && activeFrame.size() >= 21) {
            for (int offset = 0; offset + 21 <= activeFrame.size(); ++offset) {
                const QByteArray candidate = activeFrame.mid(offset, 21);
                if (quint8(candidate.at(0)) != quint8(slave)
                    || quint8(candidate.at(1)) != 0x04
                    || quint8(candidate.at(2)) != 0x10
                    || !VirtualModbusClient::hasValidCrc(candidate)) {
                    continue;
                }
                trace.append(QStringLiteral("ACTIVE RX: %1").arg(hex(candidate)));
                result.request = hex(startRequest);
                result.responseBytes = candidate;
                result.responseText = trace.join(QLatin1Char('\n'));
                result.sample = VirtualModbusClient::parseMeasurementResponse(candidate);
                result.hasSample = true;
                result.success = true;
                result.message = QCoreApplication::translate("DeviceCommunicationController", "收到主动测量帧");
                return result;
            }
            trace.append(QStringLiteral("UNSOLICITED RX: %1").arg(hex(activeFrame)));
        }

        const QByteArray statusRequest = VirtualModbusClient::buildReadHoldingRegisters(slave, 0x0002, 0x0001);
        constexpr int measurementTimeoutMs = 20000;
        constexpr int statusPollIntervalMs = 100;
        SensorModbusStatus status;
        QElapsedTimer measurementTimer;
        measurementTimer.start();
        while (measurementTimer.elapsed() < measurementTimeoutMs) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                result.message = QCoreApplication::translate("DeviceCommunicationController", "测量已取消");
                return result;
            }
            QThread::msleep(statusPollIntervalMs);
            if (!transactModbus(session, statusRequest, 7, slave, 0x03, &response, &error)) {
                result.message = error;
                return result;
            }
            trace.append(QStringLiteral("STATUS RX: %1").arg(hex(response)));
            status = VirtualModbusClient::parseStatusResponse(response);
            if (status.success)
                break;
            if (status.code != 0xFF) {
                result.message = QCoreApplication::translate("DeviceCommunicationController", "测量失败：0x%1 %2")
                                     .arg(status.code, 2, 16, QLatin1Char('0'))
                                     .arg(status.description)
                                     .toUpper();
                return result;
            }
        }
        if (!status.success) {
            result.message = QCoreApplication::translate("DeviceCommunicationController", "测量超时：等待 %1 秒后状态仍为 0x%2 %3")
                                 .arg(measurementTimeoutMs / 1000)
                                 .arg(status.code, 2, 16, QLatin1Char('0'))
                                 .arg(status.description)
                                 .toUpper();
            return result;
        }

        const QByteArray dataRequest = VirtualModbusClient::buildReadInputRegisters(slave, 0x0000, 0x0008);
        if (!transactModbus(session, dataRequest, 21, slave, 0x04, &response, &error)) {
            result.message = error;
            return result;
        }
        trace.append(QStringLiteral("DATA TX: %1\nDATA RX: %2").arg(hex(dataRequest), hex(response)));
        result.request = hex(startRequest);
        result.responseBytes = response;
        result.responseText = trace.join(QLatin1Char('\n'));
        result.sample = VirtualModbusClient::parseMeasurementResponse(response);
        result.hasSample = true;
        result.success = true;
        result.message = QCoreApplication::translate("DeviceCommunicationController", "精确测量完成");
        return result;
    });
}

bool DeviceCommunicationController::measureFrequencyTension(const QString &context)
{
    if (m_protocol == DeviceWireProtocol::Text)
        return measure(context);

    const int slave = m_slaveId;
    return startOperation(context, [this, slave, context](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        QStringList requests;
        QStringList trace;
        QString error;

        const auto updateTrace = [&] {
            result.request = requests.join(QLatin1Char('\n'));
            result.responseText = trace.join(QLatin1Char('\n'));
        };

        const QString mcuRequest = AtProtocol::command(AtCommand::GetMcuParameters);
        requests.append(mcuRequest);
        CommunicationResult mcuParameters = textCommand(session, mcuRequest, context, 1);
        trace.append(QStringLiteral("MCU RX: %1").arg(mcuParameters.responseText));
        if (!mcuParameters.success || mcuParameters.numericValues.size() < 5) {
            updateTrace();
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "读取 MCU 运行模式失败：%1")
                                 .arg(mcuParameters.message);
            return result;
        }

        const QString sensorRequest = AtProtocol::command(AtCommand::GetSensorParameters);
        requests.append(sensorRequest);
        CommunicationResult sensorParameters = textCommand(session, sensorRequest, context, 1);
        trace.append(QStringLiteral("SENSOR RX: %1").arg(sensorParameters.responseText));
        int sampleRateIndex = -1;
        int samplePointIndex = -1;
        if (!sensorParameters.success
            || !parseFrequencySamplingIndexes(sensorParameters.numericValues,
                                              &sampleRateIndex,
                                              &samplePointIndex)) {
            updateTrace();
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "读取频振索力采样配置失败：%1")
                                 .arg(sensorParameters.message);
            return result;
        }

        result.numericValues = mcuParameters.numericValues;
        result.numericValues += normalizedFrequencySensorParameters(sensorParameters.numericValues);
        const int mode = static_cast<int>(mcuParameters.numericValues.at(2));
        static constexpr int samplingRates[] = {
            5, 10, 20, 50, 100, 200, 400, 500, 800, 1000, 2000,
        };
        static constexpr int samplingPoints[] = {256, 512, 1024, 2048};
        if (sampleRateIndex < 0
            || sampleRateIndex >= int(std::size(samplingRates))
            || samplePointIndex < 0
            || samplePointIndex >= int(std::size(samplingPoints))) {
            updateTrace();
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "设备返回的采样配置索引无效：频率 %1，点数 %2")
                                 .arg(sampleRateIndex)
                                 .arg(samplePointIndex);
            return result;
        }
        if (mode < 0 || mode > 2) {
            updateTrace();
            result.message = QCoreApplication::translate(
                                 "DeviceCommunicationController",
                                 "当前 MCU 运行模式 %1 不支持频振索力测量")
                                 .arg(mode);
            return result;
        }

        QByteArray response;
        if (mode == 0) {
            const QByteArray startRequest = VirtualModbusClient::buildWriteSingleHoldingRegister(
                slave,
                0x0000,
                static_cast<quint16>(SensorModbusCommand::MeasureStart));
            requests.append(hex(startRequest));
            if (!transactModbus(session, startRequest, 8, slave, 0x06, &response, &error)) {
                updateTrace();
                result.message = error;
                return result;
            }
            if (response != startRequest) {
                trace.append(QStringLiteral("B1 RX: %1").arg(hex(response)));
                updateTrace();
                result.message = QCoreApplication::translate(
                    "DeviceCommunicationController",
                    "频振索力测量启动命令回显不一致");
                return result;
            }
            trace.append(QStringLiteral("B1 RX: %1").arg(hex(response)));
        } else if (mode == 1) {
            trace.append(QStringLiteral("MODE: AUTO (CONTINUOUS DIRECT DATA READ)"));
        } else {
            trace.append(QStringLiteral("MODE: LOW POWER (SCHEDULED DIRECT DATA READ)"));
        }

        const int sampleRate = samplingRates[sampleRateIndex];
        const int samplePoints = samplingPoints[samplePointIndex];
        const qint64 samplingDurationMs =
            (qint64(samplePoints) * 1000 + sampleRate - 1) / sampleRate;
        const int measurementTimeoutMs = int((std::clamp)(
            samplingDurationMs + (std::max)(qint64(5000), samplingDurationMs / 4),
            qint64(10000),
            qint64(600000)));
        trace.append(mode == 0
                         ? QStringLiteral("SAMPLING: %1 Hz, %2 points, timeout %3 ms")
                               .arg(sampleRate)
                               .arg(samplePoints)
                               .arg(measurementTimeoutMs)
                         : QStringLiteral("SAMPLING: %1 Hz, %2 points")
                               .arg(sampleRate)
                               .arg(samplePoints));

        const QByteArray statusRequest = VirtualModbusClient::buildReadHoldingRegisters(
            slave,
            0x0002,
            0x0001);
        constexpr int statusPollIntervalMs = 100;

        if (mode == 0) {
            requests.append(hex(statusRequest));
            SensorModbusStatus status;
            QElapsedTimer measurementTimer;
            measurementTimer.start();
            while (measurementTimer.elapsed() < measurementTimeoutMs) {
                if (QThread::currentThread()->isInterruptionRequested()) {
                    updateTrace();
                    result.message = QCoreApplication::translate(
                        "DeviceCommunicationController",
                        "频振索力测量已取消");
                    return result;
                }
                if (!transactModbus(session, statusRequest, 7, slave, 0x03, &response, &error)) {
                    updateTrace();
                    result.message = error;
                    return result;
                }
                trace.append(QStringLiteral("STATUS RX: %1").arg(hex(response)));
                status = VirtualModbusClient::parseStatusResponse(response);
                if (status.code == 0xA1)
                    break;
                if (status.code != 0xFF) {
                    updateTrace();
                    result.message = QCoreApplication::translate(
                                         "DeviceCommunicationController",
                                         "频振索力测量失败：0x%1 %2")
                                         .arg(status.code, 2, 16, QLatin1Char('0'))
                                         .arg(status.description)
                                         .toUpper();
                    return result;
                }
                QThread::msleep(statusPollIntervalMs);
            }
            if (status.code != 0xA1) {
                updateTrace();
                result.message = QCoreApplication::translate(
                                     "DeviceCommunicationController",
                                     "频振索力测量超时：按 %1 Hz / %2 点等待 %3 秒后状态仍为 0x%4 %5")
                                     .arg(sampleRate)
                                     .arg(samplePoints)
                                     .arg(measurementTimeoutMs / 1000)
                                     .arg(status.code, 2, 16, QLatin1Char('0'))
                                     .arg(status.description)
                                     .toUpper();
                return result;
            }
            const QByteArray dataRequest = VirtualModbusClient::buildReadInputRegisters(
                slave,
                0x0000,
                0x0008);
            requests.append(hex(dataRequest));
            if (!transactModbus(session, dataRequest, 21, slave, 0x04, &response, &error)) {
                updateTrace();
                result.message = error;
                return result;
            }
            trace.append(QStringLiteral("DATA RX: %1").arg(hex(response)));
            result.responseBytes = response;
            result.sample = VirtualModbusClient::parseMeasurementResponse(response);
            result.hasSample = true;
            result.success = true;
            result.message = QCoreApplication::translate(
                "DeviceCommunicationController",
                "指令模式频振索力测量完成");
            updateTrace();
            return result;
        }

        // Automatic mode is a long-lived acquisition session.  The sensor
        // triggers measurements itself, so the host periodically reads FC04
        // without sending B1.  Pace the reads by at least one complete sample
        // window instead of flooding the bus with duplicate 04 requests.
        const QByteArray dataRequest = VirtualModbusClient::buildReadInputRegisters(
            slave,
            0x0000,
            0x0008);
        int consecutiveCommunicationErrors = 0;
        constexpr int maximumCommunicationErrors = 3;
        static constexpr int gapSeconds[] = {
            0, 5, 30, 60, 1800, 3600, 7200, 43200, 86400,
        };
        const int gapIndex = qRound(mcuParameters.numericValues.at(1));
        const qint64 lowPowerGapMs = gapIndex >= 0 && gapIndex < int(std::size(gapSeconds))
            ? qint64(gapSeconds[gapIndex]) * 1000
            : 0;
        const qint64 scheduledReadIntervalMs = mode == 2
            ? (std::max)(samplingDurationMs, lowPowerGapMs)
            : samplingDurationMs;
        const int automaticReadIntervalMs = int((std::clamp)(
            scheduledReadIntervalMs,
            qint64(250),
            qint64(86400000)));
        QStringList initialTrace = trace;
        QElapsedTimer automaticReadTimer;
        automaticReadTimer.start();

        while (!QThread::currentThread()->isInterruptionRequested()) {
            if (automaticReadTimer.elapsed() < automaticReadIntervalMs) {
                QThread::msleep(qMin(statusPollIntervalMs,
                                     automaticReadIntervalMs - int(automaticReadTimer.elapsed())));
                continue;
            }

            QByteArray dataResponse;
            QString transactionError;
            if (!transactModbus(session,
                                dataRequest,
                                21,
                                slave,
                                0x04,
                                &dataResponse,
                                &transactionError)) {
                if (QThread::currentThread()->isInterruptionRequested())
                    break;
                if (++consecutiveCommunicationErrors < maximumCommunicationErrors) {
                    QThread::msleep(statusPollIntervalMs);
                    continue;
                }
                result.request = hex(dataRequest);
                result.responseText = initialTrace.join(QLatin1Char('\n'));
                result.message = transactionError;
                return result;
            }
            consecutiveCommunicationErrors = 0;
            automaticReadTimer.restart();

            CommunicationResult sampleResult;
            sampleResult.context = context;
            sampleResult.request = hex(dataRequest);
            QStringList sampleTrace;
            if (!initialTrace.isEmpty()) {
                sampleTrace = initialTrace;
                initialTrace.clear();
            }
            sampleTrace.append(QStringLiteral("DATA RX: %1").arg(hex(dataResponse)));
            sampleResult.responseBytes = dataResponse;
            sampleResult.responseText = sampleTrace.join(QLatin1Char('\n'));
            sampleResult.numericValues = result.numericValues;
            sampleResult.sample = VirtualModbusClient::parseMeasurementResponse(dataResponse);
            sampleResult.hasSample = true;
            sampleResult.success = true;
            sampleResult.message = mode == 2
                ? QCoreApplication::translate(
                      "DeviceCommunicationController",
                      "低功耗模式收到一条频振索力测量数据")
                : QCoreApplication::translate(
                      "DeviceCommunicationController",
                      "自动模式收到一条频振索力测量数据");
            QMetaObject::invokeMethod(
                this,
                [this, sampleResult] { emit frequencyTensionSampleReady(sampleResult); },
                Qt::QueuedConnection);
        }

        result.context = QStringLiteral("frequency_tension_stream_stopped");
        result.success = true;
        result.message = QCoreApplication::translate(
            "DeviceCommunicationController",
            "自动模式连续测量已停止");
        return result;
    });
}

bool DeviceCommunicationController::listen(int timeoutMs, const QString &context)
{
    if (timeoutMs <= 0 || timeoutMs > 60000)
        return false;
    return startOperation(context, [timeoutMs, context](SerialSession &session) {
        CommunicationResult result;
        result.context = context;
        result.request = QCoreApplication::translate("DeviceCommunicationController", "监听串口 %1 ms").arg(timeoutMs);
        QString error;
        if (!session.readUntilIdle(&result.responseBytes, &error, timeoutMs, 80)) {
            result.message = error;
            return result;
        }
        result.responseText = QString::fromUtf8(result.responseBytes).trimmed();
        result.numericValues = parseNumbers(result.responseText);
        result.success = true;
        result.message = QCoreApplication::translate("DeviceCommunicationController", "监听收到 %1 字节").arg(result.responseBytes.size());
        return result;
    });
}

void DeviceCommunicationController::cancel()
{
    if (m_thread)
        m_thread->requestInterruption();
}

bool DeviceCommunicationController::isRunning() const
{
    return m_thread != nullptr;
}

bool DeviceCommunicationController::isConnected() const
{
    return m_connected;
}

QString DeviceCommunicationController::portName() const
{
    return m_portName;
}

int DeviceCommunicationController::baudRate() const
{
    return m_baudRate;
}

int DeviceCommunicationController::slaveId() const
{
    return m_slaveId;
}

DeviceWireProtocol DeviceCommunicationController::protocol() const
{
    return m_protocol;
}

bool DeviceCommunicationController::startOperation(const QString &context, Operation operation)
{
    if (m_thread || m_portName.isEmpty() || m_baudRate <= 0)
        return false;

    const QString port = m_portName;
    const int baud = m_baudRate;
    auto result = std::make_shared<CommunicationResult>();
    QThread *thread = QThread::create([this, result, port, baud, context, operation = std::move(operation)] {
        QString error;
        if ((!m_serialSession->isConnected()
             || m_serialSession->portName() != port
             || m_serialSession->baudRate() != baud)
            && !m_serialSession->open(port, baud, &error)) {
            result->context = context;
            result->message = error;
            return;
        }
        *result = operation(*m_serialSession);
        if (result->context.isEmpty())
            result->context = context;
    });
    m_thread = thread;
    connect(thread, &QThread::finished, this, [this, thread, result] {
        if (m_thread == thread)
            m_thread = nullptr;
        m_connected = m_serialSession->isConnected();
        thread->deleteLater();
        emit runningChanged(false);
        emit resultReady(*result);
    });
    emit runningChanged(true);
    thread->start();
    return true;
}

} // namespace gucds
