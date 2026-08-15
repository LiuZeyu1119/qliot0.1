#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QFormLayout;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QWidget;

class DtuConfigDialog final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DtuConfigDialog)

public:
    explicit DtuConfigDialog(QWidget *parent = nullptr);

    QString configCommand() const;
    bool channelEnabled() const;
    QString protocol() const;
    QString serialChannel() const;
    QString configurationSummary() const;

public slots:
    void accept() override;

private:
    void updateProtocolPage();
    void updateEnabledState();
    void updateMqttRows();
    void updateHttpRows();
    void updateSocketRows();
    void updateWebSocketRows();
    void fitCurrentProtocolPage();

    QComboBox *m_enabled = nullptr;
    QComboBox *m_protocol = nullptr;
    QComboBox *m_serialChannel = nullptr;
    QStackedWidget *m_protocolPages = nullptr;
    QPushButton *m_saveButton = nullptr;

    QWidget *m_mqttPage = nullptr;
    QFormLayout *m_mqttForm = nullptr;
    QSpinBox *m_mqttHeartbeat = nullptr;
    QLineEdit *m_mqttHost = nullptr;
    QSpinBox *m_mqttPort = nullptr;
    QLineEdit *m_mqttClientId = nullptr;
    QLineEdit *m_mqttUsername = nullptr;
    QLineEdit *m_mqttPassword = nullptr;
    QComboBox *m_mqttVersion = nullptr;
    QComboBox *m_mqttCleanSession = nullptr;
    QComboBox *m_mqttRetain = nullptr;
    QComboBox *m_mqttSubscribeQos = nullptr;
    QComboBox *m_mqttPublishQos = nullptr;
    QLineEdit *m_mqttSubscribeTopic = nullptr;
    QLineEdit *m_mqttPublishTopic = nullptr;
    QComboBox *m_mqttWillEnabled = nullptr;
    QComboBox *m_mqttWillQos = nullptr;
    QComboBox *m_mqttWillRetain = nullptr;
    QLineEdit *m_mqttWillTopic = nullptr;
    QLineEdit *m_mqttWillPayload = nullptr;
    QComboBox *m_mqttRegistrationType = nullptr;
    QLineEdit *m_mqttRegistrationData = nullptr;
    QComboBox *m_mqttIpv6 = nullptr;
    QComboBox *m_mqttSsl = nullptr;

    QWidget *m_httpPage = nullptr;
    QFormLayout *m_httpForm = nullptr;
    QComboBox *m_httpMethod = nullptr;
    QLineEdit *m_httpHost = nullptr;
    QSpinBox *m_httpPort = nullptr;
    QLineEdit *m_httpPath = nullptr;
    QSpinBox *m_httpTimeout = nullptr;
    QComboBox *m_httpCustomHeaderEnabled = nullptr;
    QLineEdit *m_httpCustomHeader = nullptr;
    QComboBox *m_httpResponseFilter = nullptr;
    QComboBox *m_httpRegistrationType = nullptr;
    QLineEdit *m_httpRegistrationData = nullptr;
    QComboBox *m_httpIpv6 = nullptr;
    QComboBox *m_httpSsl = nullptr;

    QWidget *m_socketPage = nullptr;
    QFormLayout *m_socketForm = nullptr;
    QComboBox *m_socketHeartbeatEnabled = nullptr;
    QComboBox *m_socketHeartbeatType = nullptr;
    QLineEdit *m_socketHeartbeatData = nullptr;
    QSpinBox *m_socketHeartbeatInterval = nullptr;
    QLineEdit *m_socketHost = nullptr;
    QSpinBox *m_socketPort = nullptr;
    QComboBox *m_socketPrefixType = nullptr;
    QLineEdit *m_socketPrefixData = nullptr;
    QComboBox *m_socketSuffixType = nullptr;
    QLineEdit *m_socketSuffixData = nullptr;
    QComboBox *m_socketRegistrationType = nullptr;
    QLineEdit *m_socketRegistrationData = nullptr;
    QComboBox *m_socketIpv6 = nullptr;
    QComboBox *m_socketSsl = nullptr;

    QWidget *m_webSocketPage = nullptr;
    QFormLayout *m_webSocketForm = nullptr;
    QLineEdit *m_webSocketUrl = nullptr;
    QComboBox *m_webSocketIpv6 = nullptr;
    QComboBox *m_webSocketCustomHeaderEnabled = nullptr;
    QLineEdit *m_webSocketCustomHeader = nullptr;
    QComboBox *m_webSocketHeartbeatEnabled = nullptr;
    QComboBox *m_webSocketHeartbeatType = nullptr;
    QLineEdit *m_webSocketHeartbeatData = nullptr;
    QSpinBox *m_webSocketHeartbeatInterval = nullptr;
    QComboBox *m_webSocketPrefixType = nullptr;
    QLineEdit *m_webSocketPrefixData = nullptr;
    QComboBox *m_webSocketSuffixType = nullptr;
    QLineEdit *m_webSocketSuffixData = nullptr;
    QComboBox *m_webSocketRegistrationType = nullptr;
    QLineEdit *m_webSocketRegistrationData = nullptr;
};
