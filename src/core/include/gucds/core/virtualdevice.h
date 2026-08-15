#pragma once

#include "gucds/core/records.h"

#include <QHash>
#include <QString>
#include <QVector>

namespace gucds {

struct VirtualDeviceReply
{
    bool success = false;
    QString request;
    QString payload;
    QString message;
    MeasurementRecord measurement;
    QVector<double> spectrum;
    QVector<DeviceRecord> devices;
};

class VirtualDevice
{
public:
    VirtualDevice();

    QVector<DeviceRecord> scanDevices() const;
    VirtualDeviceReply transact(const QString &request);
    MeasurementRecord measure(const QString &deviceName = QStringLiteral("Sen01"));
    QVector<double> spectrum() const;
    QString configValue(const QString &key) const;

private:
    VirtualDeviceReply handleConfigSet(const QString &request);
    VirtualDeviceReply handleDeviceParameterSet(const QString &request);

    QVector<DeviceRecord> m_devices;
    QHash<QString, QString> m_config;
    int m_measurementCounter = 0;
};

} // namespace gucds
