#include "gucds/core/pcanbasicsession.h"

#include <QCoreApplication>
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>

#include <cstring>

namespace {

enum class Stage {
    WaitingForHeartbeat,
    WaitingForEcho,
    VerifyingStatus,
};

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    gucds::PcanBasicSession session;
    QTimer timeout;
    timeout.setSingleShot(true);
    Stage stage = Stage::WaitingForHeartbeat;
    QByteArray expectedEcho;
    bool statusErrorSeen = false;
    bool backendErrorSeen = false;

    QObject::connect(&session, &gucds::PcanBasicSession::errorOccurred,
                     [&](const QString &message) {
        backendErrorSeen = true;
        qWarning().noquote() << message;
    });
    QObject::connect(&session, &gucds::PcanBasicSession::busStatusChanged,
                     [&](quint32 status, const QString &) {
        statusErrorSeen = statusErrorSeen || status != 0U;
    });
    QObject::connect(&timeout, &QTimer::timeout, &application, [&] {
        qCritical().noquote() << (stage == Stage::WaitingForHeartbeat
                                      ? "FAIL: no valid F103 heartbeat within 5 seconds"
                                      : "FAIL: no matching 0x322 echo within 2 seconds");
        application.exit(stage == Stage::WaitingForHeartbeat ? 5 : 6);
    });
    QObject::connect(&session, &gucds::PcanBasicSession::frameReceived,
                     &application, [&](const gucds::CanFrame &frame) {
        if (stage == Stage::WaitingForHeartbeat) {
            const bool heartbeat = !frame.extended && !frame.remote && frame.id == 0x123U
                && frame.dlc == 8U && frame.payload.size() == 8
                && frame.payload.first(4) == QByteArrayLiteral("F103");
            if (!heartbeat)
                return;

            qInfo().noquote() << "RX heartbeat 0x123:" << frame.payload.toHex(' ').toUpper();
            const quint64 nonce = QRandomGenerator::global()->generate64();
            expectedEcho.resize(8);
            std::memcpy(expectedEcho.data(), &nonce, sizeof(nonce));
            gucds::CanFrame request;
            request.id = 0x321U;
            request.payload = expectedEcho;
            request.dlc = 8U;
            if (!session.sendFrame(request)) {
                qCritical().noquote() << "FAIL: cannot send 0x321:" << session.lastError();
                application.exit(7);
                return;
            }
            qInfo().noquote() << "TX request   0x321:" << expectedEcho.toHex(' ').toUpper();
            stage = Stage::WaitingForEcho;
            timeout.start(2000);
            return;
        }

        const bool echo = !frame.extended && !frame.remote && frame.id == 0x322U
            && frame.dlc == 8U && frame.payload == expectedEcho;
        if (!echo)
            return;
        qInfo().noquote() << "RX response  0x322:" << frame.payload.toHex(' ').toUpper();
        timeout.stop();
        stage = Stage::VerifyingStatus;
        QTimer::singleShot(0, &application, [&] {
            if (stage != Stage::VerifyingStatus)
                return;
            const quint32 status = session.busStatus();
            if (status != 0U || statusErrorSeen || backendErrorSeen) {
                qCritical().noquote() << "FAIL: PCAN reported an error; final status"
                                      << Qt::hex << status;
                application.exit(8);
                return;
            }
            qInfo().noquote() << "PASS: Qt PCAN backend verified heartbeat and bidirectional echo at 500 kbit/s.";
            application.exit(0);
        });
    });

    if (!session.open(1, 500000)) {
        qCritical().noquote() << "FAIL: cannot open PCAN-USB 1:" << session.lastError();
        return 4;
    }
    qInfo().noquote() << "PCAN-Basic:" << session.apiPath();
    timeout.start(5000);
    return application.exec();
}
