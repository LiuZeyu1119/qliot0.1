#pragma once

#include <QByteArray>
#include <QHash>
#include <QMetaType>
#include <QString>
#include <QVector>

namespace gucds {

enum class SensorModbusCommand : quint16
{
    MeasureStart = 0x00B1,
    SaveMcuParameters = 0x00B2,
    SaveLoraParameters = 0x00B3,
    Calibrate = 0x00B4,
    RestoreFactory = 0x00B5,
    Reboot = 0x00B6,
    SearchDevice = 0x00B7,
    ReadBattery = SearchDevice,
    StartContinuousSampling = 0x00B8,
    ReadAdditionalData = StartContinuousSampling,
    StopContinuousSampling = 0x00B9,
    SaveDtuParameters = StopContinuousSampling,
};

struct SensorModbusStatus
{
    quint8 code = 0;
    QString description;
    bool success = false;
};

struct SensorModbusSample
{
    float pitch = 0.0f;
    float roll = 0.0f;
    float error = 0.0f;
    float temperature = 0.0f;
    float battery = 0.0f;
};

class VirtualModbusClient
{
public:
    VirtualModbusClient();

    void setSlaveId(int slaveId);
    int slaveId() const;

    void writeHoldingRegister(quint16 address, quint16 value);
    QVector<quint16> readHoldingRegisters(quint16 startAddress, int count) const;
    QVector<quint16> readInputRegisters(quint16 startAddress, int count) const;
    quint16 holdingRegister(quint16 address) const;
    quint8 holdingByte(quint16 byteOffset) const;
    quint8 inputByte(quint16 byteOffset) const;
    SensorModbusStatus statusByte(quint16 byteOffset = 2) const;
    SensorModbusSample sample() const;
    bool transactFrame(const QByteArray &request, QByteArray *response);
    QString lastFrame() const;

    static quint16 crc(const QByteArray &payload);
    static bool hasValidCrc(const QByteArray &frame);
    static QByteArray appendCrc(const QByteArray &payload);
    static QByteArray buildReadHoldingRegisters(int slaveId, quint16 byteOffset, quint16 count);
    static QByteArray buildReadInputRegisters(int slaveId, quint16 byteOffset, quint16 count);
    static QByteArray buildWriteSingleHoldingRegister(int slaveId, quint16 address, quint16 value);
    static QByteArray buildWriteMultipleHoldingRegisters(int slaveId, quint16 startAddress, const QByteArray &bytes);
    static QByteArray encodeMcuBaud(quint32 baud);
    static quint32 decodeMcuBaud(const QByteArray &bytes, qsizetype offset = 0);
    static quint16 loraModuleModeToUpper(quint16 moduleMode);
    static quint16 loraUpperModeToModule(quint16 upperMode);
    static QString formatHex(const QByteArray &frame);
    static int expectedReadResponseBytes(quint16 count);
    static SensorModbusStatus parseStatusResponse(const QByteArray &frame);
    static SensorModbusSample parseMeasurementResponse(const QByteArray &frame);

private:
    void seedDefaults();
    void executeCommand(quint16 value);
    void advancePreciseMeasurement();
    void finishPreciseMeasurement();
    QVector<quint16> readRegistersFromBytes(const QByteArray &bytes, quint16 byteOffset, int count) const;
    QByteArray readBytes(const QByteArray &bytes, quint16 byteOffset, quint16 count) const;
    static void putLeFloat(QByteArray *bytes, int offset, float value);
    static float readLeFloat(const QByteArray &bytes, int offset);
    static SensorModbusStatus describeStatus(quint8 code);

    int m_slaveId = 1;
    QByteArray m_holdingBytes;
    QByteArray m_inputBytes;
    QHash<quint16, quint16> m_holdingRegisters;
    mutable QString m_lastFrame;
    int m_measurementCounter = 0;
    int m_pendingPrecisePolls = 0;
};

} // namespace gucds

Q_DECLARE_METATYPE(gucds::SensorModbusSample)
