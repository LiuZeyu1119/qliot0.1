#include "gucds/core/virtualtransport.h"

namespace gucds {

VirtualTransport::VirtualTransport() = default;

QString VirtualTransport::backendName() const
{
    return QStringLiteral("VirtualTransport");
}

QVector<DeviceRecord> VirtualTransport::scanDevices(QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();
    return m_device.scanDevices();
}

VirtualDeviceReply VirtualTransport::transact(const QString &request)
{
    return m_device.transact(request);
}

VirtualDevice &VirtualTransport::device()
{
    return m_device;
}

const VirtualDevice &VirtualTransport::device() const
{
    return m_device;
}

} // namespace gucds
