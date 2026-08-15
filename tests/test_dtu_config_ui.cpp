#include "gucds/widgets/dtuconfigdialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>
#include <QtTest/QtTest>

class DtuConfigUiTest : public QObject
{
    Q_OBJECT

private slots:
    void mqttUsesOnlyMqttParameters();
    void httpUsesOnlyHttpParameters();
    void buildsTcpAndUdpConfigurations();
    void buildsWebSocketConfiguration();
    void stoppingChannelBuildsDeleteCommand();
    void rejectsMissingRequiredParameter();
    void showsOnlyDependentParameters();
    void captureProtocolLayouts();
};

namespace {

void selectProtocol(DtuConfigDialog &dialog, const QString &protocol)
{
    auto *combo = dialog.findChild<QComboBox *>(QStringLiteral("dtuProtocol"));
    QVERIFY(combo);
    const int index = combo->findData(protocol);
    QVERIFY(index >= 0);
    combo->setCurrentIndex(index);
}

} // namespace

void DtuConfigUiTest::mqttUsesOnlyMqttParameters()
{
    DtuConfigDialog dialog;
    auto *pages = dialog.findChild<QStackedWidget *>(QStringLiteral("dtuProtocolPages"));
    auto *password = dialog.findChild<QLineEdit *>(QStringLiteral("dtuMqttPassword"));
    auto *cancelButton = dialog.findChild<QPushButton *>(QStringLiteral("dtuCancel"));
    QVERIFY(pages);
    QVERIFY(password);
    QVERIFY(cancelButton);
    QCOMPARE(cancelButton->text(), QStringLiteral("返回"));
    QCOMPARE(dialog.protocol(), QStringLiteral("mqtt"));
    QCOMPARE(pages->currentWidget()->objectName(), QStringLiteral("dtuMqttPage"));
    QCOMPARE(password->echoMode(), QLineEdit::Password);
    QCOMPARE(
        dialog.configCommand(),
        QStringLiteral("config,set,mqtt,1,uart,120,43.139.170.206,1002,518d41f0b635211f9f639aa596c9bf39,518d41f0b635211f9f639aa596c9bf39,88888888,1,1,0,0,0,518d41f0b635211f9f639aa596c9bf39/down,518d41f0b635211f9f639aa596c9bf39/up,0,0,0,0,0,0,0,0,0"));
}

void DtuConfigUiTest::httpUsesOnlyHttpParameters()
{
    DtuConfigDialog dialog;
    selectProtocol(dialog, QStringLiteral("http"));
    auto *pages = dialog.findChild<QStackedWidget *>(QStringLiteral("dtuProtocolPages"));
    auto *path = dialog.findChild<QLineEdit *>(QStringLiteral("dtuHttpPath"));
    QVERIFY(pages);
    QVERIFY(path);
    QCOMPARE(pages->currentWidget()->objectName(), QStringLiteral("dtuHttpPage"));
    path->setText(QStringLiteral("/0e348a917ab0c5e289aeb87cfce0a859"));
    QCOMPARE(
        dialog.configCommand(),
        QStringLiteral("config,set,http,1,uart,1,http://43.139.170.206,1000,/0e348a917ab0c5e289aeb87cfce0a859,30,0,0,0,0,0"));
}

void DtuConfigUiTest::buildsTcpAndUdpConfigurations()
{
    for (const QString &protocol : {QStringLiteral("tcp"), QStringLiteral("udp")}) {
        DtuConfigDialog dialog;
        selectProtocol(dialog, protocol);
        auto *pages = dialog.findChild<QStackedWidget *>(QStringLiteral("dtuProtocolPages"));
        auto *host = dialog.findChild<QLineEdit *>(QStringLiteral("dtuSocketHost"));
        QVERIFY(pages);
        QVERIFY(host);
        QCOMPARE(pages->currentWidget()->objectName(), QStringLiteral("dtuSocketPage"));
        host->setText(QStringLiteral("47.106.167.188"));
        QCOMPARE(
            dialog.configCommand(),
            QStringLiteral("config,set,%1,1,uart,1,0,00,60,47.106.167.188,1000,0,0,0,0,0,0,0,0")
                .arg(protocol));
    }
}

void DtuConfigUiTest::buildsWebSocketConfiguration()
{
    DtuConfigDialog dialog;
    selectProtocol(dialog, QStringLiteral("webs"));
    auto *pages = dialog.findChild<QStackedWidget *>(QStringLiteral("dtuProtocolPages"));
    auto *url = dialog.findChild<QLineEdit *>(QStringLiteral("dtuWebSocketUrl"));
    QVERIFY(pages);
    QVERIFY(url);
    QCOMPARE(pages->currentWidget()->objectName(), QStringLiteral("dtuWebSocketPage"));
    url->setText(QStringLiteral("ws://example.com/socket"));
    QCOMPARE(
        dialog.configCommand(),
        QStringLiteral("config,set,webs,1,uart,ws://example.com/socket,0,0,0,1,0,00,20,0,0,0,0,0,0"));
}

void DtuConfigUiTest::stoppingChannelBuildsDeleteCommand()
{
    DtuConfigDialog dialog;
    auto *enabled = dialog.findChild<QComboBox *>(QStringLiteral("dtuEnabled"));
    QVERIFY(enabled);
    enabled->setCurrentIndex(enabled->findData(0));
    QVERIFY(!dialog.channelEnabled());
    QCOMPARE(dialog.configCommand(), QStringLiteral("config,get,delnetchan,1"));
}

void DtuConfigUiTest::rejectsMissingRequiredParameter()
{
    DtuConfigDialog dialog;
    auto *publishTopic = dialog.findChild<QLineEdit *>(QStringLiteral("dtuMqttPublishTopic"));
    QVERIFY(publishTopic);
    publishTopic->clear();
    QVERIFY(dialog.configCommand().isEmpty());
}

void DtuConfigUiTest::showsOnlyDependentParameters()
{
    DtuConfigDialog dialog;

    auto *willEnabled = dialog.findChild<QComboBox *>(QStringLiteral("dtuMqttWillEnabled"));
    auto *willTopic = dialog.findChild<QLineEdit *>(QStringLiteral("dtuMqttWillTopic"));
    QVERIFY(willEnabled);
    QVERIFY(willTopic);
    QVERIFY(willTopic->isHidden());
    willEnabled->setCurrentIndex(willEnabled->findData(1));
    QVERIFY(!willTopic->isHidden());

    selectProtocol(dialog, QStringLiteral("http"));
    auto *customHeaderEnabled = dialog.findChild<QComboBox *>(QStringLiteral("dtuHttpCustomHeaderEnabled"));
    auto *customHeader = dialog.findChild<QLineEdit *>(QStringLiteral("dtuHttpCustomHeader"));
    QVERIFY(customHeaderEnabled);
    QVERIFY(customHeader);
    QVERIFY(customHeader->isHidden());
    customHeaderEnabled->setCurrentIndex(customHeaderEnabled->findData(1));
    QVERIFY(!customHeader->isHidden());

    selectProtocol(dialog, QStringLiteral("tcp"));
    auto *prefixType = dialog.findChild<QComboBox *>(QStringLiteral("dtuSocketPrefixType"));
    auto *prefixData = dialog.findChild<QLineEdit *>(QStringLiteral("dtuSocketPrefixData"));
    QVERIFY(prefixType);
    QVERIFY(prefixData);
    QVERIFY(prefixData->isHidden());
    prefixType->setCurrentIndex(prefixType->findData(2));
    QVERIFY(!prefixData->isHidden());
}

void DtuConfigUiTest::captureProtocolLayouts()
{
    const QString screenshotBase = qEnvironmentVariable("QLIOT_DTU_SCREENSHOT_BASE");
    if (screenshotBase.isEmpty())
        return;

    DtuConfigDialog dialog;
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto *scrollArea = dialog.findChild<QScrollArea *>(QStringLiteral("dtuProtocolScrollArea"));
    QVERIFY(scrollArea);
    for (const QString &protocol : {QStringLiteral("mqtt"),
                                    QStringLiteral("http"),
                                    QStringLiteral("tcp"),
                                    QStringLiteral("udp"),
                                    QStringLiteral("webs")}) {
        selectProtocol(dialog, protocol);
        scrollArea->verticalScrollBar()->setValue(0);
        QCoreApplication::processEvents();
        QVERIFY(dialog.grab().save(QStringLiteral("%1-%2.png").arg(screenshotBase, protocol)));
        if (scrollArea->verticalScrollBar()->maximum() > 0) {
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
            QCoreApplication::processEvents();
            QVERIFY(dialog.grab().save(QStringLiteral("%1-%2-bottom.png").arg(screenshotBase, protocol)));
        }
    }
}

QTEST_MAIN(DtuConfigUiTest)

#include "test_dtu_config_ui.moc"
