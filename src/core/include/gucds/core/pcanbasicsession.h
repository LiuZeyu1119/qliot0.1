#pragma once

#include "gucds/core/canframe.h"

#include <QObject>
#include <QString>

#include <memory>

class QTimer;

namespace gucds {

class PcanBasicSession final : public QObject
{
    Q_OBJECT

public:
    explicit PcanBasicSession(QObject *parent = nullptr);
    ~PcanBasicSession() override;

    bool open(int usbChannel = 1, int bitrate = 500000);
    void close();
    bool isOpen() const;
    int usbChannel() const;
    int bitrate() const;
    QString apiPath() const;
    QString lastError() const;

    bool sendFrame(const CanFrame &frame);
    quint32 busStatus() const;

    static quint16 baudCode(int bitrate);
    static quint16 usbChannelHandle(int usbChannel);
    static bool validateFrame(const CanFrame &frame, QString *errorMessage = nullptr);
    static bool hasBusError(quint32 status);

signals:
    void connectionChanged(bool connected);
    void frameReceived(const gucds::CanFrame &frame);
    void frameSent(const gucds::CanFrame &frame);
    void errorOccurred(const QString &message);
    void busStatusChanged(quint32 status, const QString &description);

private slots:
    void poll();

private:
    struct Api;

    bool loadApi();
    void setError(const QString &message);
    QString statusDescription(quint32 status) const;

    std::unique_ptr<Api> m_api;
    QTimer *m_pollTimer = nullptr;
    quint16 m_handle = 0;
    int m_usbChannel = 0;
    int m_bitrate = 0;
    quint32 m_lastBusStatus = 0;
    quint32 m_lastReadError = 0;
    QString m_apiPath;
    QString m_lastError;
    bool m_open = false;
};

} // namespace gucds
