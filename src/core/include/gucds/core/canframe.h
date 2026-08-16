#pragma once

#include <QByteArray>
#include <QMetaType>
#include <QtGlobal>

namespace gucds {

struct CanFrame
{
    quint32 id = 0;
    QByteArray payload;
    quint8 dlc = 0;
    quint64 hardwareTimestampUs = 0;
    qint64 wallClockMs = 0;
    bool extended = false;
    bool remote = false;
    bool transmitted = false;
};

} // namespace gucds

Q_DECLARE_METATYPE(gucds::CanFrame)
