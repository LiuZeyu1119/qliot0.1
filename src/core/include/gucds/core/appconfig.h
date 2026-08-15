#pragma once

#include <QString>
#include <QStringList>

namespace gucds {

class AppConfig
{
public:
    static QString applicationTitle();
    static QString applicationDescription();
    static QString organizationName();
    static QString defaultServerHost();
    static quint16 defaultServerPort();

    static QStringList mainTabs();
    static QStringList baudRates();
    static QStringList deviceBaudRates();
    static QStringList jumpCommands();
    static QStringList loraAirRates();
    static QStringList loraPowers();
    static QStringList loraWorkModes();
    static QStringList loraMasterSlaveModes();
    static QStringList mcuModes();
    static QStringList networkProtocols();
    static QStringList serialChannels();
};

} // namespace gucds
