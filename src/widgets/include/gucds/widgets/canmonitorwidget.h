#pragma once

#include "gucds/core/canframe.h"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableView;
class QTimer;

namespace gucds {

class CanFrameTableModel;
class PcanBasicSession;

class CanMonitorWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit CanMonitorWidget(QWidget *parent = nullptr);

private:
    enum class SelfTestStage {
        Idle,
        WaitingForHeartbeat,
        WaitingForEcho,
        VerifyingStatus,
    };

    static bool parseHexPayload(const QString &text, QByteArray *payload, QString *errorMessage);
    static bool parseCanId(const QString &text, quint32 *id);

    void toggleConnection();
    void updateConnectionUi(bool connected);
    void sendEditorFrame();
    void handleReceivedFrame(const gucds::CanFrame &frame);
    void handleSentFrame(const gucds::CanFrame &frame);
    void startBoardSelfTest();
    void sendSelfTestRequest();
    void finishSelfTest(bool passed, const QString &message);
    void setStateLabel(QLabel *label, const QString &text, const QString &color);
    void updateCounters();

    PcanBasicSession *m_session = nullptr;
    CanFrameTableModel *m_frameModel = nullptr;
    QComboBox *m_channelCombo = nullptr;
    QComboBox *m_bitrateCombo = nullptr;
    QLabel *m_connectionState = nullptr;
    QLabel *m_busState = nullptr;
    QLabel *m_boardState = nullptr;
    QLabel *m_rxCountLabel = nullptr;
    QLabel *m_txCountLabel = nullptr;
    QLabel *m_apiPathLabel = nullptr;
    QLabel *m_operationState = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_selfTestButton = nullptr;
    QLineEdit *m_idEditor = nullptr;
    QLineEdit *m_dataEditor = nullptr;
    QCheckBox *m_extendedCheck = nullptr;
    QCheckBox *m_remoteCheck = nullptr;
    QSpinBox *m_dlcEditor = nullptr;
    QPushButton *m_sendButton = nullptr;
    QTableView *m_frameTable = nullptr;
    QTimer *m_selfTestTimer = nullptr;
    QTimer *m_heartbeatTimer = nullptr;
    QByteArray m_expectedEcho;
    SelfTestStage m_selfTestStage = SelfTestStage::Idle;
    bool m_selfTestErrorSeen = false;
    qint64 m_lastHeartbeatMs = 0;
    quint32 m_lastHeartbeatCounter = 0;
    quint64 m_receivedCount = 0;
    quint64 m_transmittedCount = 0;
};

} // namespace gucds
