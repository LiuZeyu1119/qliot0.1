#include "gucds/widgets/dtuconfigdialog.h"

#include "gucds/core/atprotocol.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

constexpr int kNetworkChannel = 1;
constexpr auto kDefaultHost = "43.139.170.206";
constexpr auto kDefaultMqttAccount = "518d41f0b635211f9f639aa596c9bf39";

QComboBox *valueCombo(QWidget *parent, const QList<QPair<QString, int>> &items)
{
    auto *combo = new QComboBox(parent);
    for (const auto &item : items)
        combo->addItem(item.first, item.second);
    combo->setMinimumWidth(190);
    return combo;
}

QLineEdit *textEdit(QWidget *parent, const QString &text = {})
{
    auto *edit = new QLineEdit(text, parent);
    edit->setMinimumWidth(390);
    return edit;
}

QSpinBox *numberEdit(QWidget *parent, int minimum, int maximum, int value, const QString &suffix = {})
{
    auto *spin = new QSpinBox(parent);
    spin->setRange(minimum, maximum);
    spin->setValue(value);
    spin->setMinimumWidth(190);
    spin->setSuffix(suffix);
    return spin;
}

QFormLayout *pageForm(QWidget *page)
{
    auto *form = new QFormLayout(page);
    form->setContentsMargins(22, 12, 22, 18);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    return form;
}

int comboValue(const QComboBox *combo)
{
    return combo ? combo->currentData().toInt() : 0;
}

bool comboFlag(const QComboBox *combo)
{
    return comboValue(combo) != 0;
}

QString typedData(const QComboBox *type, const QLineEdit *data)
{
    return comboValue(type) >= 2 && data ? data->text() : QString();
}

} // namespace

DtuConfigDialog::DtuConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("DTU 网络通道配置"));
    setModal(true);
    resize(760, 720);
    setMinimumSize(660, 580);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 12);
    layout->setSpacing(10);

    auto *common = new QWidget(this);
    auto *commonForm = pageForm(common);
    commonForm->setContentsMargins(22, 0, 22, 8);

    m_enabled = valueCombo(common, {{tr("启动"), 1}, {tr("停止"), 0}});
    m_enabled->setObjectName(QStringLiteral("dtuEnabled"));
    m_protocol = new QComboBox(common);
    m_protocol->setObjectName(QStringLiteral("dtuProtocol"));
    m_protocol->setMinimumWidth(190);
    m_protocol->addItem(QStringLiteral("MQTT"), QStringLiteral("mqtt"));
    m_protocol->addItem(QStringLiteral("HTTP"), QStringLiteral("http"));
    m_protocol->addItem(QStringLiteral("TCP"), QStringLiteral("tcp"));
    m_protocol->addItem(QStringLiteral("UDP"), QStringLiteral("udp"));
    m_protocol->addItem(QStringLiteral("WebSocket"), QStringLiteral("webs"));
    m_serialChannel = new QComboBox(common);
    m_serialChannel->setObjectName(QStringLiteral("dtuSerialChannel"));
    m_serialChannel->setMinimumWidth(190);
    m_serialChannel->addItem(QStringLiteral("UART"), QStringLiteral("uart"));
    m_serialChannel->addItem(QStringLiteral("TTL UART"), QStringLiteral("ttluart"));
    m_serialChannel->addItem(QStringLiteral("RS232"), QStringLiteral("rs232"));
    m_serialChannel->addItem(QStringLiteral("RS485"), QStringLiteral("rs485"));
    m_serialChannel->addItem(QStringLiteral("UART 2"), QStringLiteral("uart_2"));
    m_serialChannel->addItem(QStringLiteral("RS485 2"), QStringLiteral("rs485_2"));
    m_serialChannel->addItem(QStringLiteral("RS485 3"), QStringLiteral("rs485_3"));
    commonForm->addRow(tr("是否启动"), m_enabled);
    commonForm->addRow(tr("网络通信协议"), m_protocol);
    commonForm->addRow(tr("绑定通讯串口"), m_serialChannel);
    layout->addWidget(common);

    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator);

    m_protocolPages = new QStackedWidget(this);
    m_protocolPages->setObjectName(QStringLiteral("dtuProtocolPages"));

    m_mqttPage = new QWidget(m_protocolPages);
    m_mqttPage->setObjectName(QStringLiteral("dtuMqttPage"));
    m_mqttForm = pageForm(m_mqttPage);
    m_mqttHeartbeat = numberEdit(m_mqttPage, 60, 300, 120, tr(" 秒"));
    m_mqttHeartbeat->setObjectName(QStringLiteral("dtuMqttHeartbeat"));
    m_mqttHost = textEdit(m_mqttPage, QString::fromLatin1(kDefaultHost));
    m_mqttHost->setObjectName(QStringLiteral("dtuMqttHost"));
    m_mqttPort = numberEdit(m_mqttPage, 1, 65535, 1002);
    m_mqttPort->setObjectName(QStringLiteral("dtuMqttPort"));
    m_mqttClientId = textEdit(m_mqttPage, QString::fromLatin1(kDefaultMqttAccount));
    m_mqttClientId->setObjectName(QStringLiteral("dtuMqttClientId"));
    m_mqttUsername = textEdit(m_mqttPage, QString::fromLatin1(kDefaultMqttAccount));
    m_mqttUsername->setObjectName(QStringLiteral("dtuMqttUsername"));
    m_mqttPassword = textEdit(m_mqttPage, QStringLiteral("88888888"));
    m_mqttPassword->setObjectName(QStringLiteral("dtuMqttPassword"));
    m_mqttPassword->setEchoMode(QLineEdit::Password);
    m_mqttVersion = valueCombo(m_mqttPage, {{QStringLiteral("3.1.1"), 1}, {QStringLiteral("3.1"), 0}});
    m_mqttVersion->setObjectName(QStringLiteral("dtuMqttVersion"));
    m_mqttCleanSession = valueCombo(m_mqttPage, {{tr("离线自动销毁"), 1}, {tr("持久会话"), 0}});
    m_mqttCleanSession->setObjectName(QStringLiteral("dtuMqttCleanSession"));
    m_mqttRetain = valueCombo(m_mqttPage, {{tr("否"), 0}, {tr("是"), 1}});
    m_mqttRetain->setObjectName(QStringLiteral("dtuMqttRetain"));
    m_mqttSubscribeQos = valueCombo(m_mqttPage, {{QStringLiteral("0"), 0}, {QStringLiteral("1"), 1}, {QStringLiteral("2"), 2}});
    m_mqttSubscribeQos->setObjectName(QStringLiteral("dtuMqttSubscribeQos"));
    m_mqttPublishQos = valueCombo(m_mqttPage, {{QStringLiteral("0"), 0}, {QStringLiteral("1"), 1}, {QStringLiteral("2"), 2}});
    m_mqttPublishQos->setObjectName(QStringLiteral("dtuMqttPublishQos"));
    m_mqttSubscribeTopic = textEdit(m_mqttPage, QStringLiteral("%1/down").arg(QString::fromLatin1(kDefaultMqttAccount)));
    m_mqttSubscribeTopic->setObjectName(QStringLiteral("dtuMqttSubscribeTopic"));
    m_mqttPublishTopic = textEdit(m_mqttPage, QStringLiteral("%1/up").arg(QString::fromLatin1(kDefaultMqttAccount)));
    m_mqttPublishTopic->setObjectName(QStringLiteral("dtuMqttPublishTopic"));
    m_mqttWillEnabled = valueCombo(m_mqttPage, {{tr("否"), 0}, {tr("是"), 1}});
    m_mqttWillEnabled->setObjectName(QStringLiteral("dtuMqttWillEnabled"));
    m_mqttWillQos = valueCombo(m_mqttPage, {{QStringLiteral("0"), 0}, {QStringLiteral("1"), 1}, {QStringLiteral("2"), 2}});
    m_mqttWillQos->setObjectName(QStringLiteral("dtuMqttWillQos"));
    m_mqttWillRetain = valueCombo(m_mqttPage, {{tr("否"), 0}, {tr("是"), 1}});
    m_mqttWillRetain->setObjectName(QStringLiteral("dtuMqttWillRetain"));
    m_mqttWillTopic = textEdit(m_mqttPage, QStringLiteral("/will"));
    m_mqttWillTopic->setObjectName(QStringLiteral("dtuMqttWillTopic"));
    m_mqttWillPayload = textEdit(m_mqttPage, QStringLiteral("offline"));
    m_mqttWillPayload->setObjectName(QStringLiteral("dtuMqttWillPayload"));
    m_mqttRegistrationType = valueCombo(m_mqttPage, {{tr("不发送"), 0}, {tr("固定格式"), 1}, {tr("16进制"), 2}, {tr("字符串"), 3}, {tr("自定义函数"), 4}});
    m_mqttRegistrationType->setObjectName(QStringLiteral("dtuMqttRegistrationType"));
    m_mqttRegistrationData = textEdit(m_mqttPage);
    m_mqttRegistrationData->setObjectName(QStringLiteral("dtuMqttRegistrationData"));
    m_mqttIpv6 = valueCombo(m_mqttPage, {{tr("否"), 0}, {tr("是"), 1}});
    m_mqttIpv6->setObjectName(QStringLiteral("dtuMqttIpv6"));
    m_mqttSsl = valueCombo(m_mqttPage, {{tr("不加密"), 0}, {tr("无证书加密"), 1}, {tr("有证书加密"), 2}});
    m_mqttSsl->setObjectName(QStringLiteral("dtuMqttSsl"));
    m_mqttForm->addRow(tr("心跳包发送间隔时间"), m_mqttHeartbeat);
    m_mqttForm->addRow(tr("服务器地址"), m_mqttHost);
    m_mqttForm->addRow(tr("服务器端口"), m_mqttPort);
    m_mqttForm->addRow(tr("登录客户端 ID"), m_mqttClientId);
    m_mqttForm->addRow(tr("登录用户名"), m_mqttUsername);
    m_mqttForm->addRow(tr("登录密码"), m_mqttPassword);
    m_mqttForm->addRow(tr("协议版本"), m_mqttVersion);
    m_mqttForm->addRow(tr("清除会话"), m_mqttCleanSession);
    m_mqttForm->addRow(tr("持久消息"), m_mqttRetain);
    m_mqttForm->addRow(tr("订阅 QoS"), m_mqttSubscribeQos);
    m_mqttForm->addRow(tr("发布 QoS"), m_mqttPublishQos);
    m_mqttForm->addRow(tr("订阅消息主题"), m_mqttSubscribeTopic);
    m_mqttForm->addRow(tr("发布消息主题"), m_mqttPublishTopic);
    m_mqttForm->addRow(tr("设置遗嘱"), m_mqttWillEnabled);
    m_mqttForm->addRow(tr("遗嘱 QoS"), m_mqttWillQos);
    m_mqttForm->addRow(tr("遗嘱持久消息"), m_mqttWillRetain);
    m_mqttForm->addRow(tr("遗嘱 Topic"), m_mqttWillTopic);
    m_mqttForm->addRow(tr("遗嘱内容"), m_mqttWillPayload);
    m_mqttForm->addRow(tr("登录注册信息"), m_mqttRegistrationType);
    m_mqttForm->addRow(tr("登录注册数据"), m_mqttRegistrationData);
    m_mqttForm->addRow(tr("支持 IPv6"), m_mqttIpv6);
    m_mqttForm->addRow(tr("支持 SSL"), m_mqttSsl);
    m_protocolPages->addWidget(m_mqttPage);

    m_httpPage = new QWidget(m_protocolPages);
    m_httpPage->setObjectName(QStringLiteral("dtuHttpPage"));
    m_httpForm = pageForm(m_httpPage);
    m_httpMethod = valueCombo(m_httpPage, {{QStringLiteral("POST"), 1}, {QStringLiteral("GET"), 0}});
    m_httpMethod->setObjectName(QStringLiteral("dtuHttpMethod"));
    m_httpHost = textEdit(m_httpPage, QStringLiteral("http://%1").arg(QString::fromLatin1(kDefaultHost)));
    m_httpHost->setObjectName(QStringLiteral("dtuHttpHost"));
    m_httpPort = numberEdit(m_httpPage, 1, 65535, 1000);
    m_httpPort->setObjectName(QStringLiteral("dtuHttpPort"));
    m_httpPath = textEdit(m_httpPage, QStringLiteral("/"));
    m_httpPath->setObjectName(QStringLiteral("dtuHttpPath"));
    m_httpTimeout = numberEdit(m_httpPage, 1, 300, 30, tr(" 秒"));
    m_httpTimeout->setObjectName(QStringLiteral("dtuHttpTimeout"));
    m_httpCustomHeaderEnabled = valueCombo(m_httpPage, {{tr("不添加"), 0}, {tr("添加"), 1}});
    m_httpCustomHeaderEnabled->setObjectName(QStringLiteral("dtuHttpCustomHeaderEnabled"));
    m_httpCustomHeader = textEdit(m_httpPage);
    m_httpCustomHeader->setObjectName(QStringLiteral("dtuHttpCustomHeader"));
    m_httpResponseFilter = valueCombo(m_httpPage, {{tr("不过滤"), 0}, {tr("过滤"), 1}});
    m_httpResponseFilter->setObjectName(QStringLiteral("dtuHttpResponseFilter"));
    m_httpRegistrationType = valueCombo(m_httpPage, {{tr("不发送"), 0}, {tr("固定格式"), 1}, {tr("16进制"), 2}, {tr("字符串"), 3}, {tr("自定义函数"), 4}});
    m_httpRegistrationType->setObjectName(QStringLiteral("dtuHttpRegistrationType"));
    m_httpRegistrationData = textEdit(m_httpPage);
    m_httpRegistrationData->setObjectName(QStringLiteral("dtuHttpRegistrationData"));
    m_httpIpv6 = valueCombo(m_httpPage, {{tr("否"), 0}, {tr("是"), 1}});
    m_httpIpv6->setObjectName(QStringLiteral("dtuHttpIpv6"));
    m_httpSsl = valueCombo(m_httpPage, {{tr("不加密"), 0}, {tr("无证书加密"), 1}, {tr("有证书加密"), 2}});
    m_httpSsl->setObjectName(QStringLiteral("dtuHttpSsl"));
    m_httpForm->addRow(tr("请求方法"), m_httpMethod);
    m_httpForm->addRow(tr("服务器地址"), m_httpHost);
    m_httpForm->addRow(tr("服务器端口"), m_httpPort);
    m_httpForm->addRow(tr("请求 URL"), m_httpPath);
    m_httpForm->addRow(tr("等待超时时间"), m_httpTimeout);
    m_httpForm->addRow(tr("是否自定义头部"), m_httpCustomHeaderEnabled);
    m_httpForm->addRow(tr("自定义头部数据"), m_httpCustomHeader);
    m_httpForm->addRow(tr("返回数据过滤"), m_httpResponseFilter);
    m_httpForm->addRow(tr("登录注册信息"), m_httpRegistrationType);
    m_httpForm->addRow(tr("登录注册数据"), m_httpRegistrationData);
    m_httpForm->addRow(tr("支持 IPv6"), m_httpIpv6);
    m_httpForm->addRow(tr("支持 SSL"), m_httpSsl);
    m_protocolPages->addWidget(m_httpPage);

    m_socketPage = new QWidget(m_protocolPages);
    m_socketPage->setObjectName(QStringLiteral("dtuSocketPage"));
    m_socketForm = pageForm(m_socketPage);
    m_socketHeartbeatEnabled = valueCombo(m_socketPage, {{tr("开"), 1}, {tr("关"), 0}});
    m_socketHeartbeatEnabled->setObjectName(QStringLiteral("dtuSocketHeartbeatEnabled"));
    m_socketHeartbeatType = valueCombo(m_socketPage, {{QStringLiteral("HEX"), 0}, {tr("字符串"), 1}});
    m_socketHeartbeatType->setObjectName(QStringLiteral("dtuSocketHeartbeatType"));
    m_socketHeartbeatData = textEdit(m_socketPage, QStringLiteral("00"));
    m_socketHeartbeatData->setObjectName(QStringLiteral("dtuSocketHeartbeatData"));
    m_socketHeartbeatInterval = numberEdit(m_socketPage, 60, 86400, 60, tr(" 秒"));
    m_socketHeartbeatInterval->setObjectName(QStringLiteral("dtuSocketHeartbeatInterval"));
    m_socketHost = textEdit(m_socketPage);
    m_socketHost->setObjectName(QStringLiteral("dtuSocketHost"));
    m_socketPort = numberEdit(m_socketPage, 1, 65535, 1000);
    m_socketPort->setObjectName(QStringLiteral("dtuSocketPort"));
    const QList<QPair<QString, int>> dataTypes = {{tr("不发送"), 0}, {QStringLiteral("IMEI"), 1}, {QStringLiteral("HEX"), 2}, {tr("字符串"), 3}};
    m_socketPrefixType = valueCombo(m_socketPage, dataTypes);
    m_socketPrefixType->setObjectName(QStringLiteral("dtuSocketPrefixType"));
    m_socketPrefixData = textEdit(m_socketPage);
    m_socketPrefixData->setObjectName(QStringLiteral("dtuSocketPrefixData"));
    m_socketSuffixType = valueCombo(m_socketPage, dataTypes);
    m_socketSuffixType->setObjectName(QStringLiteral("dtuSocketSuffixType"));
    m_socketSuffixData = textEdit(m_socketPage);
    m_socketSuffixData->setObjectName(QStringLiteral("dtuSocketSuffixData"));
    m_socketRegistrationType = valueCombo(m_socketPage, {{tr("不发送"), 0}, {tr("固定格式"), 1}, {tr("16进制"), 2}, {tr("字符串"), 3}, {tr("自定义函数"), 4}});
    m_socketRegistrationType->setObjectName(QStringLiteral("dtuSocketRegistrationType"));
    m_socketRegistrationData = textEdit(m_socketPage);
    m_socketRegistrationData->setObjectName(QStringLiteral("dtuSocketRegistrationData"));
    m_socketIpv6 = valueCombo(m_socketPage, {{tr("否"), 0}, {tr("是"), 1}});
    m_socketIpv6->setObjectName(QStringLiteral("dtuSocketIpv6"));
    m_socketSsl = valueCombo(m_socketPage, {{tr("不加密"), 0}, {tr("无证书加密"), 1}, {tr("有证书加密"), 2}});
    m_socketSsl->setObjectName(QStringLiteral("dtuSocketSsl"));
    m_socketForm->addRow(tr("心跳包开关"), m_socketHeartbeatEnabled);
    m_socketForm->addRow(tr("心跳包数据类型"), m_socketHeartbeatType);
    m_socketForm->addRow(tr("心跳包数据"), m_socketHeartbeatData);
    m_socketForm->addRow(tr("心跳包发送间隔时间"), m_socketHeartbeatInterval);
    m_socketForm->addRow(tr("服务器地址"), m_socketHost);
    m_socketForm->addRow(tr("服务器端口"), m_socketPort);
    m_socketForm->addRow(tr("数据前置字段"), m_socketPrefixType);
    m_socketForm->addRow(tr("前置字段数据"), m_socketPrefixData);
    m_socketForm->addRow(tr("数据后置字段"), m_socketSuffixType);
    m_socketForm->addRow(tr("后置字段数据"), m_socketSuffixData);
    m_socketForm->addRow(tr("登录注册信息"), m_socketRegistrationType);
    m_socketForm->addRow(tr("登录注册数据"), m_socketRegistrationData);
    m_socketForm->addRow(tr("支持 IPv6"), m_socketIpv6);
    m_socketForm->addRow(tr("支持 SSL"), m_socketSsl);
    m_protocolPages->addWidget(m_socketPage);

    m_webSocketPage = new QWidget(m_protocolPages);
    m_webSocketPage->setObjectName(QStringLiteral("dtuWebSocketPage"));
    m_webSocketForm = pageForm(m_webSocketPage);
    m_webSocketUrl = textEdit(m_webSocketPage);
    m_webSocketUrl->setObjectName(QStringLiteral("dtuWebSocketUrl"));
    m_webSocketIpv6 = valueCombo(m_webSocketPage, {{tr("否"), 0}, {tr("是"), 1}});
    m_webSocketIpv6->setObjectName(QStringLiteral("dtuWebSocketIpv6"));
    m_webSocketCustomHeaderEnabled = valueCombo(m_webSocketPage, {{tr("不添加"), 0}, {tr("添加"), 1}});
    m_webSocketCustomHeaderEnabled->setObjectName(QStringLiteral("dtuWebSocketCustomHeaderEnabled"));
    m_webSocketCustomHeader = textEdit(m_webSocketPage);
    m_webSocketCustomHeader->setObjectName(QStringLiteral("dtuWebSocketCustomHeader"));
    m_webSocketHeartbeatEnabled = valueCombo(m_webSocketPage, {{tr("开"), 1}, {tr("关"), 0}});
    m_webSocketHeartbeatEnabled->setObjectName(QStringLiteral("dtuWebSocketHeartbeatEnabled"));
    m_webSocketHeartbeatType = valueCombo(m_webSocketPage, {{QStringLiteral("HEX"), 0}, {tr("字符串"), 1}});
    m_webSocketHeartbeatType->setObjectName(QStringLiteral("dtuWebSocketHeartbeatType"));
    m_webSocketHeartbeatData = textEdit(m_webSocketPage, QStringLiteral("00"));
    m_webSocketHeartbeatData->setObjectName(QStringLiteral("dtuWebSocketHeartbeatData"));
    m_webSocketHeartbeatInterval = numberEdit(m_webSocketPage, 1, 86400, 20, tr(" 秒"));
    m_webSocketHeartbeatInterval->setObjectName(QStringLiteral("dtuWebSocketHeartbeatInterval"));
    m_webSocketPrefixType = valueCombo(m_webSocketPage, dataTypes);
    m_webSocketPrefixType->setObjectName(QStringLiteral("dtuWebSocketPrefixType"));
    m_webSocketPrefixData = textEdit(m_webSocketPage);
    m_webSocketPrefixData->setObjectName(QStringLiteral("dtuWebSocketPrefixData"));
    m_webSocketSuffixType = valueCombo(m_webSocketPage, dataTypes);
    m_webSocketSuffixType->setObjectName(QStringLiteral("dtuWebSocketSuffixType"));
    m_webSocketSuffixData = textEdit(m_webSocketPage);
    m_webSocketSuffixData->setObjectName(QStringLiteral("dtuWebSocketSuffixData"));
    m_webSocketRegistrationType = valueCombo(m_webSocketPage, {{tr("不发送"), 0}, {tr("固定格式"), 1}, {tr("16进制"), 2}, {tr("字符串"), 3}});
    m_webSocketRegistrationType->setObjectName(QStringLiteral("dtuWebSocketRegistrationType"));
    m_webSocketRegistrationData = textEdit(m_webSocketPage);
    m_webSocketRegistrationData->setObjectName(QStringLiteral("dtuWebSocketRegistrationData"));
    m_webSocketForm->addRow(tr("服务器地址"), m_webSocketUrl);
    m_webSocketForm->addRow(tr("支持 IPv6"), m_webSocketIpv6);
    m_webSocketForm->addRow(tr("是否自定义头部"), m_webSocketCustomHeaderEnabled);
    m_webSocketForm->addRow(tr("自定义头部数据"), m_webSocketCustomHeader);
    m_webSocketForm->addRow(tr("心跳包开关"), m_webSocketHeartbeatEnabled);
    m_webSocketForm->addRow(tr("心跳包数据类型"), m_webSocketHeartbeatType);
    m_webSocketForm->addRow(tr("心跳包数据"), m_webSocketHeartbeatData);
    m_webSocketForm->addRow(tr("心跳包发送间隔时间"), m_webSocketHeartbeatInterval);
    m_webSocketForm->addRow(tr("数据前置字段"), m_webSocketPrefixType);
    m_webSocketForm->addRow(tr("前置字段数据"), m_webSocketPrefixData);
    m_webSocketForm->addRow(tr("数据后置字段"), m_webSocketSuffixType);
    m_webSocketForm->addRow(tr("后置字段数据"), m_webSocketSuffixData);
    m_webSocketForm->addRow(tr("登录注册信息"), m_webSocketRegistrationType);
    m_webSocketForm->addRow(tr("登录注册数据"), m_webSocketRegistrationData);
    m_protocolPages->addWidget(m_webSocketPage);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(QStringLiteral("dtuProtocolScrollArea"));
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget(m_protocolPages);
    layout->addWidget(scrollArea, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_saveButton = buttons->addButton(tr("保存参数"), QDialogButtonBox::AcceptRole);
    m_saveButton->setObjectName(QStringLiteral("dtuSave"));
    auto *cancelButton = buttons->button(QDialogButtonBox::Cancel);
    cancelButton->setText(tr("返回"));
    cancelButton->setObjectName(QStringLiteral("dtuCancel"));
    connect(buttons, &QDialogButtonBox::accepted, this, &DtuConfigDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &DtuConfigDialog::reject);
    layout->addWidget(buttons);

    connect(m_enabled, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateEnabledState);
    connect(m_protocol, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateProtocolPage);
    connect(m_mqttWillEnabled, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateMqttRows);
    connect(m_mqttRegistrationType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateMqttRows);
    connect(m_httpCustomHeaderEnabled, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateHttpRows);
    connect(m_httpRegistrationType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateHttpRows);
    connect(m_socketHeartbeatEnabled, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateSocketRows);
    connect(m_socketPrefixType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateSocketRows);
    connect(m_socketSuffixType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateSocketRows);
    connect(m_socketRegistrationType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateSocketRows);
    connect(m_webSocketCustomHeaderEnabled, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateWebSocketRows);
    connect(m_webSocketHeartbeatEnabled, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateWebSocketRows);
    connect(m_webSocketPrefixType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateWebSocketRows);
    connect(m_webSocketSuffixType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateWebSocketRows);
    connect(m_webSocketRegistrationType, &QComboBox::currentIndexChanged, this, &DtuConfigDialog::updateWebSocketRows);

    updateProtocolPage();
    updateEnabledState();
    updateMqttRows();
    updateHttpRows();
    updateSocketRows();
    updateWebSocketRows();
}

QString DtuConfigDialog::configCommand() const
{
    if (!channelEnabled())
        return gucds::AtProtocol::buildDeleteDtuChannel(kNetworkChannel);

    const QString key = protocol();
    if (key == QStringLiteral("mqtt")) {
        gucds::DtuMqttConfig config;
        config.channel = kNetworkChannel;
        config.serialChannel = serialChannel();
        config.heartbeatSeconds = m_mqttHeartbeat->value();
        config.host = m_mqttHost->text();
        config.port = m_mqttPort->value();
        config.clientId = m_mqttClientId->text();
        config.username = m_mqttUsername->text();
        config.password = m_mqttPassword->text();
        config.protocolVersion = comboValue(m_mqttVersion);
        config.cleanSession = comboFlag(m_mqttCleanSession);
        config.retain = comboFlag(m_mqttRetain);
        config.subscribeQos = comboValue(m_mqttSubscribeQos);
        config.publishQos = comboValue(m_mqttPublishQos);
        config.subscribeTopic = m_mqttSubscribeTopic->text();
        config.publishTopic = m_mqttPublishTopic->text();
        config.willEnabled = comboFlag(m_mqttWillEnabled);
        config.willQos = comboValue(m_mqttWillQos);
        config.willRetain = comboFlag(m_mqttWillRetain);
        config.willTopic = m_mqttWillTopic->text();
        config.willPayload = m_mqttWillPayload->text();
        config.registrationType = comboValue(m_mqttRegistrationType);
        config.registrationData = typedData(m_mqttRegistrationType, m_mqttRegistrationData);
        config.ipv6 = comboFlag(m_mqttIpv6);
        config.sslMode = comboValue(m_mqttSsl);
        return gucds::AtProtocol::buildDtuMqttConfig(config);
    }
    if (key == QStringLiteral("http")) {
        gucds::DtuHttpConfig config;
        config.channel = kNetworkChannel;
        config.serialChannel = serialChannel();
        config.host = m_httpHost->text();
        config.port = m_httpPort->value();
        config.requestMethod = comboValue(m_httpMethod);
        config.path = m_httpPath->text();
        config.timeoutSeconds = m_httpTimeout->value();
        config.customHeaderEnabled = comboFlag(m_httpCustomHeaderEnabled);
        config.customHeader = m_httpCustomHeader->text();
        config.responseFilterEnabled = comboFlag(m_httpResponseFilter);
        config.registrationType = comboValue(m_httpRegistrationType);
        config.registrationData = typedData(m_httpRegistrationType, m_httpRegistrationData);
        config.ipv6 = comboFlag(m_httpIpv6);
        config.sslMode = comboValue(m_httpSsl);
        return gucds::AtProtocol::buildDtuHttpConfig(config);
    }
    if (key == QStringLiteral("tcp") || key == QStringLiteral("udp")) {
        gucds::DtuSocketConfig config;
        config.protocol = key;
        config.channel = kNetworkChannel;
        config.serialChannel = serialChannel();
        config.heartbeatEnabled = comboFlag(m_socketHeartbeatEnabled);
        config.heartbeatDataType = comboValue(m_socketHeartbeatType);
        config.heartbeatData = m_socketHeartbeatData->text();
        config.heartbeatSeconds = m_socketHeartbeatInterval->value();
        config.host = m_socketHost->text();
        config.port = m_socketPort->value();
        config.prefixType = comboValue(m_socketPrefixType);
        config.prefixData = typedData(m_socketPrefixType, m_socketPrefixData);
        config.suffixType = comboValue(m_socketSuffixType);
        config.suffixData = typedData(m_socketSuffixType, m_socketSuffixData);
        config.registrationType = comboValue(m_socketRegistrationType);
        config.registrationData = typedData(m_socketRegistrationType, m_socketRegistrationData);
        config.ipv6 = comboFlag(m_socketIpv6);
        config.sslMode = comboValue(m_socketSsl);
        return gucds::AtProtocol::buildDtuSocketConfig(config);
    }

    gucds::DtuWebSocketConfig config;
    config.channel = kNetworkChannel;
    config.serialChannel = serialChannel();
    config.serverUrl = m_webSocketUrl->text();
    config.ipv6 = comboFlag(m_webSocketIpv6);
    config.customHeaderEnabled = comboFlag(m_webSocketCustomHeaderEnabled);
    config.customHeader = m_webSocketCustomHeader->text();
    config.heartbeatEnabled = comboFlag(m_webSocketHeartbeatEnabled);
    config.heartbeatDataType = comboValue(m_webSocketHeartbeatType);
    config.heartbeatData = m_webSocketHeartbeatData->text();
    config.heartbeatSeconds = m_webSocketHeartbeatInterval->value();
    config.prefixType = comboValue(m_webSocketPrefixType);
    config.prefixData = typedData(m_webSocketPrefixType, m_webSocketPrefixData);
    config.suffixType = comboValue(m_webSocketSuffixType);
    config.suffixData = typedData(m_webSocketSuffixType, m_webSocketSuffixData);
    config.registrationType = comboValue(m_webSocketRegistrationType);
    config.registrationData = typedData(m_webSocketRegistrationType, m_webSocketRegistrationData);
    return gucds::AtProtocol::buildDtuWebSocketConfig(config);
}

bool DtuConfigDialog::channelEnabled() const
{
    return comboFlag(m_enabled);
}

QString DtuConfigDialog::protocol() const
{
    return m_protocol->currentData().toString();
}

QString DtuConfigDialog::serialChannel() const
{
    return m_serialChannel->currentData().toString();
}

QString DtuConfigDialog::configurationSummary() const
{
    if (!channelEnabled())
        return tr("通道 1 | 已停用");
    QString endpoint;
    if (protocol() == QStringLiteral("mqtt"))
        endpoint = QStringLiteral("%1:%2").arg(m_mqttHost->text()).arg(m_mqttPort->value());
    else if (protocol() == QStringLiteral("http"))
        endpoint = QStringLiteral("%1:%2%3").arg(m_httpHost->text()).arg(m_httpPort->value()).arg(m_httpPath->text());
    else if (protocol() == QStringLiteral("webs"))
        endpoint = m_webSocketUrl->text();
    else
        endpoint = QStringLiteral("%1:%2").arg(m_socketHost->text()).arg(m_socketPort->value());
    return QStringLiteral("%1 | %2 | %3")
        .arg(m_protocol->currentText(), endpoint, m_serialChannel->currentText());
}

void DtuConfigDialog::accept()
{
    if (configCommand().isEmpty()) {
        QMessageBox::warning(this,
                             tr("DTU 参数"),
                             tr("请完整填写当前协议的必要参数；字段不能包含逗号或换行，命令总长度不能超过 254 字节。"));
        return;
    }
    QDialog::accept();
}

void DtuConfigDialog::updateProtocolPage()
{
    const QString key = protocol();
    if (key == QStringLiteral("mqtt"))
        m_protocolPages->setCurrentWidget(m_mqttPage);
    else if (key == QStringLiteral("http"))
        m_protocolPages->setCurrentWidget(m_httpPage);
    else if (key == QStringLiteral("webs"))
        m_protocolPages->setCurrentWidget(m_webSocketPage);
    else
        m_protocolPages->setCurrentWidget(m_socketPage);
    fitCurrentProtocolPage();
}

void DtuConfigDialog::updateEnabledState()
{
    const bool enabled = channelEnabled();
    m_protocol->setEnabled(enabled);
    m_serialChannel->setEnabled(enabled);
    m_protocolPages->setEnabled(enabled);
    m_saveButton->setText(enabled ? tr("保存参数") : tr("停用通道"));
}

void DtuConfigDialog::updateMqttRows()
{
    const bool willEnabled = comboFlag(m_mqttWillEnabled);
    m_mqttForm->setRowVisible(m_mqttWillQos, willEnabled);
    m_mqttForm->setRowVisible(m_mqttWillRetain, willEnabled);
    m_mqttForm->setRowVisible(m_mqttWillTopic, willEnabled);
    m_mqttForm->setRowVisible(m_mqttWillPayload, willEnabled);
    m_mqttForm->setRowVisible(m_mqttRegistrationData, comboValue(m_mqttRegistrationType) >= 2);
    fitCurrentProtocolPage();
}

void DtuConfigDialog::updateHttpRows()
{
    m_httpForm->setRowVisible(m_httpCustomHeader, comboFlag(m_httpCustomHeaderEnabled));
    m_httpForm->setRowVisible(m_httpRegistrationData, comboValue(m_httpRegistrationType) >= 2);
    fitCurrentProtocolPage();
}

void DtuConfigDialog::updateSocketRows()
{
    const bool heartbeatEnabled = comboFlag(m_socketHeartbeatEnabled);
    m_socketForm->setRowVisible(m_socketHeartbeatType, heartbeatEnabled);
    m_socketForm->setRowVisible(m_socketHeartbeatData, heartbeatEnabled);
    m_socketForm->setRowVisible(m_socketHeartbeatInterval, heartbeatEnabled);
    m_socketForm->setRowVisible(m_socketPrefixData, comboValue(m_socketPrefixType) >= 2);
    m_socketForm->setRowVisible(m_socketSuffixData, comboValue(m_socketSuffixType) >= 2);
    m_socketForm->setRowVisible(m_socketRegistrationData, comboValue(m_socketRegistrationType) >= 2);
    fitCurrentProtocolPage();
}

void DtuConfigDialog::updateWebSocketRows()
{
    m_webSocketForm->setRowVisible(m_webSocketCustomHeader, comboFlag(m_webSocketCustomHeaderEnabled));
    const bool heartbeatEnabled = comboFlag(m_webSocketHeartbeatEnabled);
    m_webSocketForm->setRowVisible(m_webSocketHeartbeatType, heartbeatEnabled);
    m_webSocketForm->setRowVisible(m_webSocketHeartbeatData, heartbeatEnabled);
    m_webSocketForm->setRowVisible(m_webSocketHeartbeatInterval, heartbeatEnabled);
    m_webSocketForm->setRowVisible(m_webSocketPrefixData, comboValue(m_webSocketPrefixType) >= 2);
    m_webSocketForm->setRowVisible(m_webSocketSuffixData, comboValue(m_webSocketSuffixType) >= 2);
    m_webSocketForm->setRowVisible(m_webSocketRegistrationData, comboValue(m_webSocketRegistrationType) >= 2);
    fitCurrentProtocolPage();
}

void DtuConfigDialog::fitCurrentProtocolPage()
{
    QWidget *page = m_protocolPages->currentWidget();
    if (!page)
        return;
    if (page->layout())
        page->layout()->activate();
    m_protocolPages->setFixedHeight(page->sizeHint().height());
}
