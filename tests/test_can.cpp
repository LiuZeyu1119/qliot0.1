#include "gucds/core/canframetablemodel.h"
#include "gucds/core/pcanbasicsession.h"

#include <QtTest/QtTest>

class CanTest : public QObject
{
    Q_OBJECT

private slots:
    void mapsPcanChannelsAndBitrates();
    void validatesClassicCanFrames();
    void storesAndLimitsFrameHistory();
};

void CanTest::mapsPcanChannelsAndBitrates()
{
    QCOMPARE(gucds::PcanBasicSession::usbChannelHandle(1), quint16(0x51));
    QCOMPARE(gucds::PcanBasicSession::usbChannelHandle(8), quint16(0x58));
    QCOMPARE(gucds::PcanBasicSession::usbChannelHandle(9), quint16(0x509));
    QCOMPARE(gucds::PcanBasicSession::usbChannelHandle(16), quint16(0x510));
    QCOMPARE(gucds::PcanBasicSession::usbChannelHandle(0), quint16(0));
    QCOMPARE(gucds::PcanBasicSession::usbChannelHandle(17), quint16(0));

    QCOMPARE(gucds::PcanBasicSession::baudCode(500000), quint16(0x001C));
    QCOMPARE(gucds::PcanBasicSession::baudCode(125000), quint16(0x031C));
    QCOMPARE(gucds::PcanBasicSession::baudCode(123456), quint16(0));
}

void CanTest::validatesClassicCanFrames()
{
    gucds::CanFrame frame;
    frame.id = 0x7FF;
    frame.payload = QByteArray::fromHex("1122334455667788");
    frame.dlc = 8;
    QVERIFY(gucds::PcanBasicSession::validateFrame(frame));

    QString error;
    frame.id = 0x800;
    QVERIFY(!gucds::PcanBasicSession::validateFrame(frame, &error));
    QVERIFY(!error.isEmpty());

    frame.extended = true;
    frame.id = 0x1FFFFFFF;
    QVERIFY(gucds::PcanBasicSession::validateFrame(frame));
    frame.id = 0x20000000;
    QVERIFY(!gucds::PcanBasicSession::validateFrame(frame));

    frame = {};
    frame.remote = true;
    frame.dlc = 8;
    QVERIFY(gucds::PcanBasicSession::validateFrame(frame));
    frame.payload = QByteArrayLiteral("x");
    QVERIFY(!gucds::PcanBasicSession::validateFrame(frame));

    frame = {};
    frame.payload = QByteArray::fromHex("1122");
    frame.dlc = 1;
    QVERIFY(!gucds::PcanBasicSession::validateFrame(frame));
    frame.dlc = 2;
    QVERIFY(gucds::PcanBasicSession::validateFrame(frame));
}

void CanTest::storesAndLimitsFrameHistory()
{
    gucds::CanFrameTableModel model;
    model.setMaximumFrameCount(2);

    gucds::CanFrame first;
    first.id = 0x123;
    first.payload = QByteArrayLiteral("F103");
    first.dlc = 4;
    first.wallClockMs = 1000;
    model.appendFrame(first);

    gucds::CanFrame second;
    second.id = 0x321;
    second.payload = QByteArray::fromHex("1122");
    second.dlc = 2;
    second.transmitted = true;
    second.wallClockMs = 2000;
    model.appendFrame(second);

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0, gucds::CanFrameTableModel::IdColumn)).toString(),
             QStringLiteral("0x123"));
    QCOMPARE(model.data(model.index(1, gucds::CanFrameTableModel::DataColumn)).toString(),
             QStringLiteral("11 22"));

    gucds::CanFrame third;
    third.id = 0x322;
    third.extended = true;
    third.wallClockMs = 3000;
    model.appendFrame(third);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.frameAt(0).id, quint32(0x321));
    QCOMPARE(model.frameAt(1).id, quint32(0x322));

    model.clear();
    QCOMPARE(model.rowCount(), 0);
}

QTEST_APPLESS_MAIN(CanTest)

#include "test_can.moc"
