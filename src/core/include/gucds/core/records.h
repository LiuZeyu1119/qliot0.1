#pragma once

#include <QDateTime>
#include <QString>
#include <QtGlobal>

namespace gucds {

struct DeviceRecord
{
    QString name;
    QString category;
    QString model;
    QString protocol;
    QString calibration;
    QString data1;
    QString data2;
    QString data3;
    QString data4;
    QString data5;
    QString modbus;
    QString lora;
    QString dtu;
    QString parameter1;
    QString parameter2;
    QString parameter3;
    QString parameter4;
    QString parameter5;
    int calibrationPoints = 0;
    QString parameterValue1;
    QString parameterValue2;
    QString parameterValue3;
    QString parameterValue4;
    QString parameterValue5;
    qint64 databaseId = -1;
};

struct MeasurementRecord
{
    int index = 0;
    QString deviceName;
    QDateTime timestamp;
    double value = 0.0;
    QString state;
    QString message;
};

struct FrequencyTensionMeasurementRecord
{
    int index = 0;
    QString deviceName;
    QDateTime timestamp;
    double cableForceKn = 0.0;
    double naturalFrequencyHz = 0.0;
    double order = 0.0;
    double convergenceErrorPercent = 0.0;
};

struct FrequencyTensionParameterRecord
{
    QString sensorName;
    double supportFactor = 0.0;
    double unitMass = 0.0;
    double cableLength = 0.0;
    double area = 0.0;
    double elasticModulus = 0.0;
    double inertia = 0.0;
    double angle = 0.0;
    qint64 databaseId = -1;
};

struct SpectrumPoint
{
    int index = 0;
    double frequencyHz = 0.0;
    double amplitude = 0.0;
};

struct CalibrationRecord
{
    int point = 0;
    double measuredValue = 0.0;
    double referenceValue = 0.0;
    double temperature = 0.0;
    QDateTime timestamp;
    QString curveName;
    qint64 databaseId = -1;
};

struct BusDeviceRecord
{
    int index = 0;
    QString sensorName;
    QString model;
    int channel = 0;
    int group = 0;
    int address = 0;
    int dataCount = 0;
    QString responseCode;
    qint64 databaseId = -1;
};

} // namespace gucds
