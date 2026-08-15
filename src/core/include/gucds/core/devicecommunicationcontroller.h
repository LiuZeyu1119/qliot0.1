#pragma once

#include "gucds/core/records.h"
#include "gucds/core/virtualmodbusclient.h"

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVector>

#include <functional>
#include <memory>

class QThread;

namespace gucds {

class SerialSession;

enum class DeviceWireProtocol
{
    Modbus,
    Text,
};

struct CommunicationResult
{
    bool success = false;
    QString context;
    QString request;
    QString responseText;
    QByteArray responseBytes;
    QByteArray dataBytes;
    QString message;
    QVector<double> numericValues;
    QVector<DeviceRecord> devices;
    SensorModbusSample sample;
    bool hasSample = false;
};

class DeviceCommunicationController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DeviceCommunicationController)

public:
    explicit DeviceCommunicationController(QObject *parent = nullptr);
    ~DeviceCommunicationController() override;

    void configure(const QString &portName, int baudRate, int slaveId, DeviceWireProtocol protocol);
    bool sendTextCommand(const QString &request, const QString &context = {}, int repeatCount = 1);
    bool sendTextCommands(const QStringList &requests,
                          const QString &context = {},
                          int intervalMs = 0);
    bool sendModbusCommand(SensorModbusCommand command, const QString &context = {});
    bool writeModbusRegisters(quint16 startAddress,
                              const QByteArray &bytes,
                              const QString &context = {},
                              SensorModbusCommand saveCommand = SensorModbusCommand(0));
    bool writeFrequencyMcuParameters(const QByteArray &mcuBytes,
                                     quint8 sampleRateIndex,
                                     quint8 samplePointIndex,
                                     const QString &context = {});
    bool readModbusRegisters(quint16 byteOffset, quint16 registerCount, const QString &context = {});
    bool scanDevices(const QString &context = QStringLiteral("scan"));
    bool measure(const QString &context = QStringLiteral("measurement"));
    bool measureFrequencyTension(const QString &context = QStringLiteral("frequency_measurement"));
    bool listen(int timeoutMs = 3000, const QString &context = QStringLiteral("listen"));
    void cancel();

    bool isRunning() const;
    bool isConnected() const;
    QString portName() const;
    int baudRate() const;
    int slaveId() const;
    DeviceWireProtocol protocol() const;

signals:
    void runningChanged(bool running);
    void frequencyTensionSampleReady(const gucds::CommunicationResult &result);
    void resultReady(const gucds::CommunicationResult &result);

private:
    using Operation = std::function<CommunicationResult(SerialSession &)>;

    bool startOperation(const QString &context, Operation operation);

    std::unique_ptr<SerialSession> m_serialSession;
    QThread *m_thread = nullptr;
    QString m_portName = QStringLiteral("COM1");
    int m_baudRate = 9600;
    int m_slaveId = 1;
    DeviceWireProtocol m_protocol = DeviceWireProtocol::Modbus;
    bool m_connected = false;
};

} // namespace gucds

Q_DECLARE_METATYPE(gucds::CommunicationResult)
