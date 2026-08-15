#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

namespace gucds {

enum class AtCommand
{
    MeasureStart,
    GetStringData,
    GetSpectrum,
    GetMcuParameters,
    GetSensorParameters,
    GetSensorId,
    GetAllSensorParameters,
    GetLoraParameters,
    GetSecondaryData,
    GetFrequencyParameters,
    GetExtendedParameters,
    RestoreReference,
    Calibrate,
    SetSensorParameters,
    ConfigReboot,
    ConfigReset,
    ConfigSave,
};

struct ParsedStatus
{
    QString code;
    QString message;
    bool success = false;
};

struct DtuHttpConfig
{
    int channel = 1;
    QString serialChannel = QStringLiteral("uart");
    QString host;
    int port = 80;
    int requestMethod = 1;
    QString path = QStringLiteral("/");
    int timeoutSeconds = 30;
    bool customHeaderEnabled = false;
    QString customHeader;
    bool responseFilterEnabled = false;
    int registrationType = 0;
    QString registrationData;
    bool ipv6 = false;
    int sslMode = 0;
};

struct DtuMqttConfig
{
    int channel = 1;
    QString serialChannel = QStringLiteral("uart");
    QString host;
    int port = 1883;
    int heartbeatSeconds = 120;
    QString clientId = QStringLiteral("${IMEI}");
    QString username;
    QString password;
    int protocolVersion = 1;
    bool cleanSession = true;
    bool retain = false;
    int subscribeQos = 0;
    int publishQos = 0;
    QString subscribeTopic;
    QString publishTopic;
    bool willEnabled = false;
    int willQos = 0;
    bool willRetain = false;
    QString willTopic;
    QString willPayload;
    int registrationType = 0;
    QString registrationData;
    bool ipv6 = false;
    int sslMode = 0;
};

struct DtuSocketConfig
{
    QString protocol = QStringLiteral("tcp");
    int channel = 1;
    QString serialChannel = QStringLiteral("uart");
    bool heartbeatEnabled = true;
    int heartbeatDataType = 0;
    QString heartbeatData = QStringLiteral("00");
    int heartbeatSeconds = 60;
    QString host;
    int port = 80;
    int prefixType = 0;
    QString prefixData;
    int suffixType = 0;
    QString suffixData;
    int registrationType = 0;
    QString registrationData;
    bool ipv6 = false;
    int sslMode = 0;
};

struct DtuWebSocketConfig
{
    int channel = 1;
    QString serialChannel = QStringLiteral("uart");
    QString serverUrl;
    bool ipv6 = false;
    bool customHeaderEnabled = false;
    QString customHeader;
    bool heartbeatEnabled = true;
    int heartbeatDataType = 0;
    QString heartbeatData = QStringLiteral("00");
    int heartbeatSeconds = 20;
    int prefixType = 0;
    QString prefixData;
    int suffixType = 0;
    QString suffixData;
    int registrationType = 0;
    QString registrationData;
};

struct SampleStreamPoint
{
    int sequence = 0;
    float accX = 0.0f;
    float accY = 0.0f;
    float accZ = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};

struct FrequencyTensionSample
{
    double cableForceKn = 0.0;
    double naturalFrequencyHz = 0.0;
    double order = 0.0;
    double convergenceErrorPercent = 0.0;
};

struct SpectrumSample
{
    double frequencyHz = 0.0;
    double amplitude = 0.0;
};

class AtProtocol
{
public:
    static QString command(AtCommand command);
    static QString buildSensorTest(const QStringList &arguments);
    static QString buildSetDeviceParameters(const QStringList &arguments);
    static QString buildSetExtendedParameters(const QStringList &arguments);
    static QString buildConfigSet(const QString &key, const QString &value);
    static QString buildDtuSocketConfig(const QString &protocol,
                                        int channel,
                                        const QString &host,
                                        int port,
                                        bool ipv6 = false,
                                        int sslMode = 0);
    static QString buildDtuSocketConfig(const DtuSocketConfig &config);
    static QString buildDtuHttpConfig(int channel,
                                      const QString &serverAddress,
                                      int port,
                                      const QString &urlPath,
                                      bool post = false,
                                      bool ipv6 = false,
                                      int sslMode = 0);
    static QString buildDtuHttpConfig(const DtuHttpConfig &config);
    static QString buildDtuMqttConfig(const DtuMqttConfig &config);
    static QString buildDtuWebSocketConfig(const DtuWebSocketConfig &config);
    static QString buildDeleteDtuChannel(int channel);
    static QString buildSetDateTime(const QDateTime &dateTime);
    static QString buildBluetoothGet(const QString &key);
    static QString buildBluetoothSet(const QString &key, const QString &value);
    static QString buildBluetoothReset(const QString &key);
    static ParsedStatus parseStatus(const QString &packet);
    static QVector<SampleStreamPoint> parseSampleStream(const QString &packet, bool *finished = nullptr);
    static bool parseFrequencyTensionMeasurement(const QString &packet, FrequencyTensionSample *sample);
    static QVector<SpectrumSample> parseSpectrum(const QString &packet, double frequencyBinWidthHz = 0.0);
    static QStringList knownStatusCodes();
};

} // namespace gucds
