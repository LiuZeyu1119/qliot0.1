#include "gucds/widgets/canmonitorwidget.h"

#include "gucds/core/canframetablemodel.h"
#include "gucds/core/pcanbasicsession.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

#include <cstring>

namespace {

void setPrimaryButton(QPushButton *button)
{
    button->setProperty("buttonRole", QStringLiteral("primary"));
}

QLabel *makeMetricLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(QStringLiteral("metricLabel"));
    return label;
}

QLabel *makeMetricValue(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(QStringLiteral("metricValue"));
    return label;
}

} // namespace

namespace gucds {

CanMonitorWidget::CanMonitorWidget(QWidget *parent)
    : QWidget(parent)
    , m_session(new PcanBasicSession(this))
    , m_frameModel(new CanFrameTableModel(this))
    , m_selfTestTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("canMonitorWidget"));
    setProperty("workspacePage", true);
    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(14, 12, 14, 14);
    pageLayout->setSpacing(10);

    auto *title = new QLabel(tr("CAN 总线调试"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    pageLayout->addWidget(title);
    auto *subtitle = new QLabel(
        tr("通过 PCAN-USB 监视和发送经典 CAN 帧，并可一键验证 STM32F103 开发板。"), this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    subtitle->setWordWrap(true);
    pageLayout->addWidget(subtitle);

    auto *connectionGroup = new QGroupBox(tr("PCAN 连接与开发板状态"), this);
    connectionGroup->setProperty("surfaceCard", true);
    auto *connectionLayout = new QGridLayout(connectionGroup);
    connectionLayout->setHorizontalSpacing(10);
    connectionLayout->setVerticalSpacing(8);

    m_channelCombo = new QComboBox(connectionGroup);
    m_channelCombo->setObjectName(QStringLiteral("canChannel"));
    for (int channel = 1; channel <= 16; ++channel)
        m_channelCombo->addItem(QStringLiteral("PCAN-USB %1").arg(channel), channel);

    m_bitrateCombo = new QComboBox(connectionGroup);
    m_bitrateCombo->setObjectName(QStringLiteral("canBitrate"));
    const QList<int> bitrates = {1000000, 800000, 500000, 250000, 125000,
                                100000, 50000, 20000, 10000, 5000};
    for (int bitrate : bitrates) {
        const QString label = bitrate >= 1000000
            ? QStringLiteral("%1 Mbit/s").arg(double(bitrate) / 1000000.0, 0, 'g', 3)
            : QStringLiteral("%1 kbit/s").arg(bitrate / 1000);
        m_bitrateCombo->addItem(label, bitrate);
    }
    m_bitrateCombo->setCurrentIndex(m_bitrateCombo->findData(500000));

    m_connectButton = new QPushButton(tr("连接"), connectionGroup);
    m_connectButton->setObjectName(QStringLiteral("canConnect"));
    setPrimaryButton(m_connectButton);
    m_selfTestButton = new QPushButton(tr("开发板自检"), connectionGroup);
    m_selfTestButton->setObjectName(QStringLiteral("canBoardSelfTest"));

    connectionLayout->addWidget(new QLabel(tr("通道"), connectionGroup), 0, 0);
    connectionLayout->addWidget(m_channelCombo, 0, 1);
    connectionLayout->addWidget(new QLabel(tr("波特率"), connectionGroup), 0, 2);
    connectionLayout->addWidget(m_bitrateCombo, 0, 3);
    connectionLayout->addWidget(m_connectButton, 0, 4);
    connectionLayout->addWidget(m_selfTestButton, 0, 5);

    m_connectionState = makeMetricValue(tr("未连接"), connectionGroup);
    m_connectionState->setObjectName(QStringLiteral("canConnectionState"));
    m_busState = makeMetricValue(tr("未知"), connectionGroup);
    m_busState->setObjectName(QStringLiteral("canBusState"));
    m_boardState = makeMetricValue(tr("等待心跳"), connectionGroup);
    m_boardState->setObjectName(QStringLiteral("canBoardState"));
    m_boardState->setWordWrap(true);
    m_boardState->setMaximumWidth(300);
    m_rxCountLabel = makeMetricValue(QStringLiteral("0"), connectionGroup);
    m_rxCountLabel->setObjectName(QStringLiteral("canRxCount"));
    m_txCountLabel = makeMetricValue(QStringLiteral("0"), connectionGroup);
    m_txCountLabel->setObjectName(QStringLiteral("canTxCount"));

    connectionLayout->addWidget(makeMetricLabel(tr("连接"), connectionGroup), 1, 0);
    connectionLayout->addWidget(m_connectionState, 1, 1);
    connectionLayout->addWidget(makeMetricLabel(tr("总线"), connectionGroup), 1, 2);
    connectionLayout->addWidget(m_busState, 1, 3);
    connectionLayout->addWidget(makeMetricLabel(tr("F103"), connectionGroup), 1, 4);
    connectionLayout->addWidget(m_boardState, 1, 5);
    connectionLayout->addWidget(makeMetricLabel(tr("接收帧"), connectionGroup), 2, 0);
    connectionLayout->addWidget(m_rxCountLabel, 2, 1);
    connectionLayout->addWidget(makeMetricLabel(tr("发送帧"), connectionGroup), 2, 2);
    connectionLayout->addWidget(m_txCountLabel, 2, 3);
    connectionLayout->addWidget(makeMetricLabel(tr("API"), connectionGroup), 2, 4);
    m_apiPathLabel = new QLabel(tr("尚未加载"), connectionGroup);
    m_apiPathLabel->setObjectName(QStringLiteral("canApiPath"));
    m_apiPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_apiPathLabel->setWordWrap(true);
    m_apiPathLabel->setMaximumWidth(300);
    m_apiPathLabel->setToolTip(m_apiPathLabel->text());
    connectionLayout->addWidget(m_apiPathLabel, 2, 5);
    connectionLayout->addWidget(makeMetricLabel(tr("操作"), connectionGroup), 3, 0);
    m_operationState = new QLabel(tr("就绪"), connectionGroup);
    m_operationState->setObjectName(QStringLiteral("canOperationState"));
    m_operationState->setWordWrap(true);
    connectionLayout->addWidget(m_operationState, 3, 1, 1, 5);
    connectionLayout->setColumnStretch(1, 1);
    connectionLayout->setColumnStretch(3, 1);
    connectionLayout->setColumnStretch(5, 2);
    pageLayout->addWidget(connectionGroup);

    auto *sendGroup = new QGroupBox(tr("发送经典 CAN 帧"), this);
    sendGroup->setProperty("surfaceCard", true);
    auto *sendLayout = new QGridLayout(sendGroup);
    sendLayout->setHorizontalSpacing(10);
    sendLayout->setVerticalSpacing(8);

    m_idEditor = new QLineEdit(QStringLiteral("0x321"), sendGroup);
    m_idEditor->setObjectName(QStringLiteral("canSendId"));
    m_idEditor->setPlaceholderText(QStringLiteral("0x000 - 0x7FF"));
    m_dataEditor = new QLineEdit(QStringLiteral("11 22 33 44 55 66 77 88"), sendGroup);
    m_dataEditor->setObjectName(QStringLiteral("canSendData"));
    m_dataEditor->setPlaceholderText(tr("最多 8 字节，例如 11 22 33 44"));
    m_extendedCheck = new QCheckBox(tr("扩展帧"), sendGroup);
    m_extendedCheck->setObjectName(QStringLiteral("canExtended"));
    m_remoteCheck = new QCheckBox(tr("RTR 远程帧"), sendGroup);
    m_remoteCheck->setObjectName(QStringLiteral("canRemote"));
    m_dlcEditor = new QSpinBox(sendGroup);
    m_dlcEditor->setObjectName(QStringLiteral("canRemoteDlc"));
    m_dlcEditor->setRange(0, 8);
    m_dlcEditor->setValue(8);
    m_dlcEditor->setEnabled(false);
    m_sendButton = new QPushButton(tr("发送"), sendGroup);
    m_sendButton->setObjectName(QStringLiteral("canSend"));
    setPrimaryButton(m_sendButton);

    sendLayout->addWidget(new QLabel(QStringLiteral("ID"), sendGroup), 0, 0);
    sendLayout->addWidget(m_idEditor, 0, 1);
    sendLayout->addWidget(m_extendedCheck, 0, 2);
    sendLayout->addWidget(m_remoteCheck, 0, 3);
    sendLayout->addWidget(new QLabel(QStringLiteral("DLC"), sendGroup), 0, 4);
    sendLayout->addWidget(m_dlcEditor, 0, 5);
    sendLayout->addWidget(new QLabel(tr("数据"), sendGroup), 1, 0);
    sendLayout->addWidget(m_dataEditor, 1, 1, 1, 4);
    sendLayout->addWidget(m_sendButton, 1, 5);
    sendLayout->setColumnStretch(1, 2);
    sendLayout->setColumnStretch(4, 1);
    pageLayout->addWidget(sendGroup);

    auto *monitorGroup = new QGroupBox(tr("CAN 帧监视器"), this);
    monitorGroup->setProperty("surfaceCard", true);
    auto *monitorLayout = new QVBoxLayout(monitorGroup);
    auto *monitorActions = new QHBoxLayout;
    auto *monitorHint = new QLabel(tr("最多保留最近 5000 帧"), monitorGroup);
    auto *clearButton = new QPushButton(tr("清空"), monitorGroup);
    clearButton->setObjectName(QStringLiteral("canClearFrames"));
    monitorActions->addWidget(monitorHint);
    monitorActions->addStretch();
    monitorActions->addWidget(clearButton);
    monitorLayout->addLayout(monitorActions);

    m_frameTable = new QTableView(monitorGroup);
    m_frameTable->setObjectName(QStringLiteral("canFrameTable"));
    m_frameTable->setModel(m_frameModel);
    m_frameTable->setAlternatingRowColors(true);
    m_frameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_frameTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_frameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_frameTable->verticalHeader()->setVisible(false);
    m_frameTable->horizontalHeader()->setStretchLastSection(true);
    m_frameTable->horizontalHeader()->setSectionResizeMode(CanFrameTableModel::TimeColumn, QHeaderView::ResizeToContents);
    m_frameTable->horizontalHeader()->setSectionResizeMode(CanFrameTableModel::DirectionColumn, QHeaderView::ResizeToContents);
    m_frameTable->horizontalHeader()->setSectionResizeMode(CanFrameTableModel::TypeColumn, QHeaderView::ResizeToContents);
    m_frameTable->horizontalHeader()->setSectionResizeMode(CanFrameTableModel::IdColumn, QHeaderView::ResizeToContents);
    m_frameTable->horizontalHeader()->setSectionResizeMode(CanFrameTableModel::DlcColumn, QHeaderView::ResizeToContents);
    m_frameTable->horizontalHeader()->setSectionResizeMode(CanFrameTableModel::DataColumn, QHeaderView::Stretch);
    monitorLayout->addWidget(m_frameTable, 1);
    pageLayout->addWidget(monitorGroup, 1);

    m_selfTestTimer->setSingleShot(true);
    m_heartbeatTimer->setSingleShot(true);

    connect(m_connectButton, &QPushButton::clicked, this, &CanMonitorWidget::toggleConnection);
    connect(m_sendButton, &QPushButton::clicked, this, &CanMonitorWidget::sendEditorFrame);
    connect(m_selfTestButton, &QPushButton::clicked, this, &CanMonitorWidget::startBoardSelfTest);
    connect(clearButton, &QPushButton::clicked, m_frameModel, &CanFrameTableModel::clear);
    connect(m_remoteCheck, &QCheckBox::toggled, this, [this](bool remote) {
        m_dataEditor->setEnabled(!remote);
        m_dlcEditor->setEnabled(remote);
    });
    connect(m_extendedCheck, &QCheckBox::toggled, this, [this](bool extended) {
        m_idEditor->setPlaceholderText(extended
                                           ? QStringLiteral("0x00000000 - 0x1FFFFFFF")
                                           : QStringLiteral("0x000 - 0x7FF"));
    });
    connect(m_session, &PcanBasicSession::connectionChanged, this, &CanMonitorWidget::updateConnectionUi);
    connect(m_session, &PcanBasicSession::frameReceived, this, &CanMonitorWidget::handleReceivedFrame);
    connect(m_session, &PcanBasicSession::frameSent, this, &CanMonitorWidget::handleSentFrame);
    connect(m_session, &PcanBasicSession::errorOccurred, this, [this](const QString &message) {
        if (m_selfTestStage != SelfTestStage::Idle)
            m_selfTestErrorSeen = true;
        setStateLabel(m_operationState, message, QStringLiteral("#b4232c"));
        m_connectionState->setToolTip(message);
    });
    connect(m_session, &PcanBasicSession::busStatusChanged, this,
            [this](quint32 status, const QString &description) {
                if (m_selfTestStage != SelfTestStage::Idle && status != 0U)
                    m_selfTestErrorSeen = true;
                if (status != 0U)
                    setStateLabel(m_busState, tr("错误：%1").arg(description), QStringLiteral("#b4232c"));
                else
                    setStateLabel(m_busState, tr("正常"), QStringLiteral("#147a42"));
                m_busState->setToolTip(description);
            });
    connect(m_selfTestTimer, &QTimer::timeout, this, [this] {
        if (m_selfTestStage == SelfTestStage::WaitingForHeartbeat)
            finishSelfTest(false, tr("FAIL：3 秒内未收到 F103 心跳帧 0x123"));
        else if (m_selfTestStage == SelfTestStage::WaitingForEcho)
            finishSelfTest(false, tr("FAIL：1 秒内未收到匹配的 0x322 回显"));
    });
    connect(m_heartbeatTimer, &QTimer::timeout, this, [this] {
        if (!m_session->isOpen())
            return;
        m_lastHeartbeatMs = 0;
        if (m_selfTestStage == SelfTestStage::Idle)
            setStateLabel(m_boardState, tr("心跳超时，开发板离线"), QStringLiteral("#b4232c"));
    });

    updateConnectionUi(false);
    updateCounters();
}

bool CanMonitorWidget::parseHexPayload(const QString &text, QByteArray *payload, QString *errorMessage)
{
    QString normalized = text.trimmed();
    normalized.remove(QRegularExpression(QStringLiteral("0[xX]")));
    normalized.remove(QRegularExpression(QStringLiteral("[\\s,;:_-]+")));
    if (normalized.size() > 16) {
        if (errorMessage)
            *errorMessage = tr("经典 CAN 数据不能超过 8 字节");
        return false;
    }
    if ((normalized.size() % 2) != 0
        || normalized.contains(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")))) {
        if (errorMessage)
            *errorMessage = tr("HEX 数据格式错误；每个字节必须包含两位十六进制数");
        return false;
    }
    if (payload)
        *payload = QByteArray::fromHex(normalized.toLatin1());
    return true;
}

bool CanMonitorWidget::parseCanId(const QString &text, quint32 *id)
{
    QString normalized = text.trimmed();
    if (normalized.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
        normalized.remove(0, 2);
    bool ok = false;
    const quint32 value = normalized.toUInt(&ok, 16);
    if (ok && id)
        *id = value;
    return ok;
}

void CanMonitorWidget::toggleConnection()
{
    if (m_session->isOpen()) {
        m_session->close();
        return;
    }

    const int channel = m_channelCombo->currentData().toInt();
    const int bitrate = m_bitrateCombo->currentData().toInt();
    if (!m_session->open(channel, bitrate)) {
        setStateLabel(m_connectionState, tr("连接失败"), QStringLiteral("#b4232c"));
        setStateLabel(m_operationState, m_session->lastError(), QStringLiteral("#b4232c"));
        m_connectionState->setToolTip(m_session->lastError());
        return;
    }
    m_apiPathLabel->setText(m_session->apiPath());
    m_apiPathLabel->setToolTip(m_session->apiPath());
}

void CanMonitorWidget::updateConnectionUi(bool connected)
{
    m_lastHeartbeatMs = 0;
    m_heartbeatTimer->stop();
    m_channelCombo->setEnabled(!connected);
    m_bitrateCombo->setEnabled(!connected);
    m_sendButton->setEnabled(connected);
    m_connectButton->setText(connected ? tr("断开") : tr("连接"));
    setStateLabel(m_connectionState,
                  connected ? tr("已连接") : tr("未连接"),
                  connected ? QStringLiteral("#147a42") : QStringLiteral("#65758a"));
    if (!connected) {
        setStateLabel(m_busState, tr("未知"), QStringLiteral("#65758a"));
        if (m_selfTestStage != SelfTestStage::Idle)
            finishSelfTest(false, tr("FAIL：PCAN 通道已断开"));
        else
            setStateLabel(m_boardState, tr("等待连接"), QStringLiteral("#65758a"));
    } else {
        setStateLabel(m_boardState, tr("等待心跳"), QStringLiteral("#9a6700"));
        setStateLabel(m_operationState, tr("PCAN 通道已连接"), QStringLiteral("#147a42"));
    }
}

void CanMonitorWidget::sendEditorFrame()
{
    quint32 id = 0;
    if (!parseCanId(m_idEditor->text(), &id)) {
        setStateLabel(m_operationState, tr("CAN ID 格式错误"), QStringLiteral("#b4232c"));
        return;
    }

    QByteArray payload;
    QString error;
    const bool remote = m_remoteCheck->isChecked();
    if (!remote && !parseHexPayload(m_dataEditor->text(), &payload, &error)) {
        setStateLabel(m_operationState, error, QStringLiteral("#b4232c"));
        return;
    }

    CanFrame frame;
    frame.id = id;
    frame.extended = m_extendedCheck->isChecked();
    frame.remote = remote;
    frame.payload = payload;
    frame.dlc = remote ? static_cast<quint8>(m_dlcEditor->value())
                       : static_cast<quint8>(payload.size());
    if (!m_session->sendFrame(frame))
        setStateLabel(m_operationState, m_session->lastError(), QStringLiteral("#b4232c"));
}

void CanMonitorWidget::handleReceivedFrame(const CanFrame &frame)
{
    m_frameModel->appendFrame(frame);
    ++m_receivedCount;
    updateCounters();
    m_frameTable->scrollToBottom();

    const bool heartbeat = !frame.extended && !frame.remote && frame.id == 0x123U
        && frame.dlc == 8U && frame.payload.size() == 8
        && frame.payload.first(4) == QByteArrayLiteral("F103");
    if (heartbeat) {
        const auto *bytes = reinterpret_cast<const uchar *>(frame.payload.constData());
        m_lastHeartbeatCounter = quint32(bytes[4]) | (quint32(bytes[5]) << 8U)
            | (quint32(bytes[6]) << 16U) | (quint32(bytes[7]) << 24U);
        m_lastHeartbeatMs = QDateTime::currentMSecsSinceEpoch();
        m_heartbeatTimer->start(2500);
        setStateLabel(m_boardState,
                      tr("在线，心跳 #%1").arg(m_lastHeartbeatCounter),
                      QStringLiteral("#147a42"));
        if (m_selfTestStage == SelfTestStage::WaitingForHeartbeat)
            sendSelfTestRequest();
    }

    if (m_selfTestStage == SelfTestStage::WaitingForEcho && !frame.extended
        && !frame.remote && frame.id == 0x322U && frame.dlc == 8U
        && frame.payload == m_expectedEcho) {
        m_selfTestTimer->stop();
        m_selfTestStage = SelfTestStage::VerifyingStatus;
        QTimer::singleShot(0, this, [this] {
            if (m_selfTestStage != SelfTestStage::VerifyingStatus)
                return;
            const quint32 status = m_session->busStatus();
            if (status != 0U || m_selfTestErrorSeen)
                finishSelfTest(false, tr("FAIL：回显正确，但 PCAN 状态为 0x%1")
                                          .arg(status, 8, 16, QLatin1Char('0')));
            else
                finishSelfTest(true, tr("PASS：500 kbit/s 心跳与 0x321 → 0x322 双向回显通过"));
        });
    }
}

void CanMonitorWidget::handleSentFrame(const CanFrame &frame)
{
    m_frameModel->appendFrame(frame);
    ++m_transmittedCount;
    updateCounters();
    m_frameTable->scrollToBottom();
    setStateLabel(m_operationState,
                  tr("已发送 0x%1，DLC %2")
                      .arg(frame.id, frame.extended ? 8 : 3, 16, QLatin1Char('0'))
                      .arg(frame.dlc),
                  QStringLiteral("#147a42"));
}

void CanMonitorWidget::startBoardSelfTest()
{
    if (m_selfTestStage != SelfTestStage::Idle)
        return;
    if (!m_session->isOpen()) {
        toggleConnection();
        if (!m_session->isOpen()) {
            finishSelfTest(false, tr("FAIL：无法打开 PCAN-USB"));
            return;
        }
    }
    if (m_session->bitrate() != 500000) {
        finishSelfTest(false, tr("FAIL：开发板自检必须选择 500 kbit/s"));
        return;
    }

    m_selfTestButton->setEnabled(false);
    m_selfTestErrorSeen = false;
    m_selfTestStage = SelfTestStage::WaitingForHeartbeat;
    setStateLabel(m_boardState, tr("自检：等待 0x123 心跳"), QStringLiteral("#9a6700"));
    if (QDateTime::currentMSecsSinceEpoch() - m_lastHeartbeatMs <= 2500)
        sendSelfTestRequest();
    else
        m_selfTestTimer->start(3000);
}

void CanMonitorWidget::sendSelfTestRequest()
{
    m_selfTestTimer->stop();
    const quint64 nonce = QRandomGenerator::global()->generate64();
    m_expectedEcho.resize(8);
    std::memcpy(m_expectedEcho.data(), &nonce, sizeof(nonce));

    CanFrame request;
    request.id = 0x321U;
    request.payload = m_expectedEcho;
    request.dlc = 8U;
    if (!m_session->sendFrame(request)) {
        finishSelfTest(false, tr("FAIL：0x321 发送失败：%1").arg(m_session->lastError()));
        return;
    }
    m_selfTestStage = SelfTestStage::WaitingForEcho;
    setStateLabel(m_boardState, tr("自检：等待 0x322 回显"), QStringLiteral("#9a6700"));
    m_selfTestTimer->start(1000);
}

void CanMonitorWidget::finishSelfTest(bool passed, const QString &message)
{
    m_selfTestTimer->stop();
    m_selfTestStage = SelfTestStage::Idle;
    m_selfTestButton->setEnabled(true);
    setStateLabel(m_boardState,
                  message,
                  passed ? QStringLiteral("#147a42") : QStringLiteral("#b4232c"));
    setStateLabel(m_operationState,
                  message,
                  passed ? QStringLiteral("#147a42") : QStringLiteral("#b4232c"));
    m_boardState->setToolTip(message);
}

void CanMonitorWidget::setStateLabel(QLabel *label,
                                     const QString &text,
                                     const QString &color)
{
    label->setText(text);
    label->setToolTip(text);
    label->setStyleSheet(QStringLiteral("color: %1; font-weight: 600;").arg(color));
}

void CanMonitorWidget::updateCounters()
{
    m_rxCountLabel->setText(QString::number(m_receivedCount));
    m_txCountLabel->setText(QString::number(m_transmittedCount));
}

} // namespace gucds
