#include "gucds/core/appconfig.h"

#include <QCoreApplication>

namespace gucds {

QString AppConfig::applicationTitle()
{
    return QCoreApplication::translate("AppConfig", "奇力智造上位机调试系统QL-IOT App5.7.4");
}

QString AppConfig::applicationDescription()
{
    return QCoreApplication::translate("AppConfig", "拉线式位移传感器上位机");
}

QString AppConfig::organizationName()
{
    return QCoreApplication::translate("AppConfig", "北京奇力建通工程技术有限公司");
}

QString AppConfig::defaultServerHost()
{
    return QStringLiteral("43.139.170.206");
}

quint16 AppConfig::defaultServerPort()
{
    return 1002;
}

QStringList AppConfig::mainTabs()
{
    return {
        QCoreApplication::translate("AppConfig", "测试数据"),
        QCoreApplication::translate("AppConfig", "设备配置"),
        QCoreApplication::translate("AppConfig", "设备标定"),
        QCoreApplication::translate("AppConfig", "总线设备/网关"),
    };
}

QStringList AppConfig::baudRates()
{
    QStringList rates = deviceBaudRates();
    rates.append({
        QStringLiteral("128000"),
        QStringLiteral("230400"),
        QStringLiteral("256000"),
        QStringLiteral("460800"),
    });
    return rates;
}

QStringList AppConfig::deviceBaudRates()
{
    return {
        QStringLiteral("9600"),
        QStringLiteral("14400"),
        QStringLiteral("19200"),
        QStringLiteral("38400"),
        QStringLiteral("56000"),
        QStringLiteral("57600"),
        QStringLiteral("115200"),
    };
}

QStringList AppConfig::jumpCommands()
{
    return {
        QStringLiteral("Find"),
        QStringLiteral("Start"),
        QStringLiteral("MCU_SET"),
        QStringLiteral("LoRa_SET"),
        QCoreApplication::translate("AppConfig", "标定测量"),
        QCoreApplication::translate("AppConfig", "复位和校准"),
        QCoreApplication::translate("AppConfig", "恢复出厂设置"),
        QStringLiteral("Test"),
        QCoreApplication::translate("AppConfig", "AT指令"),
        QCoreApplication::translate("AppConfig", "总线网关RT"),
        QCoreApplication::translate("AppConfig", "默认"),
        QStringLiteral("DTU_SET"),
        QStringLiteral("DTU_GET"),
        QStringLiteral("DTU_ATSET"),
        QStringLiteral("DTU_TEST"),
        QCoreApplication::translate("AppConfig", "读频谱"),
        QCoreApplication::translate("AppConfig", "写频振扩展参数"),
        QCoreApplication::translate("AppConfig", "传感器测试"),
        QCoreApplication::translate("AppConfig", "自定义-TTL通讯"),
        QCoreApplication::translate("AppConfig", "读闪存数据"),
        QCoreApplication::translate("AppConfig", "磁通量开发读取"),
    };
}

QStringList AppConfig::loraAirRates()
{
    return {
        QStringLiteral("1.2k"),
        QStringLiteral("2.4k"),
        QStringLiteral("4.8k"),
        QStringLiteral("9.6k"),
        QStringLiteral("19.2k"),
        QStringLiteral("38.4k"),
        QStringLiteral("62.5k"),
    };
}

QStringList AppConfig::loraPowers()
{
    return {
        QStringLiteral("11dBm"),
        QStringLiteral("14dBm"),
        QStringLiteral("17dBm"),
        QStringLiteral("20dBm"),
    };
}

QStringList AppConfig::loraWorkModes()
{
    return {
        QCoreApplication::translate("AppConfig", "定点模式"),
        QCoreApplication::translate("AppConfig", "主从模式"),
        QCoreApplication::translate("AppConfig", "透传模式"),
    };
}

QStringList AppConfig::loraMasterSlaveModes()
{
    return {
        QCoreApplication::translate("AppConfig", "主机"),
        QCoreApplication::translate("AppConfig", "从机"),
    };
}

QStringList AppConfig::mcuModes()
{
    return {
        QCoreApplication::translate("AppConfig", "指令"),
        QCoreApplication::translate("AppConfig", "自动"),
        QCoreApplication::translate("AppConfig", "低功耗"),
    };
}

QStringList AppConfig::networkProtocols()
{
    return {
        QStringLiteral("mqtt"),
        QStringLiteral("http"),
        QStringLiteral("tcp"),
        QStringLiteral("udp"),
        QStringLiteral("webs"),
    };
}

QStringList AppConfig::serialChannels()
{
    return {
        QStringLiteral("ttluart"),
        QStringLiteral("rs232"),
        QStringLiteral("rs485"),
        QStringLiteral("uart"),
        QStringLiteral("uart_2"),
        QStringLiteral("rs485_2"),
        QStringLiteral("rs485_3"),
    };
}

} // namespace gucds
