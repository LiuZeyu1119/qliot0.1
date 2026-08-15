#include "gucds/core/atprotocol.h"
#include "gucds/core/busdevicetablemodel.h"
#include "gucds/core/calibrationtablemodel.h"
#include "gucds/core/devicecommunicationcontroller.h"
#include "gucds/core/delimitedtable.h"
#include "gucds/core/deviceparameter.h"
#include "gucds/core/devicetablemodel.h"
#include "gucds/core/labviewdatabase.h"
#include "gucds/core/measurementtablemodel.h"
#include "gucds/core/producttablemodel.h"
#include "gucds/core/serialsession.h"
#include "gucds/core/virtualdevice.h"
#include "gucds/core/virtualmodbusclient.h"
#include "gucds/core/virtualtransport.h"

#include <algorithm>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QThread>
#include <QtTest/QtTest>

namespace {

QString findLabviewRootForTest()
{
    const QString projectName = QStringLiteral("General Upper Computer Debugging Software5.5");
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList basePaths = {
        QDir::currentPath(),
        appDir,
        QDir(appDir).absoluteFilePath(QStringLiteral("..")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../..")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../../..")),
    };

    for (const QString &basePath : basePaths) {
        const QString candidate = QDir(basePath).absoluteFilePath(projectName);
        if (QDir(candidate).exists())
            return candidate;
    }
    return {};
}

int sqliteCount(const QString &sqlitePath, const QString &tableName, QString *errorMessage)
{
    static int counter = 0;
    const QString connectionName = QStringLiteral("gucds_test_sqlite_%1").arg(++counter);
    int result = -1;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        db.setDatabaseName(sqlitePath);
        if (!db.open()) {
            if (errorMessage)
                *errorMessage = db.lastError().text();
            return result;
        }

        QSqlQuery query(db);
        if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(tableName)) || !query.next()) {
            if (errorMessage)
                *errorMessage = query.lastError().text();
        } else {
            result = query.value(0).toInt();
            if (errorMessage)
                errorMessage->clear();
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return result;
}

} // namespace

class ProtocolTest : public QObject
{
    Q_OBJECT

private slots:
    void buildsKnownCommands();
    void parsesStatusCodes();
    void parsesFrequencyTensionMeasurement();
    void parsesFrequencySpectrum();
    void parsesL051LoraMeasurementFrame();
    void readsDelimitedTable();
    void virtualDeviceScansDevices();
    void virtualDeviceMeasures();
    void virtualDeviceReadsSpectrum();
    void virtualDeviceStoresDtuConfig();
    void virtualTransportRejectsUnknownCommand();
    void deviceParameterDefinitionsRoundTrip();
    void deviceTableModelMutates();
    void productTableModelMutates();
    void measurementTableModelCapsRows();
    void measurementTableModelSwitchesFrequencyTensionViews();
    void communicationControllerReportsOpenFailure();
    void calibrationTableModelMutates();
    void busDeviceTableModelUpdatesResponse();
    void labviewDatabaseImportsExistingData();
    void serialSessionTransitions();
    void serialSessionCom0ComLoopback();
    void serialSessionCom0ComTextTransaction();
    void deviceCommunicationControllerCom0ComExtendedParameters();
    void frequencyMcuConfigurationFrames();
    void frequencyHardwareSameValueRoundTrip();
    void deviceCommunicationControllerCom0ComFrequencyMcuWrite();
    void deviceCommunicationControllerCom0ComAutomaticFrequencyMeasurement();
    void deviceCommunicationControllerCom0ComLowPowerFrequencyMeasurement();
    void deviceCommunicationControllerCom0ComCommandFrequencyMeasurement();
    void deviceCommunicationControllerCom0ComActiveFrame();
    void deviceCommunicationControllerCom0ComDelayedMeasurement();
    void serialSessionCom0ComF405PreciseMeasurement();
    void virtualModbusReadsWrites();
};

void ProtocolTest::buildsKnownCommands()
{
    gucds::DtuHttpConfig httpConfig;
    gucds::DtuMqttConfig mqttConfig;
    gucds::DtuSocketConfig socketConfig;
    gucds::DtuWebSocketConfig webSocketConfig;
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::MeasureStart), QStringLiteral("AT,set,meastart"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetStringData), QStringLiteral("AT,get,stringdata"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetSpectrum), QStringLiteral("AT,get,spec"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetMcuParameters), QStringLiteral("AT,get,mcupar"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetSensorParameters), QStringLiteral("AT,get,senpar"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetSensorId), QStringLiteral("AT,get,senID"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetAllSensorParameters), QStringLiteral("AT,get,allpar"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetLoraParameters), QStringLiteral("AT,get,lorpar"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetSecondaryData), QStringLiteral("AT,get,secdata"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::GetFrequencyParameters), QStringLiteral("AT,get,FVCFexppar"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::RestoreReference), QStringLiteral("AT,set,refset"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::Calibrate), QStringLiteral("AT,set,calibrat"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::SetSensorParameters), QStringLiteral("AT,set,senpar"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::ConfigReboot), QStringLiteral("config,set,reboot"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::ConfigReset), QStringLiteral("config,set,reset"));
    QCOMPARE(gucds::AtProtocol::command(gucds::AtCommand::ConfigSave), QStringLiteral("config,set,save"));
    QCOMPARE(gucds::AtProtocol::buildConfigSet(QStringLiteral("MQTT_KEY"), QStringLiteral("abc")),
             QStringLiteral("config,set,MQTT_KEY,abc"));
    QCOMPARE(gucds::AtProtocol::buildDtuSocketConfig(
                 QStringLiteral("tcp"), 1, QStringLiteral("43.139.170.206"), 1000),
             QStringLiteral("config,set,tcp,1,uart,1,1,hello,60,43.139.170.206,1000,0,0,0,0,0,0,0,0"));
    QCOMPARE(gucds::AtProtocol::buildDtuSocketConfig(
                 QStringLiteral("http"), 1, QStringLiteral("43.139.170.206"), 1000),
             QString());
    QCOMPARE(gucds::AtProtocol::buildDtuHttpConfig(
                 1,
                 QStringLiteral("http://43.139.170.206"),
                 1000,
                 QStringLiteral("/c8cacc2e3fba1484feeaace444018e4a")),
             QStringLiteral("config,set,http,1,uart,0,http://43.139.170.206,1000,/c8cacc2e3fba1484feeaace444018e4a,30,0,0,0,0,0"));
    httpConfig.host = QStringLiteral("http://example.com");
    httpConfig.path = QStringLiteral("/upload");
    httpConfig.customHeaderEnabled = true;
    httpConfig.customHeader = QStringLiteral("Content-Type=application/json/0d/0a");
    httpConfig.responseFilterEnabled = true;
    httpConfig.registrationType = 3;
    httpConfig.registrationData = QStringLiteral("REG");
    httpConfig.ipv6 = true;
    QCOMPARE(gucds::AtProtocol::buildDtuHttpConfig(httpConfig),
             QStringLiteral("config,set,http,1,uart,1,http://example.com,80,/upload,30,1,Content-Type=application/json/0d/0a,1,3,REG,1,0"));
    mqttConfig.host = QStringLiteral("43.139.170.206");
    mqttConfig.port = 1002;
    mqttConfig.clientId = QStringLiteral("518d41f0b635211f9f639aa596c9bf39");
    mqttConfig.username = QStringLiteral("518d41f0b635211f9f639aa596c9bf39");
    mqttConfig.password = QStringLiteral("88888888");
    mqttConfig.subscribeTopic = QStringLiteral("518d41f0b635211f9f639aa596c9bf39/down");
    mqttConfig.publishTopic = QStringLiteral("518d41f0b635211f9f639aa596c9bf39/up");
    QCOMPARE(gucds::AtProtocol::buildDtuMqttConfig(mqttConfig),
             QStringLiteral("config,set,mqtt,1,uart,120,43.139.170.206,1002,518d41f0b635211f9f639aa596c9bf39,518d41f0b635211f9f639aa596c9bf39,88888888,1,1,0,0,0,518d41f0b635211f9f639aa596c9bf39/down,518d41f0b635211f9f639aa596c9bf39/up,0,0,0,0,0,0,0,0,0"));
    gucds::DtuMqttConfig mqttAdvanced = mqttConfig;
    mqttAdvanced.willEnabled = true;
    mqttAdvanced.willQos = 1;
    mqttAdvanced.willRetain = true;
    mqttAdvanced.willTopic = QStringLiteral("/will");
    mqttAdvanced.willPayload = QStringLiteral("offline");
    mqttAdvanced.registrationType = 3;
    mqttAdvanced.registrationData = QStringLiteral("REG");
    QCOMPARE(gucds::AtProtocol::buildDtuMqttConfig(mqttAdvanced),
             QStringLiteral("config,set,mqtt,1,uart,120,43.139.170.206,1002,518d41f0b635211f9f639aa596c9bf39,518d41f0b635211f9f639aa596c9bf39,88888888,1,1,0,0,0,518d41f0b635211f9f639aa596c9bf39/down,518d41f0b635211f9f639aa596c9bf39/up,1,1,1,/will,offline,3,REG,0,0"));
    mqttConfig.publishTopic.clear();
    QVERIFY(gucds::AtProtocol::buildDtuMqttConfig(mqttConfig).isEmpty());
    mqttConfig.publishTopic = QString(220, QLatin1Char('x'));
    QVERIFY(gucds::AtProtocol::buildDtuMqttConfig(mqttConfig).isEmpty());
    socketConfig.protocol = QStringLiteral("tcp");
    socketConfig.serialChannel = QStringLiteral("ttluart");
    socketConfig.heartbeatDataType = 1;
    socketConfig.heartbeatData = QStringLiteral("hello");
    socketConfig.host = QStringLiteral("47.106.167.188");
    socketConfig.port = 80;
    QCOMPARE(gucds::AtProtocol::buildDtuSocketConfig(socketConfig),
             QStringLiteral("config,set,tcp,1,ttluart,1,1,hello,60,47.106.167.188,80,0,0,0,0,0,0,0,0"));
    gucds::DtuSocketConfig socketAdvanced = socketConfig;
    socketAdvanced.prefixType = 2;
    socketAdvanced.prefixData = QStringLiteral("AA");
    socketAdvanced.suffixType = 3;
    socketAdvanced.suffixData = QStringLiteral("END");
    socketAdvanced.registrationType = 3;
    socketAdvanced.registrationData = QStringLiteral("REG");
    socketAdvanced.ipv6 = true;
    socketAdvanced.sslMode = 1;
    QCOMPARE(gucds::AtProtocol::buildDtuSocketConfig(socketAdvanced),
             QStringLiteral("config,set,tcp,1,ttluart,1,1,hello,60,47.106.167.188,80,2,AA,3,END,3,REG,1,1"));
    socketConfig.protocol = QStringLiteral("udp");
    QCOMPARE(gucds::AtProtocol::buildDtuSocketConfig(socketConfig),
             QStringLiteral("config,set,udp,1,ttluart,1,1,hello,60,47.106.167.188,80,0,0,0,0,0,0,0,0"));
    webSocketConfig.serverUrl = QStringLiteral("ws://example.com/socket");
    QCOMPARE(gucds::AtProtocol::buildDtuWebSocketConfig(webSocketConfig),
             QStringLiteral("config,set,webs,1,uart,ws://example.com/socket,0,0,0,1,0,00,20,0,0,0,0,0,0"));
    webSocketConfig.serverUrl = QStringLiteral("wss://example.com/socket");
    webSocketConfig.ipv6 = true;
    webSocketConfig.customHeaderEnabled = true;
    webSocketConfig.customHeader = QStringLiteral("Authorization=abc/0d/0a");
    webSocketConfig.heartbeatDataType = 1;
    webSocketConfig.heartbeatData = QStringLiteral("PING");
    webSocketConfig.heartbeatSeconds = 30;
    webSocketConfig.prefixType = 2;
    webSocketConfig.prefixData = QStringLiteral("AA");
    webSocketConfig.suffixType = 3;
    webSocketConfig.suffixData = QStringLiteral("END");
    webSocketConfig.registrationType = 3;
    webSocketConfig.registrationData = QStringLiteral("REG");
    QCOMPARE(gucds::AtProtocol::buildDtuWebSocketConfig(webSocketConfig),
             QStringLiteral("config,set,webs,1,uart,wss://example.com/socket,1,1,Authorization=abc/0d/0a,1,1,PING,30,2,AA,3,END,3,REG"));
    QCOMPARE(gucds::AtProtocol::buildDeleteDtuChannel(1), QStringLiteral("config,get,delnetchan,1"));
    QVERIFY(gucds::AtProtocol::buildDeleteDtuChannel(0).isEmpty());
    QCOMPARE(gucds::AtProtocol::buildSetDeviceParameters({QStringLiteral("1"), QStringLiteral("2")}),
             QStringLiteral("AT,set,devpar,1,2"));
    QCOMPARE(gucds::AtProtocol::buildSetExtendedParameters(
                 {QStringLiteral("0.97"),
                  QStringLiteral("1.101"),
                  QStringLiteral("2.7"),
                  QStringLiteral("1.4"),
                  QStringLiteral("195000"),
                  QStringLiteral("0.156"),
                  QStringLiteral("0")}),
             QStringLiteral("AT,set,exppar,0.97,1.101,2.7,1.4,195000,0.156,0"));
    QCOMPARE(gucds::AtProtocol::buildSetDateTime(
                 QDateTime::fromString(QStringLiteral("2026-07-10 12:34:56"), QStringLiteral("yyyy-MM-dd HH:mm:ss"))),
             QStringLiteral("AT,set,datetime,2026-07-10 12:34:56"));
    QCOMPARE(gucds::AtProtocol::buildBluetoothGet(QStringLiteral("mac")), QStringLiteral("AT,get,ble,mac"));
    QCOMPARE(gucds::AtProtocol::buildBluetoothSet(QStringLiteral("baud"), QStringLiteral("115200")),
             QStringLiteral("AT,set,ble,baud,115200"));
    QCOMPARE(gucds::AtProtocol::buildBluetoothReset(QStringLiteral("factory")),
             QStringLiteral("AT,rst,ble,factory"));
}

void ProtocolTest::parsesFrequencyTensionMeasurement()
{
    gucds::FrequencyTensionSample sample;
    QVERIFY(gucds::AtProtocol::parseFrequencyTensionMeasurement(
        QStringLiteral("MEASTART_OK\n1,Sen05,2208.776,3.906,1.000,0.002"),
        &sample));
    QCOMPARE(sample.cableForceKn, 2208.776);
    QCOMPARE(sample.naturalFrequencyHz, 3.906);
    QCOMPARE(sample.order, 1.0);
    QCOMPARE(sample.convergenceErrorPercent, 0.002);

    QVERIFY(gucds::AtProtocol::parseFrequencyTensionMeasurement(
        QStringLiteral("force=5149.791, fn=5.859, n=1.000, error=0.001"),
        &sample));
    QCOMPARE(sample.cableForceKn, 5149.791);
    QCOMPARE(sample.naturalFrequencyHz, 5.859);
    QCOMPARE(sample.order, 1.0);
    QCOMPARE(sample.convergenceErrorPercent, 0.001);
}

void ProtocolTest::parsesFrequencySpectrum()
{
    QVERIFY(gucds::AtProtocol::parseSpectrum(
                QStringLiteral("SPEC_OK\n0.026884,0.020960,0.140737,0.273781"))
                .isEmpty());
    const QVector<gucds::SpectrumSample> amplitudes = gucds::AtProtocol::parseSpectrum(
        QStringLiteral("SPEC_OK\n0.026884,0.020960,0.140737,0.273781"),
        1.953125);
    QCOMPARE(amplitudes.size(), 4);
    QCOMPARE(amplitudes.at(0).frequencyHz, 0.0);
    QCOMPARE(amplitudes.at(1).frequencyHz, 1.953125);
    QCOMPARE(amplitudes.at(3).amplitude, 0.273781);

    const QVector<gucds::SpectrumSample> wrappedAmplitudes = gucds::AtProtocol::parseSpectrum(
        QStringLiteral("SPEC_OK\n0.026884,0.020960,0.140737,0.273781\n0.131352,0.035720,0.052231,0.027881"),
        1.953125);
    QCOMPARE(wrappedAmplitudes.size(), 8);
    QCOMPARE(wrappedAmplitudes.at(4).frequencyHz, 7.8125);
    QCOMPARE(wrappedAmplitudes.at(7).amplitude, 0.027881);

    const QVector<gucds::SpectrumSample> pairs = gucds::AtProtocol::parseSpectrum(
        QStringLiteral("0.000000,0.026884\n1.953125,0.020960\n3.906250,0.140737"));
    QCOMPARE(pairs.size(), 3);
    QCOMPARE(pairs.at(2).frequencyHz, 3.90625);
    QCOMPARE(pairs.at(2).amplitude, 0.140737);

    const QVector<gucds::SpectrumSample> labeledArrays = gucds::AtProtocol::parseSpectrum(
        QStringLiteral("frequency:0.000000,0.390625,0.781250,1.171875\n"
                       "amplitude:0.011473,-0.010714,0.005257,-0.003923"));
    QCOMPARE(labeledArrays.size(), 4);
    QCOMPARE(labeledArrays.at(1).frequencyHz, 0.390625);
    QCOMPARE(labeledArrays.at(1).amplitude, -0.010714);

    const QVector<gucds::SpectrumSample> interleaved = gucds::AtProtocol::parseSpectrum(
        QStringLiteral("0.000000,0.011473,0.390625,-0.010714,0.781250,0.005257,1.171875,-0.003923"));
    QCOMPARE(interleaved.size(), 4);
    QCOMPARE(interleaved.at(2).frequencyHz, 0.78125);
    QCOMPARE(interleaved.at(3).amplitude, -0.003923);
}

void ProtocolTest::parsesStatusCodes()
{
    const gucds::ParsedStatus ok = gucds::AtProtocol::parseStatus(QStringLiteral("+RESP:DEVPAR_OK"));
    QCOMPARE(ok.code, QStringLiteral("DEVPAR_OK"));
    QVERIFY(ok.success);

    const gucds::ParsedStatus error = gucds::AtProtocol::parseStatus(QStringLiteral("EXPPAR_VALUEERROR"));
    QCOMPARE(error.code, QStringLiteral("EXPPAR_VALUEERROR"));
    QVERIFY(!error.success);

    const gucds::ParsedStatus extendedOk = gucds::AtProtocol::parseStatus(QStringLiteral("set,exppar_OK\r\n"));
    QCOMPARE(extendedOk.code, QStringLiteral("EXPPAR_OK"));
    QVERIFY(extendedOk.success);
    QVERIFY(!gucds::AtProtocol::parseStatus(QStringLiteral("set,exppar_NUMERROR")).success);
    QVERIFY(!gucds::AtProtocol::parseStatus(QStringLiteral("set,exppar_TYPEERROR")).success);
    QVERIFY(!gucds::AtProtocol::parseStatus(QStringLiteral("set,exppar_FLASHERR")).success);

    const gucds::ParsedStatus sampleOk = gucds::AtProtocol::parseStatus(QStringLiteral("set,MEASTART_OK\r\n"));
    QCOMPARE(sampleOk.code, QStringLiteral("MEASTART_OK"));
    QVERIFY(sampleOk.success);

    const gucds::ParsedStatus bluetoothError = gucds::AtProtocol::parseStatus(QStringLiteral("ERR_MODULE,over\r\n"));
    QCOMPARE(bluetoothError.code, QStringLiteral("ERR_MODULE"));
    QVERIFY(!bluetoothError.success);

    const gucds::ParsedStatus unknown = gucds::AtProtocol::parseStatus(QStringLiteral("NO_MATCH"));
    QCOMPARE(unknown.code, QStringLiteral("UNKNOWN"));
    QVERIFY(!unknown.success);

    const gucds::ParsedStatus dtuOk = gucds::AtProtocol::parseStatus(QStringLiteral("config,HTTP_KEY,ok"));
    QCOMPARE(dtuOk.code, QStringLiteral("DTU_CONFIG_OK"));
    QVERIFY(dtuOk.success);

    const gucds::ParsedStatus dtuError = gucds::AtProtocol::parseStatus(QStringLiteral("config,HTTP_KEY,error,1"));
    QCOMPARE(dtuError.code, QStringLiteral("DTU_CONFIG_ERROR"));
    QVERIFY(!dtuError.success);

    bool streamFinished = false;
    const QVector<gucds::SampleStreamPoint> points = gucds::AtProtocol::parseSampleStream(
        QStringLiteral("1,0.001,0.002,1.000,0.120,-0.340;2,0.003,0.004,0.999,0.125,-0.330;!"),
        &streamFinished);
    QVERIFY(streamFinished);
    QCOMPARE(points.size(), 2);
    QCOMPARE(points.at(0).sequence, 1);
    QVERIFY(qAbs(points.at(0).pitch - 0.120f) < 0.0001f);
    QVERIFY(qAbs(points.at(1).roll + 0.330f) < 0.0001f);
}

void ProtocolTest::parsesL051LoraMeasurementFrame()
{
    // L051 LoRa firmware emits 01 04 10 plus four little-endian floats and
    // serializes CRC16 as high byte followed by low byte.
    const QByteArray payload = QByteArray::fromHex(
        "010410"
        "0000C03F"  // pitch: 1.5
        "000010C0"  // roll: -2.25
        "0000003F"  // error: 0.5
        "0000C841"  // temperature: 25.0
    );
    const quint16 crc = gucds::VirtualModbusClient::crc(payload);
    QByteArray l051Frame = payload;
    l051Frame.append(char(crc >> 8));
    l051Frame.append(char(crc & 0xFF));
    QVERIFY(gucds::VirtualModbusClient::hasValidCrc(l051Frame));
    QCOMPARE(l051Frame.size(), 21);

    const gucds::SensorModbusSample sample = gucds::VirtualModbusClient::parseMeasurementResponse(l051Frame);
    QCOMPARE(sample.pitch, 1.5f);
    QCOMPARE(sample.roll, -2.25f);
    QCOMPARE(sample.error, 0.5f);
    QCOMPARE(sample.temperature, 25.0f);

    QByteArray standardModbusFrame = payload;
    standardModbusFrame.append(char(crc & 0xFF));
    standardModbusFrame.append(char(crc >> 8));
    QVERIFY(gucds::VirtualModbusClient::hasValidCrc(standardModbusFrame));
}

void ProtocolTest::readsDelimitedTable()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("table.txt"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QString::fromUtf8("名称\t值\n波特率\t9600\n").toUtf8());
    file.close();

    QString error;
    const gucds::DelimitedTable::Rows rows = gucds::DelimitedTable::read(path, &error, QByteArrayLiteral("UTF-8"));
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(rows.size(), 2);
    QCOMPARE(rows.at(0).at(0), QStringLiteral("名称"));
    QCOMPARE(rows.at(1).at(1), QStringLiteral("9600"));
}

void ProtocolTest::virtualDeviceScansDevices()
{
    gucds::VirtualDevice device;
    const QVector<gucds::DeviceRecord> devices = device.scanDevices();
    QCOMPARE(devices.size(), 3);
    QCOMPARE(devices.at(0).name, QStringLiteral("Sen01"));
    QCOMPARE(devices.at(1).protocol, QStringLiteral("TTL/AT"));
}

void ProtocolTest::virtualDeviceMeasures()
{
    gucds::VirtualDevice device;
    const gucds::VirtualDeviceReply reply = device.transact(gucds::AtProtocol::command(gucds::AtCommand::MeasureStart));
    QVERIFY(reply.success);
    QCOMPARE(reply.payload, QStringLiteral("MEASURE_OK"));
    QCOMPARE(reply.measurement.index, 1);
    QCOMPARE(reply.measurement.deviceName, QStringLiteral("Sen01"));
    QVERIFY(reply.measurement.timestamp.isValid());
    QVERIFY(reply.measurement.value > 499.0);
}

void ProtocolTest::virtualDeviceReadsSpectrum()
{
    gucds::VirtualDevice device;
    const gucds::VirtualDeviceReply reply = device.transact(gucds::AtProtocol::command(gucds::AtCommand::GetSpectrum));
    QVERIFY(reply.success);
    QCOMPARE(reply.payload, QStringLiteral("SPEC_OK"));
    QVERIFY(reply.spectrum.size() >= 10);
    QVERIFY(reply.spectrum.at(5) > reply.spectrum.at(0));
}

void ProtocolTest::virtualDeviceStoresDtuConfig()
{
    gucds::VirtualDevice device;
    const gucds::VirtualDeviceReply writeReply =
        device.transact(gucds::AtProtocol::buildConfigSet(QStringLiteral("MQTT_KEY"), QStringLiteral("mqtt://example")));
    QVERIFY(writeReply.success);
    QCOMPARE(writeReply.payload, QStringLiteral("DEVPAR_OK"));
    QCOMPARE(device.configValue(QStringLiteral("MQTT_KEY")), QStringLiteral("mqtt://example"));

    const gucds::VirtualDeviceReply saveReply = device.transact(gucds::AtProtocol::command(gucds::AtCommand::ConfigSave));
    QVERIFY(saveReply.success);
    QCOMPARE(saveReply.payload, QStringLiteral("CONFIG_SAVE_OK"));
}

void ProtocolTest::virtualTransportRejectsUnknownCommand()
{
    gucds::VirtualTransport transport;
    const gucds::VirtualDeviceReply reply = transport.transact(QStringLiteral("AT,unknown"));
    QVERIFY(!reply.success);
    QCOMPARE(reply.payload, QStringLiteral("AT_PARSINGFAILED"));
}

void ProtocolTest::deviceTableModelMutates()
{
    gucds::DeviceTableModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("设备名称"));

    gucds::DeviceRecord tilt;
    tilt.name = QStringLiteral("倾角传感器");
    tilt.category = QStringLiteral("姿态传感器");
    tilt.model = QStringLiteral("QL-F405-CHP");

    gucds::DeviceRecord flux;
    flux.name = QStringLiteral("磁通量传感器");
    flux.category = QStringLiteral("磁通量传感器");
    flux.model = QStringLiteral("QL-CMFS-D120");

    model.addRecord(tilt);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toString(), QStringLiteral("倾角传感器"));
    QVERIFY(model.containsEquivalent(tilt));

    model.addRecord(flux);
    QCOMPARE(model.rowCount(), 2);
    QVERIFY(model.moveRecord(1, 0));
    QCOMPARE(model.recordAt(0).model, QStringLiteral("QL-CMFS-D120"));
    QCOMPARE(model.recordAt(1).model, QStringLiteral("QL-F405-CHP"));

    QVERIFY(!model.moveRecord(2, 0));
    QVERIFY(model.removeRecordAt(1));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.recordAt(0).name, QStringLiteral("磁通量传感器"));

    model.clear();
    QCOMPARE(model.rowCount(), 0);
}

void ProtocolTest::deviceParameterDefinitionsRoundTrip()
{
    const QString text = QStringLiteral("量程,菜单,整数:±2g,±4g,±8g,±16g");
    const gucds::DeviceParameterDefinition definition = gucds::parseDeviceParameterDefinition(text);
    QVERIFY(definition.isValid());
    QCOMPARE(definition.name, QStringLiteral("量程"));
    QCOMPARE(definition.editorMode, QStringLiteral("菜单"));
    QCOMPARE(definition.valueType, QStringLiteral("整数"));
    QCOMPARE(definition.options(), QStringList({QStringLiteral("±2g"), QStringLiteral("±4g"), QStringLiteral("±8g"), QStringLiteral("±16g")}));
    QCOMPARE(gucds::formatDeviceParameterDefinition(definition), text);

    gucds::DeviceRecord record;
    gucds::setDeviceParameterDefinition(&record, 2, text);
    gucds::setDeviceParameterValue(&record, 2, QStringLiteral("±8g"));
    QCOMPARE(gucds::deviceParameterDefinition(record, 2), text);
    QCOMPARE(gucds::deviceParameterValue(record, 2), QStringLiteral("±8g"));
    QCOMPARE(gucds::formatDeviceParameterDefinition({}), QStringLiteral("未定义"));
}

void ProtocolTest::productTableModelMutates()
{
    gucds::DeviceRecord tilt;
    tilt.databaseId = 11;
    tilt.name = QStringLiteral("倾角传感器");
    tilt.category = QStringLiteral("倾角传感器");
    tilt.model = QStringLiteral("QL-F405-CHP");
    tilt.data1 = QStringLiteral("Pitch(°)");
    tilt.modbus = QStringLiteral("开");
    tilt.parameter1 = QStringLiteral("上阈值,字符,浮点:20.0");
    tilt.calibrationPoints = 3;

    gucds::ProductTableModel model;
    model.setRecords({tilt});
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, gucds::ProductTableModel::CommunicationColumn)).toString(), QStringLiteral("Modbus"));
    QCOMPARE(model.data(model.index(0, gucds::ProductTableModel::ParameterColumn)).toInt(), 1);
    QCOMPARE(model.rowForDatabaseId(11), 0);

    tilt.model = QStringLiteral("QL-F405-CHP-V2");
    QVERIFY(model.updateRecord(tilt));
    QCOMPARE(model.data(model.index(0, gucds::ProductTableModel::ModelColumn)).toString(), QStringLiteral("QL-F405-CHP-V2"));
    QVERIFY(model.removeRecord(11));
    QCOMPARE(model.rowCount(), 0);
}

void ProtocolTest::measurementTableModelCapsRows()
{
    gucds::MeasurementTableModel model;
    for (int index = 0; index < 10005; ++index) {
        gucds::MeasurementRecord record;
        record.index = index + 1;
        record.value = index;
        model.addMeasurement(record);
    }
    QCOMPARE(model.rowCount(), 10000);
    QCOMPARE(model.data(model.index(0, 0)).toString(), QStringLiteral("5.000"));
    QCOMPARE(model.data(model.index(model.rowCount() - 1, 0)).toString(), QStringLiteral("10004.000"));
}

void ProtocolTest::measurementTableModelSwitchesFrequencyTensionViews()
{
    gucds::MeasurementTableModel model;
    model.setMode(gucds::MeasurementTableModel::Mode::FrequencyTension);
    QCOMPARE(model.columnCount(), 7);
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("序号"));
    QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), QStringLiteral("索力(kN)"));
    QCOMPARE(model.headerData(3, Qt::Horizontal).toString(), QStringLiteral("fn(Hz)"));
    QCOMPARE(model.headerData(5, Qt::Horizontal).toString(), QStringLiteral("收敛误差(%)"));

    gucds::FrequencyTensionMeasurementRecord measurement;
    measurement.index = 1;
    measurement.deviceName = QStringLiteral("Sen05");
    measurement.cableForceKn = 2208.776;
    measurement.naturalFrequencyHz = 3.906;
    measurement.order = 1.0;
    measurement.convergenceErrorPercent = 0.002;
    measurement.timestamp = QDateTime::currentDateTime();
    model.addFrequencyTensionMeasurement(measurement);
    measurement.index = 2;
    measurement.deviceName = QStringLiteral("Sen06");
    measurement.cableForceKn = 3295.852;
    model.addFrequencyTensionMeasurement(measurement);
    QCOMPARE(model.rowCount(), 2);
    QVERIFY(model.removeRow(0));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
    QCOMPARE(model.data(model.index(0, 1)).toString(), QStringLiteral("Sen06"));
    QCOMPARE(model.data(model.index(0, 2)).toString(), QStringLiteral("3295.852"));

    QTemporaryDir csvDirectory;
    QVERIFY(csvDirectory.isValid());
    const QString csvPath = csvDirectory.filePath(QStringLiteral("measurements.csv"));
    QString csvError;
    QVERIFY2(model.saveCsv(csvPath, &csvError), qPrintable(csvError));
    QFile csvFile(csvPath);
    QVERIFY(csvFile.open(QIODevice::ReadOnly));
    const QByteArray csv = csvFile.readAll();
    QVERIFY(csv.startsWith(QByteArray::fromHex("EFBBBF")));
    QVERIFY(csv.contains(QStringLiteral("序号,设备名,索力(kN),fn(Hz),n(阶数),收敛误差(%),日期时间")
                             .toUtf8()));
    QVERIFY(csv.contains(QByteArrayLiteral("1,Sen06,3295.852,3.906,1.000,0.002")));
    QVERIFY(!model.saveCsv({}, &csvError));
    QVERIFY(!csvError.isEmpty());

    model.setSpectrumPoints({{1, 0.0, 0.026884}, {2, 1.953125, 0.020960}});
    QCOMPARE(model.mode(), gucds::MeasurementTableModel::Mode::Spectrum);
    QCOMPARE(model.columnCount(), 3);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QStringLiteral("频率(Hz)"));
    QCOMPARE(model.data(model.index(1, 1)).toString(), QStringLiteral("1.953125"));
    QVERIFY(model.removeRow(0));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toInt(), 1);
}

void ProtocolTest::communicationControllerReportsOpenFailure()
{
    gucds::DeviceCommunicationController controller;
    controller.configure(QStringLiteral("COM9999"), 9600, 1, gucds::DeviceWireProtocol::Text);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);
    QVERIFY(controller.sendTextCommand(QStringLiteral("AT,get,mcupar"), QStringLiteral("failure_test")));
    QVERIFY(controller.isRunning());
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 3000);
    const gucds::CommunicationResult result = qvariant_cast<gucds::CommunicationResult>(
        resultSpy.takeFirst().at(0));
    QVERIFY(!result.success);
    QCOMPARE(result.context, QStringLiteral("failure_test"));
    QVERIFY(!result.message.isEmpty());
    QVERIFY(!controller.isRunning());
}

void ProtocolTest::calibrationTableModelMutates()
{
    gucds::CalibrationTableModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QStringLiteral("曲线名"));

    model.addPoint({4, 500.125, 1000.000, 26.0, QDateTime::fromString(QStringLiteral("2026/7/2 09:30:00"), QStringLiteral("yyyy/M/d HH:mm:ss")), QStringLiteral("磁通量D120")});
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0)).toString(), QStringLiteral("磁通量D120"));
    QCOMPARE(model.data(model.index(0, 2)).toString(), QStringLiteral("500.125"));

    gucds::CalibrationRecord updated = model.recordAt(0);
    updated.measuredValue = 600.5;
    QVERIFY(model.updatePoint(0, updated));
    QCOMPARE(model.data(model.index(0, 2)).toString(), QStringLiteral("600.500"));
    QVERIFY(!model.updatePoint(2, updated));
    QVERIFY(model.removePoint(0));
    QCOMPARE(model.rowCount(), 0);

    model.removeLast();
    QCOMPARE(model.rowCount(), 0);
    model.clear();
    QCOMPARE(model.rowCount(), 0);
}

void ProtocolTest::busDeviceTableModelUpdatesResponse()
{
    gucds::BusDeviceTableModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.headerData(7, Qt::Horizontal).toString(), QStringLiteral("应答码"));

    model.addDevice({4, QStringLiteral("Sen12"), QStringLiteral("QL-VIRTUAL"), 25, 1, 12, 2, QStringLiteral("WAIT")});
    QCOMPARE(model.rowCount(), 1);
    model.updateLastResponse(QStringLiteral("secparaOK"));
    QCOMPARE(model.data(model.index(0, 7)).toString(), QStringLiteral("secparaOK"));
    gucds::BusDeviceRecord updated = model.recordAt(0);
    updated.address = 22;
    QVERIFY(model.updateDevice(0, updated));
    QCOMPARE(model.data(model.index(0, 5)).toInt(), 22);
    QVERIFY(model.removeDevice(0));
    QCOMPARE(model.rowCount(), 0);
}

void ProtocolTest::labviewDatabaseImportsExistingData()
{
    const QString labviewRoot = findLabviewRootForTest();
    QVERIFY2(!labviewRoot.isEmpty(), "LabVIEW project data directory was not found");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString sqlitePath = dir.filePath(QStringLiteral("gucds.sqlite"));

    QString errorMessage;
    QVERIFY2(gucds::LabviewDatabase::importFromLabviewProject(labviewRoot, sqlitePath, &errorMessage),
             qPrintable(errorMessage));

    const QVector<gucds::DeviceRecord> devices = gucds::LabviewDatabase::loadDeviceRecords(sqlitePath, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(devices.size() >= 140);
    QVERIFY(devices.first().databaseId > 0);
    QVERIFY(std::any_of(devices.cbegin(), devices.cend(), [](const gucds::DeviceRecord &record) {
        return record.model == QStringLiteral("QL-SPS-WNDUG-1") && record.category == QStringLiteral("定位(卫星)传感器");
    }));

    const QVector<gucds::CalibrationRecord> calibrations = gucds::LabviewDatabase::loadCalibrationRecords(sqlitePath, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(calibrations.size(), 67);
    QVERIFY(std::any_of(calibrations.cbegin(), calibrations.cend(), [](const gucds::CalibrationRecord &record) {
        return record.curveName == QStringLiteral("磁通量D120")
            && record.point == 1
            && qFuzzyCompare(record.measuredValue, 300.3);
    }));

    const QVector<gucds::BusDeviceRecord> busDevices = gucds::LabviewDatabase::loadBusDeviceRecords(sqlitePath, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(busDevices.size(), 10);
    QVERIFY(std::any_of(busDevices.cbegin(), busDevices.cend(), [](const gucds::BusDeviceRecord &record) {
        return record.sensorName == QStringLiteral("测斜传感器")
            && record.model == QStringLiteral("QL-ATIS-RxDP-Mx")
            && record.address == 10;
    }));

    QCOMPARE(sqliteCount(sqlitePath, QStringLiteral("bus_device_records"), &errorMessage), 10);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(sqliteCount(sqlitePath, QStringLiteral("calibration_records"), &errorMessage), 67);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    gucds::CalibrationRecord managedCalibration;
    managedCalibration.curveName = QStringLiteral("自动化标定");
    managedCalibration.point = 1;
    managedCalibration.measuredValue = 12.5;
    managedCalibration.referenceValue = 13.0;
    managedCalibration.temperature = 25.0;
    managedCalibration.timestamp = QDateTime::currentDateTime();
    QVERIFY2(gucds::LabviewDatabase::saveCalibrationRecord(sqlitePath, &managedCalibration, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(managedCalibration.databaseId > 0);
    managedCalibration.measuredValue = 12.75;
    QVERIFY2(gucds::LabviewDatabase::saveCalibrationRecord(sqlitePath, &managedCalibration, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY2(gucds::LabviewDatabase::deleteCalibrationRecord(sqlitePath, managedCalibration.databaseId, &errorMessage),
             qPrintable(errorMessage));
    QCOMPARE(sqliteCount(sqlitePath, QStringLiteral("calibration_records"), &errorMessage), 67);

    gucds::BusDeviceRecord managedBus;
    managedBus.index = 99;
    managedBus.sensorName = QStringLiteral("自动化总线设备");
    managedBus.model = QStringLiteral("QL-TEST-BUS");
    managedBus.channel = 20;
    managedBus.group = 1;
    managedBus.address = 99;
    managedBus.dataCount = 2;
    QVERIFY2(gucds::LabviewDatabase::saveBusDeviceRecord(sqlitePath, &managedBus, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(managedBus.databaseId > 0);
    managedBus.responseCode = QStringLiteral("OK");
    QVERIFY2(gucds::LabviewDatabase::saveBusDeviceRecord(sqlitePath, &managedBus, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY2(gucds::LabviewDatabase::deleteBusDeviceRecord(sqlitePath, managedBus.databaseId, &errorMessage),
             qPrintable(errorMessage));
    QCOMPARE(sqliteCount(sqlitePath, QStringLiteral("bus_device_records"), &errorMessage), 10);
    QVERIFY(sqliteCount(sqlitePath, QStringLiteral("device_records"), &errorMessage) >= 140);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(sqliteCount(sqlitePath, QStringLiteral("labview_table_cells"), &errorMessage) > 40);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(sqliteCount(sqlitePath, QStringLiteral("tdms_cells"), &errorMessage) > 700);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    const int importedDeviceCount = sqliteCount(sqlitePath, QStringLiteral("device_records"), &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    gucds::DeviceRecord managedProduct;
    managedProduct.name = QStringLiteral("测试产品");
    managedProduct.category = QStringLiteral("自动化测试");
    managedProduct.model = QStringLiteral("QL-TEST-PRODUCT-001");
    managedProduct.data1 = QStringLiteral("Pitch(°)");
    managedProduct.modbus = QStringLiteral("开");
    managedProduct.parameter1 = QStringLiteral("量程,菜单,整数:±2g,±4g");
    managedProduct.calibrationPoints = 2;
    QVERIFY2(gucds::LabviewDatabase::saveDeviceRecord(sqlitePath, &managedProduct, &errorMessage),
             qPrintable(errorMessage));
    QVERIFY(managedProduct.databaseId > 0);
    QCOMPARE(sqliteCount(sqlitePath, QStringLiteral("device_records"), &errorMessage), importedDeviceCount + 1);

    managedProduct.data1 = QStringLiteral("PitchUpdated(°)");
    QVERIFY2(gucds::LabviewDatabase::saveDeviceRecord(sqlitePath, &managedProduct, &errorMessage),
             qPrintable(errorMessage));
    QCOMPARE(sqliteCount(sqlitePath, QStringLiteral("device_records"), &errorMessage), importedDeviceCount + 1);
    const QVector<gucds::DeviceRecord> updatedDevices = gucds::LabviewDatabase::loadDeviceRecords(sqlitePath, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(std::any_of(updatedDevices.cbegin(), updatedDevices.cend(), [&managedProduct](const gucds::DeviceRecord &record) {
        return record.databaseId == managedProduct.databaseId && record.data1 == QStringLiteral("PitchUpdated(°)");
    }));

    gucds::DeviceRecord duplicateProduct = managedProduct;
    duplicateProduct.databaseId = -1;
    duplicateProduct.model = duplicateProduct.model.toLower();
    QVERIFY(!gucds::LabviewDatabase::saveDeviceRecord(sqlitePath, &duplicateProduct, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("已经存在")));

    QVERIFY2(gucds::LabviewDatabase::deleteDeviceRecord(sqlitePath, managedProduct.databaseId, &errorMessage),
             qPrintable(errorMessage));
    QCOMPARE(sqliteCount(sqlitePath, QStringLiteral("device_records"), &errorMessage), importedDeviceCount);
}

void ProtocolTest::serialSessionTransitions()
{
    gucds::SerialSession session;
    QCOMPARE(session.statusText(), QStringLiteral("未连接"));

    QString error;
    QVERIFY(!session.open(QString(), 9600, &error));
    QVERIFY(!error.isEmpty());

    QVERIFY(session.isBackendAvailable());
    const QStringList ports = gucds::SerialSession::availablePorts();
    QVERIFY(std::all_of(ports.cbegin(),
                        ports.cend(),
                        [](const QString &port) { return !port.isEmpty(); }));

    QVERIFY(!session.open(QStringLiteral("COM9999"), 115200, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!session.isConnected());
    QCOMPARE(session.portName(), QStringLiteral("COM9999"));
    QCOMPARE(session.baudRate(), 115200);
    QCOMPARE(session.statusText(), QStringLiteral("未连接"));

    session.close();
    QVERIFY(!session.isConnected());
}

void ProtocolTest::serialSessionCom0ComLoopback()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the com0com serial smoke test.");

    gucds::SerialSession pcSession;
    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QVERIFY2(pcSession.open(pcPort, 9600, &error), qPrintable(error));

    const QByteArray frame = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));

    QByteArray sensorRx;
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        if (!sensorSession.readFrame(frame.size(), &sensorRx, &sensorError, 2000))
            return;
        if (sensorRx != frame) {
            sensorError = QStringLiteral("Sensor side received unexpected frame: %1")
                              .arg(gucds::VirtualModbusClient::formatHex(sensorRx));
            return;
        }
        sensorSession.writeFrame(sensorRx, &sensorError);
    });

    sensorThread->start();
    QByteArray pcRx;
    const bool pcOk = pcSession.transactFrame(frame, frame.size(), &pcRx, &error, 2000);
    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;

    QVERIFY(sensorFinished);
    QVERIFY2(pcOk, qPrintable(error));
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
    QCOMPARE(sensorRx, frame);
    QCOMPARE(pcRx, frame);
}

void ProtocolTest::serialSessionCom0ComTextTransaction()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the com0com text transaction test.");

    gucds::SerialSession pcSession;
    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QVERIFY2(pcSession.open(pcPort, 9600, &error), qPrintable(error));

    const QByteArray expectedRequest = QByteArrayLiteral("AT,get,mcupar\r\n");
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        QByteArray request;
        if (!sensorSession.readFrame(expectedRequest.size(), &request, &sensorError, 2000))
            return;
        if (request != expectedRequest) {
            sensorError = QStringLiteral("Unexpected text request: %1").arg(QString::fromUtf8(request));
            return;
        }
        sensorSession.writeFrame(QByteArrayLiteral("get,mcupar,9600,1,0,1,1,over\r\n"), &sensorError);
    });
    sensorThread->start();

    QByteArray response;
    const bool ok = pcSession.transactText(QStringLiteral("AT,get,mcupar"), &response, &error, 2000, 80);
    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;

    QVERIFY(sensorFinished);
    QVERIFY2(ok, qPrintable(error));
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
    QCOMPARE(response, QByteArrayLiteral("get,mcupar,9600,1,0,1,1,over\r\n"));
}

void ProtocolTest::deviceCommunicationControllerCom0ComExtendedParameters()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the extended-parameter controller test.");

    const QByteArray setRequest = QByteArrayLiteral(
        "AT,set,exppar,0.97,1.101,2.7,1.4,195000,0.156,0\r\n");
    const QByteArray getRequest = QByteArrayLiteral("AT,get,FVCFexppar\r\n");

    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        QByteArray request;
        if (!sensorSession.readFrame(setRequest.size(), &request, &sensorError, 3000))
            return;
        if (request != setRequest) {
            sensorError = QStringLiteral("Unexpected extended-parameter set request: %1")
                              .arg(QString::fromUtf8(request));
            return;
        }
        if (!sensorSession.writeFrame(QByteArrayLiteral("set,exppar_OK\r\n"), &sensorError))
            return;

        request.clear();
        if (!sensorSession.readFrame(getRequest.size(), &request, &sensorError, 3000))
            return;
        if (request != getRequest) {
            sensorError = QStringLiteral("Unexpected extended-parameter get request: %1")
                              .arg(QString::fromUtf8(request));
            return;
        }
        sensorSession.writeFrame(
            QByteArrayLiteral("get,FVCFexppar,0.97,1.101,2.7,1.4,195000,0.156,0,over\r\n"),
            &sensorError);
    });
    sensorThread->start();

    gucds::DeviceCommunicationController controller;
    controller.configure(pcPort, 9600, 1, gucds::DeviceWireProtocol::Text);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);
    QVERIFY(controller.sendTextCommands(
        {QString::fromUtf8(setRequest).trimmed(), QString::fromUtf8(getRequest).trimmed()},
        QStringLiteral("frequency_parameters_write"),
        200));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 8000);
    const gucds::CommunicationResult result = qvariant_cast<gucds::CommunicationResult>(
        resultSpy.takeFirst().at(0));
    QVERIFY2(result.success, qPrintable(result.message));
    QCOMPARE(result.numericValues.size(), 7);
    QCOMPARE(result.numericValues.at(0), 0.97);
    QCOMPARE(result.numericValues.at(4), 195000.0);
    QCOMPARE(result.numericValues.at(6), 0.0);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::frequencyMcuConfigurationFrames()
{
    const QByteArray mcuBytes = QByteArray::fromHex("80002500000001000101");
    gucds::DeviceCommunicationController controller;
    controller.configure(QStringLiteral("COM_INVALID"), 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QVERIFY(!controller.writeFrequencyMcuParameters(QByteArray(8, char(0)), 4, 0));
    QVERIFY(!controller.writeFrequencyMcuParameters(mcuBytes, 11, 0));
    QVERIFY(!controller.writeFrequencyMcuParameters(mcuBytes, 4, 4));

    const QByteArray samplingBytes = QByteArray::fromHex("0000000400000000");
    const QByteArray frame = gucds::VirtualModbusClient::buildWriteMultipleHoldingRegisters(
        1,
        0x0012,
        samplingBytes);
    QCOMPARE(frame, QByteArray::fromHex("0110001200040800000004000000007f82"));
}

void ProtocolTest::frequencyHardwareSameValueRoundTrip()
{
    const QString port = qEnvironmentVariable("QLIOT_HARDWARE_PORT");
    if (port.isEmpty())
        QSKIP("Set QLIOT_HARDWARE_PORT to run the same-value frequency/tension hardware test.");

    gucds::DeviceCommunicationController controller;
    controller.configure(port, 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);

    QVERIFY(controller.sendTextCommands(
        {gucds::AtProtocol::command(gucds::AtCommand::GetMcuParameters),
         gucds::AtProtocol::command(gucds::AtCommand::GetSensorParameters)},
        QStringLiteral("hardware_read_configuration"),
        100));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 10000);
    gucds::CommunicationResult configuration =
        qvariant_cast<gucds::CommunicationResult>(resultSpy.takeFirst().at(0));
    QVERIFY2(configuration.success, qPrintable(configuration.message));
    QCOMPARE(configuration.numericValues.size(), 11);
    QCOMPARE(qRound(configuration.numericValues.at(2)), 1); // Automatic mode.
    const int sampleRateIndex = qRound(configuration.numericValues.at(7));
    const int samplePointIndex = qRound(configuration.numericValues.at(8));
    QVERIFY(sampleRateIndex >= 0 && sampleRateIndex <= 10);
    QVERIFY(samplePointIndex >= 0 && samplePointIndex <= 3);
    qInfo().noquote() << QStringLiteral("COM14 readback: baud=%1 mode=%2 rateIndex=%3 pointIndex=%4")
                            .arg(qRound64(configuration.numericValues.at(0)))
                            .arg(qRound(configuration.numericValues.at(2)))
                            .arg(sampleRateIndex)
                            .arg(samplePointIndex);

    const quint32 baud = quint32(qRound64(configuration.numericValues.at(0)));
    const quint16 gap = quint16(qRound(configuration.numericValues.at(1)));
    QByteArray mcuBytes = gucds::VirtualModbusClient::encodeMcuBaud(baud);
    mcuBytes.append(char((gap >> 8) & 0xFF));
    mcuBytes.append(char(gap & 0xFF));
    mcuBytes.append(char(qRound(configuration.numericValues.at(2))));
    mcuBytes.append(char(0));
    mcuBytes.append(char(qRound(configuration.numericValues.at(3))));
    mcuBytes.append(char(qRound(configuration.numericValues.at(4))));

    QVERIFY(controller.writeFrequencyMcuParameters(
        mcuBytes,
        quint8(sampleRateIndex),
        quint8(samplePointIndex),
        QStringLiteral("hardware_write_configuration")));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 15000);
    const gucds::CommunicationResult writeConfiguration =
        qvariant_cast<gucds::CommunicationResult>(resultSpy.takeFirst().at(0));
    QVERIFY2(writeConfiguration.success, qPrintable(writeConfiguration.message));
    QCOMPARE(qRound(writeConfiguration.numericValues.at(7)), sampleRateIndex);
    QCOMPARE(qRound(writeConfiguration.numericValues.at(8)), samplePointIndex);
    qInfo().noquote() << QStringLiteral("COM14 MCU/sampling same-value write and readback: %1")
                            .arg(writeConfiguration.message);

    QVERIFY(controller.sendTextCommand(
        gucds::AtProtocol::command(gucds::AtCommand::GetFrequencyParameters),
        QStringLiteral("hardware_read_extended_parameters")));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 10000);
    const gucds::CommunicationResult extended =
        qvariant_cast<gucds::CommunicationResult>(resultSpy.takeFirst().at(0));
    QVERIFY2(extended.success, qPrintable(extended.message));
    QCOMPARE(extended.numericValues.size(), 7);
    QStringList extendedArguments;
    for (const double value : extended.numericValues)
        extendedArguments.append(QString::number(value, 'g', 12));

    QVERIFY(controller.sendTextCommands(
        {gucds::AtProtocol::buildSetExtendedParameters(extendedArguments),
         gucds::AtProtocol::command(gucds::AtCommand::GetFrequencyParameters)},
        QStringLiteral("hardware_write_extended_parameters"),
        200));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 10000);
    const gucds::CommunicationResult writeExtended =
        qvariant_cast<gucds::CommunicationResult>(resultSpy.takeFirst().at(0));
    QVERIFY2(writeExtended.success, qPrintable(writeExtended.message));
    QCOMPARE(writeExtended.numericValues.size(), 7);
    for (int index = 0; index < extended.numericValues.size(); ++index)
        QVERIFY(qAbs(writeExtended.numericValues.at(index) - extended.numericValues.at(index)) <= 0.01);
    qInfo().noquote() << QStringLiteral("COM14 extended-parameter same-value write/readback: %1")
                            .arg(writeExtended.responseText.simplified());

    QSignalSpy sampleSpy(&controller, &gucds::DeviceCommunicationController::frequencyTensionSampleReady);
    QVERIFY(controller.measureFrequencyTension(QStringLiteral("hardware_automatic_measurement")));
    QElapsedTimer measurementWait;
    measurementWait.start();
    while (sampleSpy.isEmpty() && resultSpy.isEmpty() && measurementWait.elapsed() < 30000)
        QTest::qWait(100);
    if (controller.isRunning()) {
        controller.cancel();
        QTRY_VERIFY_WITH_TIMEOUT(!controller.isRunning(), 3000);
    }
    if (!sampleSpy.isEmpty()) {
        const gucds::CommunicationResult measurement =
            qvariant_cast<gucds::CommunicationResult>(sampleSpy.first().at(0));
        QVERIFY(measurement.success);
        QVERIFY(measurement.hasSample);
        QVERIFY(measurement.responseText.contains(
            QStringLiteral("MODE: AUTO (CONTINUOUS DIRECT DATA READ)")));
        QVERIFY(!measurement.responseText.contains(QStringLiteral("STATUS RX:")));
    } else {
        qInfo().noquote() << QStringLiteral(
            "COM14 automatic mode produced no completed FF->A1 cycle within 30 seconds; "
            "the continuous session was stopped normally.");
    }
}

void ProtocolTest::deviceCommunicationControllerCom0ComFrequencyMcuWrite()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the frequency MCU write test.");

    const QByteArray mcuBytes = QByteArray::fromHex("80002500000001000101");
    const QByteArray samplingBytes = QByteArray::fromHex("0000000400000000");
    const QByteArray mcuWrite = gucds::VirtualModbusClient::buildWriteMultipleHoldingRegisters(
        1,
        0x0002,
        mcuBytes);
    const QByteArray samplingWrite = gucds::VirtualModbusClient::buildWriteMultipleHoldingRegisters(
        1,
        0x0012,
        samplingBytes);
    const QByteArray saveRequest = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::SaveMcuParameters));
    const QByteArray getMcuRequest = QByteArrayLiteral("AT,get,mcupar\r\n");
    const QByteArray getSensorRequest = QByteArrayLiteral("AT,get,senpar\r\n");

    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        gucds::VirtualModbusClient sensor;
        const QList<QByteArray> modbusRequests = {mcuWrite, samplingWrite, saveRequest};
        for (const QByteArray &expected : modbusRequests) {
            QByteArray request;
            if (!sensorSession.readFrame(expected.size(), &request, &sensorError, 3000))
                return;
            if (request != expected) {
                sensorError = QStringLiteral("Unexpected frequency MCU frame: %1")
                                  .arg(gucds::VirtualModbusClient::formatHex(request));
                return;
            }
            QByteArray response;
            if (!sensor.transactFrame(request, &response)) {
                sensorError = sensor.lastFrame();
                return;
            }
            if (!sensorSession.writeFrame(response, &sensorError))
                return;
        }

        QByteArray request;
        if (!sensorSession.readFrame(getMcuRequest.size(), &request, &sensorError, 3000))
            return;
        if (request != getMcuRequest) {
            sensorError = QStringLiteral("Unexpected MCU readback request");
            return;
        }
        if (!sensorSession.writeFrame(
                QByteArrayLiteral("get,mcupar,9600,0,1,1,1,over\r\n"),
                &sensorError)) {
            return;
        }

        request.clear();
        if (!sensorSession.readFrame(getSensorRequest.size(), &request, &sensorError, 3000))
            return;
        if (request != getSensorRequest) {
            sensorError = QStringLiteral("Unexpected sensor readback request");
            return;
        }
        sensorSession.writeFrame(QByteArrayLiteral("get,senpar,4,0,over\r\n"), &sensorError);
    });
    sensorThread->start();

    gucds::DeviceCommunicationController controller;
    controller.configure(pcPort, 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);
    QVERIFY(controller.writeFrequencyMcuParameters(
        mcuBytes,
        4,
        0,
        QStringLiteral("frequency_mcu_write_test")));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 8000);
    const gucds::CommunicationResult result = qvariant_cast<gucds::CommunicationResult>(
        resultSpy.takeFirst().at(0));
    QVERIFY2(result.success, qPrintable(result.message));
    QCOMPARE(result.numericValues.size(), 9);
    QCOMPARE(result.numericValues.at(2), 1.0);
    QCOMPARE(result.numericValues.at(7), 4.0);
    QCOMPARE(result.numericValues.at(8), 0.0);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::deviceCommunicationControllerCom0ComAutomaticFrequencyMeasurement()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the automatic frequency measurement test.");

    const QByteArray getMcuRequest = QByteArrayLiteral("AT,get,mcupar\r\n");
    const QByteArray getSensorRequest = QByteArrayLiteral("AT,get,senpar\r\n");
    const QByteArray dataRequest = gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 8);
    gucds::VirtualModbusClient completedDevice;
    QByteArray ignored;
    QByteArray dataResponse;
    const QByteArray startRequest = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    QVERIFY(completedDevice.transactFrame(startRequest, &ignored));
    QVERIFY(completedDevice.transactFrame(dataRequest, &dataResponse));

    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        QByteArray request;
        if (!sensorSession.readFrame(getMcuRequest.size(), &request, &sensorError, 3000))
            return;
        if (request != getMcuRequest
            || !sensorSession.writeFrame(
                QByteArrayLiteral("get,mcupar,9600,0,1,1,1,over\r\n"),
                &sensorError)) {
            return;
        }

        request.clear();
        if (!sensorSession.readFrame(getSensorRequest.size(), &request, &sensorError, 3000))
            return;
        if (request != getSensorRequest
            || !sensorSession.writeFrame(
                QByteArrayLiteral("get,senpar,1,0,4,0,0,0,over\r\n"),
                &sensorError)) {
            return;
        }

        const auto receive = [&](const QByteArray &expected, int timeoutMs = 3000) {
            QByteArray actual;
            if (!sensorSession.readFrame(expected.size(), &actual, &sensorError, timeoutMs))
                return false;
            if (actual != expected) {
                sensorError = QStringLiteral("Unexpected automatic-mode frame: %1")
                                  .arg(gucds::VirtualModbusClient::formatHex(actual));
                return false;
            }
            return true;
        };
        const auto replyData = [&] {
            return receive(dataRequest) && sensorSession.writeFrame(dataResponse, &sensorError);
        };

        // Automatic mode reads data at the configured sampling cadence and
        // must not send B1 or status-query frames.
        if (!replyData() || !replyData())
            return;

        // One final pending reply keeps the controller in its continuous loop;
        // cancellation may happen before this frame is sent, which is normal.
        QByteArray finalRequest;
        QString finalReadError;
        if (sensorSession.readFrame(dataRequest.size(), &finalRequest, &finalReadError, 3000)
            && finalRequest == dataRequest) {
            QString ignoredWriteError;
            sensorSession.writeFrame(dataResponse, &ignoredWriteError);
        }
    });
    sensorThread->start();

    gucds::DeviceCommunicationController controller;
    controller.configure(pcPort, 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QSignalSpy sampleSpy(&controller, &gucds::DeviceCommunicationController::frequencyTensionSampleReady);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);
    QVERIFY(controller.measureFrequencyTension(QStringLiteral("automatic_frequency_test")));
    QTRY_COMPARE_WITH_TIMEOUT(sampleSpy.size(), 2, 12000);
    QVERIFY(controller.isRunning());
    const gucds::CommunicationResult firstSample = qvariant_cast<gucds::CommunicationResult>(
        sampleSpy.at(0).at(0));
    const gucds::CommunicationResult secondSample = qvariant_cast<gucds::CommunicationResult>(
        sampleSpy.at(1).at(0));
    QVERIFY(firstSample.success);
    QVERIFY(firstSample.hasSample);
    QVERIFY(firstSample.responseText.contains(QStringLiteral("MODE: AUTO (CONTINUOUS DIRECT DATA READ)")));
    QVERIFY(!firstSample.responseText.contains(QStringLiteral("B1 RX:")));
    QVERIFY(!firstSample.responseText.contains(QStringLiteral("STATUS RX:")));
    QCOMPARE(firstSample.numericValues.at(2), 1.0);
    QCOMPARE(firstSample.numericValues.at(7), 4.0);
    QCOMPARE(firstSample.numericValues.at(8), 0.0);
    QVERIFY(secondSample.success);
    QVERIFY(secondSample.hasSample);

    controller.cancel();
    QTRY_VERIFY_WITH_TIMEOUT(!controller.isRunning(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 3000);
    const gucds::CommunicationResult stopped = qvariant_cast<gucds::CommunicationResult>(
        resultSpy.takeFirst().at(0));
    QVERIFY(stopped.success);
    QCOMPARE(stopped.context, QStringLiteral("frequency_tension_stream_stopped"));

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::deviceCommunicationControllerCom0ComCommandFrequencyMeasurement()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the command frequency measurement test.");

    const QByteArray getMcuRequest = QByteArrayLiteral("AT,get,mcupar\r\n");
    const QByteArray getSensorRequest = QByteArrayLiteral("AT,get,senpar\r\n");
    const QByteArray startRequest = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    const QByteArray statusRequest = gucds::VirtualModbusClient::buildReadHoldingRegisters(1, 0x0002, 1);
    const QByteArray dataRequest = gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 8);
    gucds::VirtualModbusClient sensorDevice;
    QByteArray startResponse;
    QByteArray pendingStatus;
    QByteArray readyStatus;
    QByteArray dataResponse;
    QVERIFY(sensorDevice.transactFrame(startRequest, &startResponse));
    QVERIFY(sensorDevice.transactFrame(statusRequest, &pendingStatus));
    QVERIFY(sensorDevice.transactFrame(statusRequest, &readyStatus));
    QVERIFY(sensorDevice.transactFrame(dataRequest, &dataResponse));

    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        const auto receiveAndReply = [&](const QByteArray &expected, const QByteArray &reply) {
            QByteArray request;
            if (!sensorSession.readFrame(expected.size(), &request, &sensorError, 3000))
                return false;
            if (request != expected) {
                sensorError = QStringLiteral("Unexpected command-mode frame: %1")
                                  .arg(gucds::VirtualModbusClient::formatHex(request));
                return false;
            }
            return sensorSession.writeFrame(reply, &sensorError);
        };

        if (!receiveAndReply(
                getMcuRequest,
                QByteArrayLiteral("get,mcupar,9600,3,0,1,1,over\r\n"))) {
            return;
        }
        if (!receiveAndReply(
                getSensorRequest,
                QByteArrayLiteral("get,senpar,8,2,over\r\n"))) {
            return;
        }
        if (!receiveAndReply(startRequest, startResponse))
            return;
        if (!receiveAndReply(statusRequest, pendingStatus))
            return;
        if (!receiveAndReply(statusRequest, readyStatus))
            return;
        receiveAndReply(dataRequest, dataResponse);
    });
    sensorThread->start();

    gucds::DeviceCommunicationController controller;
    controller.configure(pcPort, 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);
    QVERIFY(controller.measureFrequencyTension(QStringLiteral("command_frequency_test")));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 8000);
    const gucds::CommunicationResult result = qvariant_cast<gucds::CommunicationResult>(
        resultSpy.takeFirst().at(0));
    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(result.hasSample);
    QVERIFY(result.responseText.contains(QStringLiteral("B1 RX:")));
    QVERIFY(result.responseText.contains(QStringLiteral("SAMPLING: 800 Hz, 1024 points")));
    QCOMPARE(result.numericValues.size(), 9);
    QCOMPARE(result.numericValues.at(2), 0.0);
    QCOMPARE(result.numericValues.at(7), 8.0);
    QCOMPARE(result.numericValues.at(8), 2.0);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::deviceCommunicationControllerCom0ComLowPowerFrequencyMeasurement()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the low-power frequency test.");

    const QByteArray getMcuRequest = QByteArrayLiteral("AT,get,mcupar\r\n");
    const QByteArray getSensorRequest = QByteArrayLiteral("AT,get,senpar\r\n");
    const QByteArray dataRequest =
        gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 8);
    gucds::VirtualModbusClient device;
    QByteArray ignored;
    QByteArray dataResponse;
    const QByteArray startRequest = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    QVERIFY(device.transactFrame(startRequest, &ignored));
    QVERIFY(device.transactFrame(dataRequest, &dataResponse));

    gucds::SerialSession sensorSession;
    QString openError;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &openError), qPrintable(openError));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        const auto receiveAndReply = [&](const QByteArray &expected, const QByteArray &reply) {
            QByteArray actual;
            if (!sensorSession.readFrame(expected.size(), &actual, &sensorError, 7000))
                return false;
            if (actual != expected) {
                sensorError = QStringLiteral("Unexpected low-power frame: %1")
                                  .arg(gucds::VirtualModbusClient::formatHex(actual));
                return false;
            }
            return sensorSession.writeFrame(reply, &sensorError);
        };
        if (!receiveAndReply(
                getMcuRequest,
                QByteArrayLiteral("get,mcupar,9600,0,2,1,1,over\r\n"))) {
            return;
        }
        if (!receiveAndReply(
                getSensorRequest,
                QByteArrayLiteral("get,senpar,4,0,over\r\n"))) {
            return;
        }
        receiveAndReply(dataRequest, dataResponse);
    });
    sensorThread->start();

    gucds::DeviceCommunicationController controller;
    controller.configure(pcPort, 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QSignalSpy sampleSpy(&controller,
                         &gucds::DeviceCommunicationController::frequencyTensionSampleReady);
    QVERIFY(controller.measureFrequencyTension(QStringLiteral("low_power_frequency_test")));
    QTRY_COMPARE_WITH_TIMEOUT(sampleSpy.size(), 1, 7000);
    const gucds::CommunicationResult sample = qvariant_cast<gucds::CommunicationResult>(
        sampleSpy.first().at(0));
    QVERIFY(sample.success);
    QVERIFY(sample.hasSample);
    QVERIFY(sample.responseText.contains(
        QStringLiteral("MODE: LOW POWER (SCHEDULED DIRECT DATA READ)")));
    QVERIFY(!sample.responseText.contains(QStringLiteral("B1 RX:")));
    controller.cancel();
    QTRY_VERIFY_WITH_TIMEOUT(!controller.isRunning(), 3000);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::deviceCommunicationControllerCom0ComActiveFrame()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the active-frame controller test.");

    const QByteArray startRequest = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    gucds::VirtualModbusClient sensorDevice;
    QByteArray echo;
    QVERIFY(sensorDevice.transactFrame(startRequest, &echo));
    const QByteArray statusRequest = gucds::VirtualModbusClient::buildReadHoldingRegisters(1, 0x0002, 1);
    QByteArray ignored;
    QVERIFY(sensorDevice.transactFrame(statusRequest, &ignored));
    QVERIFY(sensorDevice.transactFrame(statusRequest, &ignored));
    const QByteArray dataRequest = gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 8);
    QByteArray activeFrame;
    QVERIFY(sensorDevice.transactFrame(dataRequest, &activeFrame));
    QCOMPARE(activeFrame.size(), 21);

    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        QByteArray request;
        if (!sensorSession.readFrame(startRequest.size(), &request, &sensorError, 2000))
            return;
        if (request != startRequest) {
            sensorError = QStringLiteral("Unexpected B1 request");
            return;
        }
        if (!sensorSession.writeFrame(echo, &sensorError))
            return;
        // Send the unsolicited frame after the B1 echo transaction's idle window,
        // matching two distinct frames on the wire instead of one coalesced read.
        QThread::msleep(220);
        sensorSession.writeFrame(activeFrame, &sensorError);
    });
    sensorThread->start();

    gucds::DeviceCommunicationController controller;
    controller.configure(pcPort, 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);
    QVERIFY(controller.measure(QStringLiteral("active_frame_test")));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 5000);
    const gucds::CommunicationResult result = qvariant_cast<gucds::CommunicationResult>(
        resultSpy.takeFirst().at(0));
    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(result.hasSample);
    QCOMPARE(result.responseBytes, activeFrame);
    QCOMPARE(result.message, QStringLiteral("收到主动测量帧"));

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::deviceCommunicationControllerCom0ComDelayedMeasurement()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the delayed-measurement controller test.");

    const QByteArray startRequest = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    const QByteArray statusRequest = gucds::VirtualModbusClient::buildReadHoldingRegisters(1, 0x0002, 1);
    const QByteArray dataRequest = gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 8);

    gucds::VirtualModbusClient pendingDevice;
    QByteArray startEcho;
    QByteArray pendingStatus;
    QVERIFY(pendingDevice.transactFrame(startRequest, &startEcho));
    QVERIFY(pendingDevice.transactFrame(statusRequest, &pendingStatus));
    QCOMPARE(gucds::VirtualModbusClient::parseStatusResponse(pendingStatus).code, quint8(0xFF));

    gucds::VirtualModbusClient completedDevice;
    QByteArray ignored;
    QByteArray completedStatus;
    QByteArray dataResponse;
    QVERIFY(completedDevice.transactFrame(startRequest, &ignored));
    QVERIFY(completedDevice.transactFrame(statusRequest, &ignored));
    QVERIFY(completedDevice.transactFrame(statusRequest, &completedStatus));
    QVERIFY(gucds::VirtualModbusClient::parseStatusResponse(completedStatus).success);
    QVERIFY(completedDevice.transactFrame(dataRequest, &dataResponse));

    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        QByteArray request;
        if (!sensorSession.readFrame(startRequest.size(), &request, &sensorError, 3000))
            return;
        if (request != startRequest || !sensorSession.writeFrame(startEcho, &sensorError))
            return;

        constexpr int pendingPollCount = 10;
        for (int poll = 0; poll < pendingPollCount; ++poll) {
            request.clear();
            if (!sensorSession.readFrame(statusRequest.size(), &request, &sensorError, 5000))
                return;
            if (request != statusRequest || !sensorSession.writeFrame(pendingStatus, &sensorError))
                return;
        }

        request.clear();
        if (!sensorSession.readFrame(statusRequest.size(), &request, &sensorError, 5000))
            return;
        if (request != statusRequest || !sensorSession.writeFrame(completedStatus, &sensorError))
            return;

        request.clear();
        if (!sensorSession.readFrame(dataRequest.size(), &request, &sensorError, 5000))
            return;
        if (request != dataRequest)
            return;
        sensorSession.writeFrame(dataResponse, &sensorError);
    });
    sensorThread->start();

    gucds::DeviceCommunicationController controller;
    controller.configure(pcPort, 9600, 1, gucds::DeviceWireProtocol::Modbus);
    QSignalSpy resultSpy(&controller, &gucds::DeviceCommunicationController::resultReady);
    QVERIFY(controller.measure(QStringLiteral("delayed_measurement_test")));
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 10000);
    const gucds::CommunicationResult result = qvariant_cast<gucds::CommunicationResult>(
        resultSpy.takeFirst().at(0));
    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(result.hasSample);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::serialSessionCom0ComF405PreciseMeasurement()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the com0com F405 smoke test.");

    gucds::SerialSession pcSession;
    gucds::SerialSession sensorSession;
    QString error;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &error), qPrintable(error));
    QVERIFY2(pcSession.open(pcPort, 9600, &error), qPrintable(error));

    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        gucds::VirtualModbusClient sensor;
        for (int i = 0; i < 4; ++i) {
            QByteArray request;
            if (!sensorSession.readFrame(8, &request, &sensorError, 3000))
                return;
            QByteArray response;
            if (!sensor.transactFrame(request, &response)) {
                sensorError = sensor.lastFrame();
                return;
            }
            if (!sensorSession.writeFrame(response, &sensorError))
                return;
        }
    });

    sensorThread->start();

    const QByteArray startFrame = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    QByteArray response;
    QVERIFY2(pcSession.transactFrame(startFrame, startFrame.size(), &response, &error, 2000), qPrintable(error));
    QCOMPARE(response, startFrame);

    const QByteArray statusQuery = gucds::VirtualModbusClient::buildReadHoldingRegisters(1, 0x0002, 0x0001);
    QVERIFY2(pcSession.transactFrame(statusQuery,
                                     gucds::VirtualModbusClient::expectedReadResponseBytes(1),
                                     &response,
                                     &error,
                                     2000),
             qPrintable(error));
    QCOMPARE(gucds::VirtualModbusClient::parseStatusResponse(response).code, quint8(0xFF));

    QVERIFY2(pcSession.transactFrame(statusQuery,
                                     gucds::VirtualModbusClient::expectedReadResponseBytes(1),
                                     &response,
                                     &error,
                                     2000),
             qPrintable(error));
    const gucds::SensorModbusStatus status = gucds::VirtualModbusClient::parseStatusResponse(response);
    QCOMPARE(status.code, quint8(0xA1));
    QVERIFY(status.success);

    const QByteArray dataQuery = gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 0x0008);
    QVERIFY2(pcSession.transactFrame(dataQuery,
                                     gucds::VirtualModbusClient::expectedReadResponseBytes(8),
                                     &response,
                                     &error,
                                     2000),
             qPrintable(error));
    const gucds::SensorModbusSample sample = gucds::VirtualModbusClient::parseMeasurementResponse(response);
    QVERIFY(sample.pitch > 1.0f);
    QVERIFY(sample.roll < 0.0f);
    QVERIFY(sample.error > 0.0f);
    QVERIFY(sample.temperature > 20.0f);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;

    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void ProtocolTest::virtualModbusReadsWrites()
{
    const QByteArray encodedBaud = gucds::VirtualModbusClient::encodeMcuBaud(9600);
    QCOMPARE(encodedBaud, QByteArray::fromHex("80002500"));
    QCOMPARE(gucds::VirtualModbusClient::decodeMcuBaud(encodedBaud), quint32(9600));
    QCOMPARE(gucds::VirtualModbusClient::loraModuleModeToUpper(2), quint16(0));
    QCOMPARE(gucds::VirtualModbusClient::loraModuleModeToUpper(4), quint16(1));
    QCOMPARE(gucds::VirtualModbusClient::loraModuleModeToUpper(1), quint16(2));
    QCOMPARE(gucds::VirtualModbusClient::loraUpperModeToModule(0), quint16(2));
    QCOMPARE(gucds::VirtualModbusClient::loraUpperModeToModule(1), quint16(4));
    QCOMPARE(gucds::VirtualModbusClient::loraUpperModeToModule(2), quint16(1));

    gucds::VirtualModbusClient client;
    client.setSlaveId(300);
    QCOMPARE(client.slaveId(), 247);
    client.setSlaveId(1);

    const QByteArray startFrame = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    QCOMPARE(gucds::VirtualModbusClient::formatHex(startFrame), QStringLiteral("01 06 00 00 00 B1 49 BE"));
    QVERIFY(gucds::VirtualModbusClient::hasValidCrc(startFrame));

    QByteArray response;
    QVERIFY(client.transactFrame(startFrame, &response));
    QCOMPARE(response, startFrame);

    const gucds::SensorModbusStatus measurementStatus = client.statusByte(0x0002);
    QCOMPARE(measurementStatus.code, quint8(0xFF));
    QVERIFY(!measurementStatus.success);

    QVector<quint16> statusRegisters = client.readHoldingRegisters(0x0002, 1);
    QCOMPARE(statusRegisters.size(), 1);
    QCOMPARE(statusRegisters.at(0), quint16(0xFF00));

    const QByteArray statusQuery = gucds::VirtualModbusClient::buildReadHoldingRegisters(1, 0x0002, 0x0001);
    QVERIFY(client.transactFrame(statusQuery, &response));
    QCOMPARE(quint8(response.at(0)), quint8(0x01));
    QCOMPARE(quint8(response.at(1)), quint8(0x03));
    QCOMPARE(quint8(response.at(2)), quint8(0x02));
    QCOMPARE(quint8(response.at(3)), quint8(0xFF));
    QVERIFY(gucds::VirtualModbusClient::hasValidCrc(response));

    QVERIFY(client.transactFrame(statusQuery, &response));
    QCOMPARE(quint8(response.at(3)), quint8(0xA1));
    const gucds::SensorModbusStatus readyStatus = gucds::VirtualModbusClient::parseStatusResponse(response);
    QVERIFY(readyStatus.success);
    QVERIFY(gucds::VirtualModbusClient::hasValidCrc(response));

    const QByteArray measurementQuery = gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 0x0008);
    QVERIFY(client.transactFrame(measurementQuery, &response));
    const gucds::SensorModbusSample sample = gucds::VirtualModbusClient::parseMeasurementResponse(response);
    QVERIFY(sample.pitch > 1.0f);
    QVERIFY(sample.roll < 0.0f);
    QVERIFY(sample.temperature > 20.0f);

    const QByteArray searchFrame = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::SearchDevice));
    QVERIFY(client.transactFrame(searchFrame, &response));
    const gucds::SensorModbusStatus searchStatus = client.statusByte(0x0000);
    QCOMPARE(searchStatus.code, quint8(0xA7));
    QVERIFY(searchStatus.success);

    const QByteArray saveMcuFrame = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::SaveMcuParameters));
    QVERIFY(client.transactFrame(saveMcuFrame, &response));
    QCOMPARE(client.statusByte(0x0000).code, quint8(0xA2));

    const QByteArray writeMany = gucds::VirtualModbusClient::buildWriteMultipleHoldingRegisters(
        1,
        2,
        QByteArray::fromHex("00000BB800010000"));
    QVERIFY(!writeMany.isEmpty());
    QVERIFY(client.transactFrame(writeMany, &response));
    QCOMPARE(response.left(6), QByteArray::fromHex("011000020004"));
    QVERIFY(gucds::VirtualModbusClient::hasValidCrc(response));

    const QByteArray continuousFrame = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::StartContinuousSampling));
    QVERIFY(client.transactFrame(continuousFrame, &response));
    QCOMPARE(client.statusByte(0x0000).code, quint8(0xA8));
}

QTEST_MAIN(ProtocolTest)
#include "test_protocol.moc"
