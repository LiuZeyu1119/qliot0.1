#include "gucds/core/atprotocol.h"

#include <QCoreApplication>
#include <QPair>
#include <QRegularExpression>
#include <QUrl>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <utility>

namespace gucds {

namespace {

QVector<QPair<QString, ParsedStatus>> statusCatalog()
{
    return {
        {QStringLiteral("AT_PARSINGFAILED"), {QStringLiteral("AT_PARSINGFAILED"), QCoreApplication::translate("AtProtocol", "AT 回包解析失败"), false}},
        {QStringLiteral("ATData_ADDOVER"), {QStringLiteral("ATData_ADDOVER"), QCoreApplication::translate("AtProtocol", "AT 数据追加数量越界"), false}},
        {QStringLiteral("ATData_FlashOver"), {QStringLiteral("ATData_FlashOver"), QCoreApplication::translate("AtProtocol", "AT 数据写入 Flash 溢出"), false}},
        {QStringLiteral("ATData_NUMOVER"), {QStringLiteral("ATData_NUMOVER"), QCoreApplication::translate("AtProtocol", "AT 数据数量越界"), false}},
        {QStringLiteral("CALIBRAT_OK"), {QStringLiteral("CALIBRAT_OK"), QCoreApplication::translate("AtProtocol", "校准/复位成功"), true}},
        {QStringLiteral("CALIBRAT_ERROR"), {QStringLiteral("CALIBRAT_ERROR"), QCoreApplication::translate("AtProtocol", "校准/复位失败"), false}},
        {QStringLiteral("DEVPAR_OK"), {QStringLiteral("DEVPAR_OK"), QCoreApplication::translate("AtProtocol", "设备参数配置成功"), true}},
        {QStringLiteral("DEVPAR_ADDOVERFLOW"), {QStringLiteral("DEVPAR_ADDOVERFLOW"), QCoreApplication::translate("AtProtocol", "设备参数追加溢出"), false}},
        {QStringLiteral("DEVPAR_FLASHOVERFLOW"), {QStringLiteral("DEVPAR_FLASHOVERFLOW"), QCoreApplication::translate("AtProtocol", "设备参数 Flash 溢出"), false}},
        {QStringLiteral("DEVPAR_FORMATERROR"), {QStringLiteral("DEVPAR_FORMATERROR"), QCoreApplication::translate("AtProtocol", "设备参数格式错误"), false}},
        {QStringLiteral("DEVPAR_NUMERROR"), {QStringLiteral("DEVPAR_NUMERROR"), QCoreApplication::translate("AtProtocol", "设备参数数量错误"), false}},
        {QStringLiteral("EXPPAR_OK"), {QStringLiteral("EXPPAR_OK"), QCoreApplication::translate("AtProtocol", "扩展参数配置成功"), true}},
        {QStringLiteral("EXPPAR_FLASHERR"), {QStringLiteral("EXPPAR_FLASHERR"), QCoreApplication::translate("AtProtocol", "扩展参数 Flash 写入错误"), false}},
        {QStringLiteral("EXPPAR_NUMERROR"), {QStringLiteral("EXPPAR_NUMERROR"), QCoreApplication::translate("AtProtocol", "扩展参数数量错误"), false}},
        {QStringLiteral("EXPPAR_TYPEERROR"), {QStringLiteral("EXPPAR_TYPEERROR"), QCoreApplication::translate("AtProtocol", "扩展参数类型错误"), false}},
        {QStringLiteral("EXPPAR_VALUEERROR"), {QStringLiteral("EXPPAR_VALUEERROR"), QCoreApplication::translate("AtProtocol", "扩展参数值错误"), false}},
        {QStringLiteral("MEASTART_OK"), {QStringLiteral("MEASTART_OK"), QCoreApplication::translate("AtProtocol", "连续采样启动成功"), true}},
        {QStringLiteral("MEASTART_BUSY"), {QStringLiteral("MEASTART_BUSY"), QCoreApplication::translate("AtProtocol", "连续采样正在进行"), false}},
        {QStringLiteral("MEASTART_ERROR"), {QStringLiteral("MEASTART_ERROR"), QCoreApplication::translate("AtProtocol", "连续采样启动失败"), false}},
        {QStringLiteral("REFSET_OK"), {QStringLiteral("REFSET_OK"), QCoreApplication::translate("AtProtocol", "恢复出厂/参考设置成功"), true}},
        {QStringLiteral("REFSET_ERROR"), {QStringLiteral("REFSET_ERROR"), QCoreApplication::translate("AtProtocol", "恢复出厂/参考设置失败"), false}},
        {QStringLiteral("secparaERR_COUNT"), {QStringLiteral("secparaERR_COUNT"), QCoreApplication::translate("AtProtocol", "二级参数数量错误"), false}},
        {QStringLiteral("secparaERR_SENNUM"), {QStringLiteral("secparaERR_SENNUM"), QCoreApplication::translate("AtProtocol", "传感器数量错误"), false}},
        {QStringLiteral("SETDEVPAR_NUMERROR"), {QStringLiteral("SETDEVPAR_NUMERROR"), QCoreApplication::translate("AtProtocol", "写设备参数数量错误"), false}},
        {QStringLiteral("SETDEVPAR_NUMOVERFLOW"), {QStringLiteral("SETDEVPAR_NUMOVERFLOW"), QCoreApplication::translate("AtProtocol", "写设备参数数量超过 U8 范围"), false}},
        {QStringLiteral("secparaERR_UNSUPPORT"), {QStringLiteral("secparaERR_UNSUPPORT"), QCoreApplication::translate("AtProtocol", "不支持该二级命令"), false}},
        {QStringLiteral("secparaERR_CMD"), {QStringLiteral("secparaERR_CMD"), QCoreApplication::translate("AtProtocol", "二级命令无效"), false}},
        {QStringLiteral("ERR_CMD"), {QStringLiteral("ERR_CMD"), QCoreApplication::translate("AtProtocol", "命令无效"), false}},
        {QStringLiteral("ERR_PARAM"), {QStringLiteral("ERR_PARAM"), QCoreApplication::translate("AtProtocol", "命令参数无效"), false}},
        {QStringLiteral("ERR_MODULE"), {QStringLiteral("ERR_MODULE"), QCoreApplication::translate("AtProtocol", "模块执行命令失败"), false}},
        {QStringLiteral("SETTYP_NULL"), {QStringLiteral("SETTYP_NULL"), QCoreApplication::translate("AtProtocol", "缺少 SET 命令类型"), false}},
        {QStringLiteral("ATSET_UNDEFINE"), {QStringLiteral("ATSET_UNDEFINE"), QCoreApplication::translate("AtProtocol", "未定义的 SET 命令"), false}},
    };
}

QString joinArguments(const QString &prefix, const QStringList &arguments)
{
    if (arguments.isEmpty())
        return prefix;
    return prefix + arguments.join(QLatin1Char(','));
}

bool isValidDtuField(const QString &value, bool allowEmpty = false)
{
    return (allowEmpty || !value.isEmpty())
        && !value.contains(QLatin1Char(','))
        && !value.contains(QLatin1Char('\r'))
        && !value.contains(QLatin1Char('\n'));
}

QString buildDtuCommand(const QStringList &fields)
{
    const QString command = fields.join(QLatin1Char(','));
    return command.toUtf8().size() <= 254 ? command : QString();
}

bool isValidSerialChannel(const QString &channel)
{
    static const QStringList channels = {
        QStringLiteral("ttluart"),
        QStringLiteral("rs232"),
        QStringLiteral("rs485"),
        QStringLiteral("uart"),
        QStringLiteral("uart_2"),
        QStringLiteral("rs485_2"),
        QStringLiteral("rs485_3"),
    };
    return channels.contains(channel.trimmed().toLower());
}

QString dtuDataField(const QString &value)
{
    const QString normalized = value.trimmed();
    return normalized.isEmpty() ? QStringLiteral("0") : normalized;
}

bool isValidTypedData(int type, int maximumType, const QString &data)
{
    if (type < 0 || type > maximumType)
        return false;
    return type < 2 || isValidDtuField(data.trimmed());
}

QVector<double> numbersInText(const QString &text)
{
    static const QRegularExpression numberPattern(
        QStringLiteral(R"([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)"));
    QVector<double> numbers;
    auto match = numberPattern.globalMatch(text);
    while (match.hasNext()) {
        bool ok = false;
        const double value = match.next().captured().toDouble(&ok);
        if (ok && std::isfinite(value))
            numbers.append(value);
    }
    return numbers;
}

QVector<double> numericFields(const QString &text)
{
    QVector<double> numbers;
    const QStringList fields = text.split(QRegularExpression(QStringLiteral(R"([,\t\s]+)")), Qt::SkipEmptyParts);
    for (const QString &field : fields) {
        bool ok = false;
        const double value = field.toDouble(&ok);
        if (ok && std::isfinite(value))
            numbers.append(value);
    }
    return numbers;
}

bool containsAny(const QString &text, const QStringList &tokens)
{
    return std::any_of(tokens.cbegin(), tokens.cend(), [&text](const QString &token) {
        return text.contains(token, Qt::CaseInsensitive);
    });
}

bool looksLikeFrequencyAxis(const QVector<double> &values)
{
    if (values.size() < 2 || values.first() < 0.0)
        return false;

    const double expectedStep = (values.last() - values.first()) / double(values.size() - 1);
    if (!(expectedStep > 0.0) || !std::isfinite(expectedStep))
        return false;

    const double tolerance = (std::max)(1e-9, expectedStep * 0.01);
    for (qsizetype index = 1; index < values.size(); ++index) {
        const double step = values.at(index) - values.at(index - 1);
        if (!std::isfinite(values.at(index)) || std::abs(step - expectedStep) > tolerance)
            return false;
    }
    return true;
}

QVector<SpectrumSample> zipSpectrum(const QVector<double> &frequencies,
                                    const QVector<double> &amplitudes)
{
    if (frequencies.size() != amplitudes.size() || !looksLikeFrequencyAxis(frequencies))
        return {};

    QVector<SpectrumSample> samples;
    samples.reserve(frequencies.size());
    for (qsizetype index = 0; index < frequencies.size(); ++index) {
        if (!std::isfinite(amplitudes.at(index)))
            return {};
        samples.append({frequencies.at(index), amplitudes.at(index)});
    }
    return samples;
}

bool labeledNumber(const QString &text, const QStringList &labels, double *value)
{
    static const QString number = QStringLiteral(R"(([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?))");
    for (const QString &label : labels) {
        const QRegularExpression pattern(
            QStringLiteral(R"((?:^|[,;\s])%1\s*[:=]\s*%2)")
                .arg(QRegularExpression::escape(label), number),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = pattern.match(text);
        if (!match.hasMatch())
            continue;
        bool ok = false;
        const double parsed = match.captured(1).toDouble(&ok);
        if (ok && std::isfinite(parsed)) {
            if (value)
                *value = parsed;
            return true;
        }
    }
    return false;
}

} // namespace

QString AtProtocol::command(AtCommand command)
{
    switch (command) {
    case AtCommand::MeasureStart:
        return QStringLiteral("AT,set,meastart");
    case AtCommand::GetStringData:
        return QStringLiteral("AT,get,stringdata");
    case AtCommand::GetSpectrum:
        return QStringLiteral("AT,get,spec");
    case AtCommand::GetMcuParameters:
        return QStringLiteral("AT,get,mcupar");
    case AtCommand::GetSensorParameters:
        return QStringLiteral("AT,get,senpar");
    case AtCommand::GetSensorId:
        return QStringLiteral("AT,get,senID");
    case AtCommand::GetAllSensorParameters:
        return QStringLiteral("AT,get,allpar");
    case AtCommand::GetLoraParameters:
        return QStringLiteral("AT,get,lorpar");
    case AtCommand::GetSecondaryData:
        return QStringLiteral("AT,get,secdata");
    case AtCommand::GetFrequencyParameters:
        return QStringLiteral("AT,get,FVCFexppar");
    case AtCommand::GetExtendedParameters:
        return QStringLiteral("AT,get,exppar");
    case AtCommand::RestoreReference:
        return QStringLiteral("AT,set,refset");
    case AtCommand::Calibrate:
        return QStringLiteral("AT,set,calibrat");
    case AtCommand::SetSensorParameters:
        return QStringLiteral("AT,set,senpar");
    case AtCommand::ConfigReboot:
        return QStringLiteral("config,set,reboot");
    case AtCommand::ConfigReset:
        return QStringLiteral("config,set,reset");
    case AtCommand::ConfigSave:
        return QStringLiteral("config,set,save");
    }
    return {};
}

QString AtProtocol::buildSensorTest(const QStringList &arguments)
{
    return joinArguments(QStringLiteral("AT,set,sentest,"), arguments);
}

QString AtProtocol::buildSetDeviceParameters(const QStringList &arguments)
{
    return joinArguments(QStringLiteral("AT,set,devpar,"), arguments);
}

QString AtProtocol::buildSetExtendedParameters(const QStringList &arguments)
{
    return joinArguments(QStringLiteral("AT,set,exppar,"), arguments);
}

QString AtProtocol::buildConfigSet(const QString &key, const QString &value)
{
    return QStringLiteral("config,set,%1,%2").arg(key, value);
}

QString AtProtocol::buildDtuSocketConfig(const QString &protocol,
                                         int channel,
                                         const QString &host,
                                         int port,
                                         bool ipv6,
                                         int sslMode)
{
    DtuSocketConfig config;
    config.protocol = protocol;
    config.channel = channel;
    config.heartbeatDataType = 1;
    config.heartbeatData = QStringLiteral("hello");
    config.host = host;
    config.port = port;
    config.ipv6 = ipv6;
    config.sslMode = sslMode;
    return buildDtuSocketConfig(config);
}

QString AtProtocol::buildDtuSocketConfig(const DtuSocketConfig &config)
{
    const QString protocol = config.protocol.trimmed().toLower();
    const QString serialChannel = config.serialChannel.trimmed().toLower();
    const QString host = config.host.trimmed();
    if ((protocol != QStringLiteral("tcp") && protocol != QStringLiteral("udp"))
        || config.channel < 1 || config.channel > 255
        || !isValidSerialChannel(serialChannel)
        || config.heartbeatDataType < 0 || config.heartbeatDataType > 1
        || config.heartbeatSeconds < 60 || config.heartbeatSeconds > 86400
        || config.port < 1 || config.port > 65535
        || config.sslMode < 0 || config.sslMode > 2
        || host.contains(QLatin1Char('/')) || !isValidDtuField(host)
        || (config.heartbeatEnabled && !isValidDtuField(config.heartbeatData.trimmed()))
        || !isValidTypedData(config.prefixType, 3, config.prefixData)
        || !isValidTypedData(config.suffixType, 3, config.suffixData)
        || !isValidTypedData(config.registrationType, 4, config.registrationData)) {
        return {};
    }

    return buildDtuCommand({
        QStringLiteral("config"),
        QStringLiteral("set"),
        protocol,
        QString::number(config.channel),
        serialChannel,
        config.heartbeatEnabled ? QStringLiteral("1") : QStringLiteral("0"),
        QString::number(config.heartbeatDataType),
        config.heartbeatEnabled ? dtuDataField(config.heartbeatData) : QStringLiteral("0"),
        QString::number(config.heartbeatSeconds),
        host,
        QString::number(config.port),
        QString::number(config.prefixType),
        dtuDataField(config.prefixData),
        QString::number(config.suffixType),
        dtuDataField(config.suffixData),
        QString::number(config.registrationType),
        dtuDataField(config.registrationData),
        config.ipv6 ? QStringLiteral("1") : QStringLiteral("0"),
        QString::number(config.sslMode),
    });
}

QString AtProtocol::buildDtuHttpConfig(int channel,
                                       const QString &serverAddress,
                                       int port,
                                       const QString &urlPath,
                                       bool post,
                                       bool ipv6,
                                       int sslMode)
{
    DtuHttpConfig config;
    config.channel = channel;
    config.host = serverAddress;
    config.port = port;
    config.requestMethod = post ? 1 : 0;
    config.path = urlPath;
    config.ipv6 = ipv6;
    config.sslMode = sslMode;
    return buildDtuHttpConfig(config);
}

QString AtProtocol::buildDtuHttpConfig(const DtuHttpConfig &config)
{
    QString address = config.host.trimmed();
    QString path = config.path.trimmed();
    const QString serialChannel = config.serialChannel.trimmed().toLower();
    if (!address.contains(QStringLiteral("://")))
        address.prepend(config.sslMode == 0 ? QStringLiteral("http://") : QStringLiteral("https://"));
    const QUrl server(address, QUrl::StrictMode);
    if (config.channel < 1 || config.channel > 255
        || !isValidSerialChannel(serialChannel)
        || config.port < 1 || config.port > 65535
        || (config.requestMethod != 0 && config.requestMethod != 1)
        || config.timeoutSeconds < 1 || config.timeoutSeconds > 300
        || config.sslMode < 0 || config.sslMode > 2
        || !isValidDtuField(address)
        || !server.isValid() || server.host().isEmpty()
        || (server.scheme() != QStringLiteral("http") && server.scheme() != QStringLiteral("https"))
        || !isValidDtuField(path, true)
        || (config.customHeaderEnabled && !isValidDtuField(config.customHeader.trimmed()))
        || config.registrationType < 0 || config.registrationType > 4
        || (config.registrationType >= 2 && !isValidDtuField(config.registrationData.trimmed()))) {
        return {};
    }

    if (path.isEmpty())
        path = QStringLiteral("/");
    else if (!path.startsWith(QLatin1Char('/')))
        path.prepend(QLatin1Char('/'));

    const QString serverUrl = server.toString(
        QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment);
    QStringList fields({
        QStringLiteral("config"),
        QStringLiteral("set"),
        QStringLiteral("http"),
        QString::number(config.channel),
        serialChannel,
        QString::number(config.requestMethod),
        serverUrl,
        QString::number(config.port),
        path,
        QString::number(config.timeoutSeconds),
        config.customHeaderEnabled ? QStringLiteral("1") : QStringLiteral("0"),
    });
    if (config.customHeaderEnabled)
        fields.append(config.customHeader.trimmed());
    fields.append(config.responseFilterEnabled ? QStringLiteral("1") : QStringLiteral("0"));
    fields.append(QString::number(config.registrationType));
    if (config.registrationType >= 2)
        fields.append(config.registrationData.trimmed());
    fields.append(config.ipv6 ? QStringLiteral("1") : QStringLiteral("0"));
    fields.append(QString::number(config.sslMode));
    return buildDtuCommand(fields);
}

QString AtProtocol::buildDtuMqttConfig(const DtuMqttConfig &config)
{
    QString host = config.host.trimmed();
    const QString clientId = config.clientId.trimmed();
    const QString username = config.username.trimmed();
    const QString password = config.password;
    const QString subscribeTopic = config.subscribeTopic.trimmed();
    const QString publishTopic = config.publishTopic.trimmed();
    const QString serialChannel = config.serialChannel.trimmed().toLower();

    if (host.startsWith(QStringLiteral("mqtt://"), Qt::CaseInsensitive))
        host.remove(0, 7);
    else if (host.startsWith(QStringLiteral("mqtts://"), Qt::CaseInsensitive))
        host.remove(0, 8);

    if (config.channel < 1 || config.channel > 255
        || !isValidSerialChannel(serialChannel)
        || config.port < 1 || config.port > 65535
        || config.heartbeatSeconds < 60 || config.heartbeatSeconds > 300
        || config.protocolVersion < 0 || config.protocolVersion > 1
        || config.subscribeQos < 0 || config.subscribeQos > 2
        || config.publishQos < 0 || config.publishQos > 2
        || config.willQos < 0 || config.willQos > 2
        || config.registrationType < 0 || config.registrationType > 4
        || config.sslMode < 0 || config.sslMode > 2
        || host.contains(QLatin1Char('/'))
        || !isValidDtuField(host) || !isValidDtuField(clientId)
        || !isValidDtuField(username, true) || !isValidDtuField(password, true)
        || !isValidDtuField(subscribeTopic, true) || !isValidDtuField(publishTopic)
        || (config.willEnabled && (!isValidDtuField(config.willTopic.trimmed())
                                   || !isValidDtuField(config.willPayload.trimmed())))
        || (config.registrationType >= 2 && !isValidDtuField(config.registrationData.trimmed()))) {
        return {};
    }

    return buildDtuCommand({
        QStringLiteral("config"),
        QStringLiteral("set"),
        QStringLiteral("mqtt"),
        QString::number(config.channel),
        serialChannel,
        QString::number(config.heartbeatSeconds),
        host,
        QString::number(config.port),
        clientId,
        username,
        password,
        QString::number(config.protocolVersion),
        config.cleanSession ? QStringLiteral("1") : QStringLiteral("0"),
        config.retain ? QStringLiteral("1") : QStringLiteral("0"),
        QString::number(config.subscribeQos),
        QString::number(config.publishQos),
        subscribeTopic,
        publishTopic,
        config.willEnabled ? QStringLiteral("1") : QStringLiteral("0"),
        QString::number(config.willQos),
        config.willRetain ? QStringLiteral("1") : QStringLiteral("0"),
        config.willEnabled ? dtuDataField(config.willTopic) : QStringLiteral("0"),
        config.willEnabled ? dtuDataField(config.willPayload) : QStringLiteral("0"),
        QString::number(config.registrationType),
        dtuDataField(config.registrationData),
        config.ipv6 ? QStringLiteral("1") : QStringLiteral("0"),
        QString::number(config.sslMode),
    });
}

QString AtProtocol::buildDtuWebSocketConfig(const DtuWebSocketConfig &config)
{
    QString serverUrl = config.serverUrl.trimmed();
    const QString serialChannel = config.serialChannel.trimmed().toLower();
    if (!serverUrl.contains(QStringLiteral("://")))
        serverUrl.prepend(QStringLiteral("ws://"));
    const QUrl server(serverUrl, QUrl::StrictMode);
    if (config.channel < 1 || config.channel > 255
        || !isValidSerialChannel(serialChannel)
        || !server.isValid() || server.host().isEmpty()
        || (server.scheme() != QStringLiteral("ws") && server.scheme() != QStringLiteral("wss"))
        || config.heartbeatDataType < 0 || config.heartbeatDataType > 1
        || config.heartbeatSeconds < 1 || config.heartbeatSeconds > 86400
        || (config.customHeaderEnabled && !isValidDtuField(config.customHeader.trimmed()))
        || (config.heartbeatEnabled && !isValidDtuField(config.heartbeatData.trimmed()))
        || !isValidTypedData(config.prefixType, 3, config.prefixData)
        || !isValidTypedData(config.suffixType, 3, config.suffixData)
        || !isValidTypedData(config.registrationType, 3, config.registrationData)) {
        return {};
    }

    return buildDtuCommand({
        QStringLiteral("config"),
        QStringLiteral("set"),
        QStringLiteral("webs"),
        QString::number(config.channel),
        serialChannel,
        serverUrl,
        config.ipv6 ? QStringLiteral("1") : QStringLiteral("0"),
        config.customHeaderEnabled ? QStringLiteral("1") : QStringLiteral("0"),
        config.customHeaderEnabled ? dtuDataField(config.customHeader) : QStringLiteral("0"),
        config.heartbeatEnabled ? QStringLiteral("1") : QStringLiteral("0"),
        QString::number(config.heartbeatDataType),
        config.heartbeatEnabled ? dtuDataField(config.heartbeatData) : QStringLiteral("0"),
        QString::number(config.heartbeatSeconds),
        QString::number(config.prefixType),
        dtuDataField(config.prefixData),
        QString::number(config.suffixType),
        dtuDataField(config.suffixData),
        QString::number(config.registrationType),
        dtuDataField(config.registrationData),
    });
}

QString AtProtocol::buildDeleteDtuChannel(int channel)
{
    if (channel < 1 || channel > 255)
        return {};
    return QStringLiteral("config,get,delnetchan,%1").arg(channel);
}

QString AtProtocol::buildSetDateTime(const QDateTime &dateTime)
{
    return QStringLiteral("AT,set,datetime,%1")
        .arg(dateTime.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
}

QString AtProtocol::buildBluetoothGet(const QString &key)
{
    return QStringLiteral("AT,get,ble,%1").arg(key.trimmed());
}

QString AtProtocol::buildBluetoothSet(const QString &key, const QString &value)
{
    return QStringLiteral("AT,set,ble,%1,%2").arg(key.trimmed(), value.trimmed());
}

QString AtProtocol::buildBluetoothReset(const QString &key)
{
    return QStringLiteral("AT,rst,ble,%1").arg(key.trimmed());
}

ParsedStatus AtProtocol::parseStatus(const QString &packet)
{
    for (const auto &item : statusCatalog()) {
        if (packet.contains(item.first, Qt::CaseInsensitive))
            return item.second;
    }

    QString normalizedPacket = packet;
    normalizedPacket.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    const QStringList lines = normalizedPacket.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList fields = line.trimmed().split(QLatin1Char(','));
        if (fields.size() < 3 || fields.at(0).compare(QStringLiteral("config"), Qt::CaseInsensitive) != 0)
            continue;
        if (fields.at(2).compare(QStringLiteral("ok"), Qt::CaseInsensitive) == 0) {
            return {
                QStringLiteral("DTU_CONFIG_OK"),
                QCoreApplication::translate("AtProtocol", "DTU 命令执行成功"),
                true,
            };
        }
        if (fields.at(2).compare(QStringLiteral("error"), Qt::CaseInsensitive) == 0) {
            return {
                QStringLiteral("DTU_CONFIG_ERROR"),
                QCoreApplication::translate("AtProtocol", "DTU 命令执行失败（错误码 %1）")
                    .arg(fields.value(3, QCoreApplication::translate("AtProtocol", "未知"))),
                false,
            };
        }
    }

    return {
        QStringLiteral("UNKNOWN"),
        QCoreApplication::translate("AtProtocol", "未知回包状态"),
        false,
    };
}

QVector<SampleStreamPoint> AtProtocol::parseSampleStream(const QString &packet, bool *finished)
{
    if (finished)
        *finished = packet.contains(QLatin1Char('!'));

    QVector<SampleStreamPoint> points;
    const QString normalized = packet.left(packet.indexOf(QLatin1Char('!')) >= 0
                                               ? packet.indexOf(QLatin1Char('!'))
                                               : packet.size());
    const QStringList frames = normalized.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    points.reserve(frames.size());

    for (const QString &frame : frames) {
        const QStringList fields = frame.split(QLatin1Char(','), Qt::KeepEmptyParts);
        if (fields.size() != 6)
            continue;

        bool ok = false;
        SampleStreamPoint point;
        point.sequence = fields.at(0).trimmed().toInt(&ok);
        if (!ok)
            continue;

        point.accX = fields.at(1).trimmed().toFloat(&ok);
        if (!ok)
            continue;
        point.accY = fields.at(2).trimmed().toFloat(&ok);
        if (!ok)
            continue;
        point.accZ = fields.at(3).trimmed().toFloat(&ok);
        if (!ok)
            continue;
        point.pitch = fields.at(4).trimmed().toFloat(&ok);
        if (!ok)
            continue;
        point.roll = fields.at(5).trimmed().toFloat(&ok);
        if (!ok)
            continue;

        points.append(point);
    }

    return points;
}

bool AtProtocol::parseFrequencyTensionMeasurement(const QString &packet, FrequencyTensionSample *sample)
{
    if (!sample)
        return false;

    FrequencyTensionSample parsed;
    const bool hasLabels = labeledNumber(packet, {QStringLiteral("索力"), QStringLiteral("force")}, &parsed.cableForceKn)
        && labeledNumber(packet, {QStringLiteral("fn"), QStringLiteral("频率"), QStringLiteral("frequency")}, &parsed.naturalFrequencyHz)
        && labeledNumber(packet, {QStringLiteral("n"), QStringLiteral("阶数"), QStringLiteral("order")}, &parsed.order)
        && labeledNumber(packet, {QStringLiteral("收敛误差"), QStringLiteral("error")}, &parsed.convergenceErrorPercent);
    if (hasLabels) {
        *sample = parsed;
        return true;
    }

    QString normalized = packet;
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    const QStringList lines = normalized.split(QRegularExpression(QStringLiteral("[\n;]+")), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.contains(QStringLiteral("MEASTART"), Qt::CaseInsensitive)
            || line.contains(QStringLiteral("AT,"), Qt::CaseInsensitive)) {
            continue;
        }

        QVector<double> numbers = numericFields(line);
        if (numbers.size() < 4)
            continue;

        qsizetype firstValue = 0;
        if (numbers.size() >= 5
            && std::abs(numbers.at(0) - std::round(numbers.at(0))) < 1e-9
            && std::abs(numbers.at(1)) > (std::max)(10.0, std::abs(numbers.at(0)) * 10.0)) {
            firstValue = 1;
        }
        if (numbers.size() - firstValue < 4)
            continue;

        parsed.cableForceKn = numbers.at(firstValue);
        parsed.naturalFrequencyHz = numbers.at(firstValue + 1);
        parsed.order = numbers.at(firstValue + 2);
        parsed.convergenceErrorPercent = numbers.at(firstValue + 3);
        if (parsed.cableForceKn < 0.0 || parsed.naturalFrequencyHz < 0.0
            || parsed.order < 0.0 || parsed.convergenceErrorPercent < 0.0) {
            continue;
        }
        *sample = parsed;
        return true;
    }
    return false;
}

QVector<SpectrumSample> AtProtocol::parseSpectrum(const QString &packet, double frequencyBinWidthHz)
{
    QString normalized = packet;
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    const QStringList lines = normalized.split(QRegularExpression(QStringLiteral("[\n;]+")), Qt::SkipEmptyParts);

    QStringList dataLines;
    for (const QString &line : lines) {
        if (!line.contains(QStringLiteral("SPEC"), Qt::CaseInsensitive)
            && !line.contains(QStringLiteral("AT,"), Qt::CaseInsensitive)) {
            dataLines.append(line.trimmed());
        }
    }

    QVector<double> frequencyValues;
    QVector<double> amplitudeValues;
    for (const QString &line : dataLines) {
        if (containsAny(line, {QStringLiteral("frequency"), QStringLiteral("freq"), QStringLiteral("频率")})) {
            frequencyValues += numbersInText(line);
        } else if (containsAny(line, {QStringLiteral("amplitude"), QStringLiteral("amp"), QStringLiteral("幅值"), QStringLiteral("振幅")})) {
            amplitudeValues += numbersInText(line);
        }
    }
    QVector<SpectrumSample> samples = zipSpectrum(frequencyValues, amplitudeValues);
    if (!samples.isEmpty())
        return samples;

    QVector<SpectrumSample> pairs;
    bool containsNonPairLine = false;
    for (const QString &line : dataLines) {
        const QVector<double> numbers = numbersInText(line);
        if (numbers.size() < 2)
            continue;

        qsizetype frequencyIndex = 0;
        if (numbers.size() == 3
            && std::abs(numbers.at(0) - std::round(numbers.at(0))) < 1e-9) {
            frequencyIndex = 1;
        } else if (numbers.size() != 2) {
            containsNonPairLine = true;
            continue;
        }
        pairs.append({numbers.at(frequencyIndex), numbers.at(frequencyIndex + 1)});
    }
    if (!containsNonPairLine && pairs.size() >= 2) {
        QVector<double> frequencies;
        QVector<double> amplitudes;
        frequencies.reserve(pairs.size());
        amplitudes.reserve(pairs.size());
        for (const SpectrumSample &pair : std::as_const(pairs)) {
            frequencies.append(pair.frequencyHz);
            amplitudes.append(pair.amplitude);
        }
        if (looksLikeFrequencyAxis(frequencies))
            return pairs;
    }

    if (dataLines.size() == 2) {
        samples = zipSpectrum(numbersInText(dataLines.at(0)), numbersInText(dataLines.at(1)));
        if (!samples.isEmpty())
            return samples;
    }

    QVector<double> flattened;
    for (const QString &line : dataLines)
        flattened += numbersInText(line);

    if (flattened.size() >= 6 && flattened.size() % 2 == 0) {
        const qsizetype pointCount = flattened.size() / 2;
        QVector<double> interleavedFrequencies;
        QVector<double> interleavedAmplitudes;
        interleavedFrequencies.reserve(pointCount);
        interleavedAmplitudes.reserve(pointCount);
        for (qsizetype index = 0; index < pointCount; ++index) {
            interleavedFrequencies.append(flattened.at(index * 2));
            interleavedAmplitudes.append(flattened.at(index * 2 + 1));
        }
        samples = zipSpectrum(interleavedFrequencies, interleavedAmplitudes);
        if (!samples.isEmpty())
            return samples;

        const QVector<double> sequentialFrequencies = flattened.mid(0, pointCount);
        const QVector<double> sequentialAmplitudes = flattened.mid(pointCount);
        samples = zipSpectrum(sequentialFrequencies, sequentialAmplitudes);
        if (!samples.isEmpty())
            return samples;
    }

    if (!(frequencyBinWidthHz > 0.0) || !std::isfinite(frequencyBinWidthHz))
        return {};

    samples.reserve(flattened.size());
    for (qsizetype index = 0; index < flattened.size(); ++index)
        samples.append({double(index) * frequencyBinWidthHz, flattened.at(index)});
    return samples;
}

QStringList AtProtocol::knownStatusCodes()
{
    QStringList codes;
    for (const auto &item : statusCatalog())
        codes.append(item.first);
    return codes;
}

} // namespace gucds
