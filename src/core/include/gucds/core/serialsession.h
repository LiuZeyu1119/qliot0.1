#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace gucds {

class SerialSession
{
public:
    SerialSession();
    ~SerialSession();

    SerialSession(const SerialSession &) = delete;
    SerialSession &operator=(const SerialSession &) = delete;

    static QStringList availablePorts();

    bool open(const QString &portName, int baudRate, QString *error = nullptr);
    bool reconnect(QString *error = nullptr);
    void close();

    bool writeFrame(const QByteArray &frame, QString *error = nullptr);
    bool readFrame(int expectedBytes, QByteArray *frame, QString *error = nullptr, int timeoutMs = 1000);
    bool transactFrame(const QByteArray &request,
                       int expectedResponseBytes,
                       QByteArray *response,
                       QString *error = nullptr,
                       int timeoutMs = 1000);
    bool transactText(const QString &request,
                      QByteArray *response,
                      QString *error = nullptr,
                      int timeoutMs = 3000,
                      int idleTimeoutMs = 120);
    bool readUntilIdle(QByteArray *data,
                       QString *error = nullptr,
                       int timeoutMs = 3000,
                       int idleTimeoutMs = 120);

    bool isBackendAvailable() const;
    bool isConnected() const;
    QString portName() const;
    int baudRate() const;
    QString statusText() const;

private:
    QString m_portName = QStringLiteral("COM1");
    int m_baudRate = 9600;
    bool m_connected = false;
    void *m_handle = nullptr;
};

} // namespace gucds
