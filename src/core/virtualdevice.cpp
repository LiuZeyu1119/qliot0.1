#include "gucds/core/virtualdevice.h"

#include "gucds/core/atprotocol.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QStringList>

namespace {

QString virtualDeviceText(const char *source)
{
    return QCoreApplication::translate("VirtualDevice", source);
}

} // namespace

namespace gucds {

VirtualDevice::VirtualDevice()
    : m_devices({
          {QStringLiteral("Sen01"), QStringLiteral("加速度传感器"), QStringLiteral("QL-MACS-RXXXX-GP3"), QStringLiteral("Modbus"), QStringLiteral("默认")},
          {QStringLiteral("Sen02"), QStringLiteral("加速度传感器"), QStringLiteral("QL-MACS-RXXXX-EC3"), QStringLiteral("TTL/AT"), QStringLiteral("默认")},
          {QStringLiteral("Sen08"), QStringLiteral("磁通量传感器"), QStringLiteral("QL-CMFS-LSDIG-x"), QStringLiteral("LoRa"), QStringLiteral("磁通量D90")},
      })
{
    m_config.insert(QStringLiteral("MQTT_KEY"), QStringLiteral("mqtt://43.139.170.206:1002"));
    m_config.insert(QStringLiteral("HTTP_KEY"), QStringLiteral("http://43.139.170.206:1002"));
    m_config.insert(QStringLiteral("TCP_KEY"), QStringLiteral("43.139.170.206:1002"));
    m_config.insert(QStringLiteral("UDP_KEY"), QStringLiteral("43.139.170.206:1002"));
}

QVector<DeviceRecord> VirtualDevice::scanDevices() const
{
    return m_devices;
}

VirtualDeviceReply VirtualDevice::transact(const QString &request)
{
    const QString trimmed = request.trimmed();
    if (trimmed == AtProtocol::command(AtCommand::MeasureStart)) {
        VirtualDeviceReply reply;
        reply.success = true;
        reply.request = trimmed;
        reply.payload = QStringLiteral("MEASURE_OK");
        reply.message = virtualDeviceText("测量完成，正在读取数据");
        reply.measurement = measure();
        return reply;
    }

    if (trimmed == AtProtocol::command(AtCommand::GetStringData)) {
        VirtualDeviceReply reply;
        reply.success = true;
        reply.request = trimmed;
        reply.payload = QStringLiteral("Sen01,499.950,OK");
        reply.message = virtualDeviceText("字符串数据读取成功");
        return reply;
    }

    if (trimmed == AtProtocol::command(AtCommand::GetSpectrum)) {
        VirtualDeviceReply reply;
        reply.success = true;
        reply.request = trimmed;
        reply.payload = QStringLiteral("SPEC_OK");
        reply.message = virtualDeviceText("频谱数据读取成功");
        reply.spectrum = spectrum();
        return reply;
    }

    if (trimmed == AtProtocol::command(AtCommand::ConfigSave)) {
        VirtualDeviceReply reply;
        reply.success = true;
        reply.request = trimmed;
        reply.payload = QStringLiteral("CONFIG_SAVE_OK");
        reply.message = virtualDeviceText("DTU 网络配置保存成功");
        return reply;
    }

    if (trimmed.startsWith(QStringLiteral("config,set,")))
        return handleConfigSet(trimmed);

    if (trimmed.startsWith(QStringLiteral("AT,set,devpar,")))
        return handleDeviceParameterSet(trimmed);

    if (trimmed.startsWith(QStringLiteral("AT,set,exppar,"))) {
        VirtualDeviceReply reply;
        reply.success = true;
        reply.request = trimmed;
        reply.payload = QStringLiteral("EXPPAR_OK");
        reply.message = virtualDeviceText("扩展参数配置成功");
        return reply;
    }

    if (trimmed.startsWith(QStringLiteral("AT,set,sentest,"))) {
        VirtualDeviceReply reply;
        reply.success = true;
        reply.request = trimmed;
        reply.payload = QStringLiteral("SENTEST_OK");
        reply.message = virtualDeviceText("传感器测试命令完成");
        return reply;
    }

    VirtualDeviceReply reply;
    reply.request = trimmed;
    reply.payload = QStringLiteral("AT_PARSINGFAILED");
    reply.message = virtualDeviceText("虚拟设备不识别该命令");
    return reply;
}

MeasurementRecord VirtualDevice::measure(const QString &deviceName)
{
    ++m_measurementCounter;
    const double value = 499.950 + (m_measurementCounter % 9) * 0.012;
    return {
        m_measurementCounter,
        deviceName,
        QDateTime::currentDateTime(),
        value,
        QStringLiteral("OK"),
        virtualDeviceText("虚拟设备测量完成"),
    };
}

QVector<double> VirtualDevice::spectrum() const
{
    return {
        0.012,
        0.030,
        0.071,
        0.118,
        0.244,
        0.501,
        0.330,
        0.162,
        0.084,
        0.040,
        0.022,
    };
}

QString VirtualDevice::configValue(const QString &key) const
{
    return m_config.value(key);
}

VirtualDeviceReply VirtualDevice::handleConfigSet(const QString &request)
{
    const QStringList parts = request.split(QLatin1Char(','));
    VirtualDeviceReply reply;
    reply.request = request;

    if (parts.size() < 4) {
        reply.payload = QStringLiteral("DEVPAR_FORMATERROR");
        reply.message = virtualDeviceText("DTU 网络配置格式错误");
        return reply;
    }

    const QString key = parts.at(2);
    const QString value = parts.mid(3).join(QLatin1Char(','));
    m_config.insert(key, value);

    reply.success = true;
    reply.payload = QStringLiteral("DEVPAR_OK");
    reply.message = virtualDeviceText("DTU 网络配置写入成功");
    return reply;
}

VirtualDeviceReply VirtualDevice::handleDeviceParameterSet(const QString &request)
{
    VirtualDeviceReply reply;
    reply.request = request;

    const QStringList parts = request.split(QLatin1Char(','), Qt::KeepEmptyParts);
    if (parts.size() <= 3) {
        reply.payload = QStringLiteral("DEVPAR_NUMERROR");
        reply.message = virtualDeviceText("设备参数数量错误");
        return reply;
    }

    reply.success = true;
    reply.payload = QStringLiteral("DEVPAR_OK");
    reply.message = virtualDeviceText("设备参数配置成功");
    return reply;
}

} // namespace gucds
