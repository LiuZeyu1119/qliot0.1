#pragma once

#include "gucds/core/virtualdevice.h"

#include <QString>

namespace gucds {

class Transport
{
public:
    virtual ~Transport() = default;
    virtual QString backendName() const = 0;
    virtual QVector<DeviceRecord> scanDevices(QString *errorMessage = nullptr) = 0;
    virtual VirtualDeviceReply transact(const QString &request) = 0;
};

class VirtualTransport final : public Transport
{
public:
    VirtualTransport();

    QString backendName() const override;
    QVector<DeviceRecord> scanDevices(QString *errorMessage = nullptr) override;
    VirtualDeviceReply transact(const QString &request) override;
    VirtualDevice &device();
    const VirtualDevice &device() const;

private:
    VirtualDevice m_device;
};

} // namespace gucds
