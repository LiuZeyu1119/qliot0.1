#include "applicationtranslator.h"
#include "mainwindow.h"

#include "gucds/core/labviewdatabase.h"
#include "gucds/core/devicecommunicationcontroller.h"
#include "gucds/core/measurementtablemodel.h"
#include "gucds/core/serialsession.h"
#include "gucds/core/virtualdevice.h"
#include "gucds/core/virtualmodbusclient.h"
#include "gucds/widgets/dtuconfigdialog.h"
#include "gucds/widgets/frequencytensionparameterdialog.h"
#include "gucds/widgets/productmanagementdialog.h"
#include "spectrumplotwidget.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QPalette>
#include <QPushButton>
#include <QSettings>
#include <QScopeGuard>
#include <QSpinBox>
#include <QStyleHints>
#include <QTabWidget>
#include <QTableView>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeWidget>
#include <QtTest/QtTest>

class MainWindowUiTest : public QObject
{
    Q_OBJECT

private slots:
    void exposesImplementedOperations();
    void localFormActionsMutateFields();
    void frequencyTensionDeviceSwitchesTableAndPlot();
    void frequencyStartButtonTriggersCompactCommandMeasurement();
    void frequencyStartButtonStreamsAutomaticMeasurementsUntilStopped();
    void frequencyParameterManagerOpensOffline();
    void languageMenuIsBilingualAndRequiresRestart();
    void languageCanSwitchBothDirections();
    void englishUiKeepsProtocolKeysStable();
    void lightThemeOverridesDarkSystemPalette();
};

void MainWindowUiTest::exposesImplementedOperations()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QStringList actions = {
        QStringLiteral("action:MCU卡:读MCU"),
        QStringLiteral("action:MCU卡:写MCU"),
        QStringLiteral("action:MCU卡:重启设备"),
        QStringLiteral("action:LoRa设置:读LoRa"),
        QStringLiteral("action:LoRa设置:写LoRa"),
        QStringLiteral("action:DTU主卡:网络配置"),
        QStringLiteral("action:DTU主卡:检测状态"),
        QStringLiteral("action:标定:添加"),
        QStringLiteral("action:标定:保存"),
        QStringLiteral("action:标定:删除"),
        QStringLiteral("action:标定:测量"),
        QStringLiteral("action:标定:修改"),
        QStringLiteral("action:总线设备管理器:保存"),
        QStringLiteral("action:总线设备管理器:修改"),
        QStringLiteral("action:总线设备管理器:删除"),
        QStringLiteral("action:总线设备管理器:开始测试"),
        QStringLiteral("action:DTU网络:网络配置"),
        QStringLiteral("action:DTU网络:检测状态"),
        QStringLiteral("action:AT管理器:发送"),
        QStringLiteral("action:AT管理器:清除"),
    };
    for (const QString &objectName : actions)
        QVERIFY2(window.findChild<QPushButton *>(objectName), qPrintable(objectName));

    auto *configSections = window.findChild<QTabWidget *>(QStringLiteral("deviceConfigSections"));
    QVERIFY(configSections);
    QCOMPARE(configSections->count(), 4);
    QCOMPARE(configSections->tabText(0), QStringLiteral("设备参数"));
    QCOMPARE(configSections->tabText(1), QStringLiteral("MCU模块"));
    QCOMPARE(configSections->tabText(2), QStringLiteral("LoRa设置"));
    QCOMPARE(configSections->tabText(3), QStringLiteral("DTU模块"));

    auto *sideTools = window.findChild<QTabWidget *>(QStringLiteral("sideTools"));
    QVERIFY(sideTools);
    QCOMPARE(sideTools->count(), 2);
    QCOMPARE(sideTools->tabText(0), QStringLiteral("设备库"));
    QCOMPARE(sideTools->tabText(1), QStringLiteral("通信控制"));

    auto *mainTabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
    QVERIFY(mainTabs);
    QCOMPARE(mainTabs->count(), 5);
    QCOMPARE(mainTabs->tabText(4), QStringLiteral("CAN总线"));
    QVERIFY(window.findChild<QWidget *>(QStringLiteral("canMonitorWidget")));
    auto *canBitrate = window.findChild<QComboBox *>(QStringLiteral("canBitrate"));
    QVERIFY(canBitrate);
    QCOMPARE(canBitrate->currentData().toInt(), 500000);
    QVERIFY(window.findChild<QPushButton *>(QStringLiteral("canConnect")));
    QVERIFY(window.findChild<QPushButton *>(QStringLiteral("canBoardSelfTest")));
    QVERIFY(window.findChild<QTableView *>(QStringLiteral("canFrameTable")));

    auto *toolSections = window.findChild<QTabWidget *>(QStringLiteral("toolSections"));
    QVERIFY(toolSections);
    QCOMPARE(toolSections->count(), 3);
    QCOMPARE(toolSections->tabText(0), QStringLiteral("设备管理"));
    QCOMPARE(toolSections->tabText(1), QStringLiteral("DTU网络"));
    QCOMPARE(toolSections->tabText(2), QStringLiteral("AT指令"));

    auto *buffer = window.findChild<QPlainTextEdit *>(QStringLiteral("serialBuffer"));
    QVERIFY(buffer);
    QCOMPARE(buffer->maximumBlockCount(), 5000);
    QVERIFY(window.findChild<QCheckBox *>(QStringLiteral("serialHexDisplay")));
    QVERIFY(window.findChild<QPushButton *>(QStringLiteral("serialListen")));
    auto *dtuSummary = window.findChild<QLineEdit *>(QStringLiteral("parameter:DTU主卡:配置摘要"));
    QVERIFY(dtuSummary);
    QVERIFY(dtuSummary->isReadOnly());
    QVERIFY(!window.findChild<QWidget *>(QStringLiteral("parameter:DTU主卡:请求方法")));

    auto *samplingRate = window.findChild<QComboBox *>(
        QStringLiteral("parameter:MCU卡:采样频率_FVCF"));
    auto *samplingPoints = window.findChild<QComboBox *>(
        QStringLiteral("parameter:MCU卡:采样点数_FVCF"));
    QVERIFY(samplingRate);
    QVERIFY(samplingPoints);
    QCOMPARE(samplingRate->count(), 11);
    QCOMPARE(samplingPoints->count(), 4);
    QVERIFY(!samplingRate->isEnabled());
    QVERIFY(!samplingPoints->isEnabled());

    auto *saveMeasurementsButton = window.findChild<QPushButton *>(
        QStringLiteral("saveMeasurementsButton"));
    auto *deleteMeasurementsButton = window.findChild<QPushButton *>(
        QStringLiteral("deleteSelectedMeasurementsButton"));
    QVERIFY(saveMeasurementsButton);
    QVERIFY(deleteMeasurementsButton);
    QVERIFY(!saveMeasurementsButton->isEnabled());
    QVERIFY(!deleteMeasurementsButton->isEnabled());

    bool dtuDialogOpened = false;
    QTimer::singleShot(0, &window, [&] {
        auto *dialog = window.findChild<DtuConfigDialog *>();
        dtuDialogOpened = dialog && dialog->findChild<QLineEdit *>(QStringLiteral("dtuMqttClientId"));
        if (dialog)
            dialog->reject();
    });
    QTest::mouseClick(
        window.findChild<QPushButton *>(QStringLiteral("action:DTU主卡:网络配置")),
        Qt::LeftButton);
    QVERIFY(dtuDialogOpened);

    const QString screenshotBase = qEnvironmentVariable("QLIOT_UI_SCREENSHOT_BASE");
    if (!screenshotBase.isEmpty()) {
        auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
        QVERIFY(tabs);
        for (int index = 0; index < tabs->count(); ++index) {
            tabs->setCurrentIndex(index);
            QTest::qWait(80);
            QVERIFY(window.grab().save(QStringLiteral("%1-%2.png").arg(screenshotBase).arg(index)));
        }
    }

    const QString deviceConfigScreenshotBase = qEnvironmentVariable("QLIOT_DEVICE_CONFIG_SCREENSHOT_BASE");
    if (!deviceConfigScreenshotBase.isEmpty()) {
        auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
        QVERIFY(tabs);
        tabs->setCurrentIndex(1);
        for (int index = 0; index < configSections->count(); ++index) {
            configSections->setCurrentIndex(index);
            QTest::qWait(80);
            QVERIFY(window.grab().save(QStringLiteral("%1-%2.png").arg(deviceConfigScreenshotBase).arg(index)));
        }
    }

    const QString sideToolScreenshotBase = qEnvironmentVariable("QLIOT_SIDE_TOOL_SCREENSHOT_BASE");
    if (!sideToolScreenshotBase.isEmpty()) {
        auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
        auto *sideTools = window.findChild<QTabWidget *>(QStringLiteral("sideTools"));
        QVERIFY(tabs);
        QVERIFY(sideTools);
        tabs->setCurrentIndex(0);
        for (int index = 0; index < sideTools->count(); ++index) {
            sideTools->setCurrentIndex(index);
            QTest::qWait(80);
            QVERIFY(window.grab().save(QStringLiteral("%1-%2.png").arg(sideToolScreenshotBase).arg(index)));
        }
    }

    const QString minimumScreenshotBase = qEnvironmentVariable("QLIOT_UI_MIN_SCREENSHOT_BASE");
    if (!minimumScreenshotBase.isEmpty()) {
        auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
        QVERIFY(tabs);
        window.resize(window.minimumSize());
        for (int index = 0; index < tabs->count(); ++index) {
            tabs->setCurrentIndex(index);
            QTest::qWait(80);
            QVERIFY(window.grab().save(QStringLiteral("%1-%2.png").arg(minimumScreenshotBase).arg(index)));
        }
    }
}

void MainWindowUiTest::localFormActionsMutateFields()
{
    MainWindow window;
    auto *curve = window.findChild<QLineEdit *>(QStringLiteral("parameter:标定:曲线名_标定"));
    auto *point = window.findChild<QLineEdit *>(QStringLiteral("parameter:标定:点号_标定"));
    auto *measured = window.findChild<QLineEdit *>(QStringLiteral("parameter:标定:测量值_标定"));
    auto *add = window.findChild<QPushButton *>(QStringLiteral("action:标定:添加"));
    QVERIFY(curve);
    QVERIFY(point);
    QVERIFY(measured);
    QVERIFY(add);
    curve->setText(QStringLiteral("测试曲线"));
    measured->setText(QStringLiteral("12.5"));
    QTest::mouseClick(add, Qt::LeftButton);
    QVERIFY(!point->text().isEmpty());
    QVERIFY(measured->text().isEmpty());
    QCOMPARE(curve->text(), QStringLiteral("测试曲线"));

    auto *command = window.findChild<QLineEdit *>(QStringLiteral("parameter:AT管理器:AT指令"));
    auto *reply = window.findChild<QLineEdit *>(QStringLiteral("parameter:AT管理器:AT回包"));
    auto *clear = window.findChild<QPushButton *>(QStringLiteral("action:AT管理器:清除"));
    QVERIFY(command);
    QVERIFY(reply);
    QVERIFY(clear);
    command->setText(QStringLiteral("AT,get,mcupar"));
    reply->setText(QStringLiteral("response"));
    QTest::mouseClick(clear, Qt::LeftButton);
    QVERIFY(command->text().isEmpty());
    QVERIFY(reply->text().isEmpty());
}

void MainWindowUiTest::frequencyTensionDeviceSwitchesTableAndPlot()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *tree = window.findChild<QTreeWidget *>(QStringLiteral("deviceLibraryTree"));
    auto *table = window.findChild<QTableView *>(QStringLiteral("measurementTable"));
    auto *plot = window.findChild<SpectrumPlotWidget *>(QStringLiteral("spectrumPlot"));
    auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
    auto *title = window.findChild<QLabel *>(QStringLiteral("measurementSectionTitle"));
    auto *controller = window.findChild<gucds::DeviceCommunicationController *>();
    QVERIFY(tree);
    QVERIFY(table);
    QVERIFY(plot);
    QVERIFY(tabs);
    QVERIFY(title);
    QVERIFY(controller);

    QTreeWidgetItem *frequencyDevice = nullptr;
    for (int group = 0; group < tree->topLevelItemCount() && !frequencyDevice; ++group) {
        QTreeWidgetItem *root = tree->topLevelItem(group);
        for (int row = 0; row < root->childCount(); ++row) {
            QTreeWidgetItem *candidate = root->child(row);
            if (candidate->text(0).contains(QStringLiteral("QL-FOFS"), Qt::CaseInsensitive)
                || candidate->text(0).contains(QStringLiteral("QL-FOPS"), Qt::CaseInsensitive)) {
                frequencyDevice = candidate;
                break;
            }
        }
    }
    QVERIFY2(frequencyDevice, "The imported LabVIEW device library must contain a QL-FOFS frequency/tension sensor");
    tree->setCurrentItem(frequencyDevice);
    Q_EMIT tree->itemClicked(frequencyDevice, 0);

    auto *samplingRate = window.findChild<QComboBox *>(
        QStringLiteral("parameter:MCU卡:采样频率_FVCF"));
    auto *samplingPoints = window.findChild<QComboBox *>(
        QStringLiteral("parameter:MCU卡:采样点数_FVCF"));
    QVERIFY(samplingRate);
    QVERIFY(samplingPoints);
    QVERIFY(samplingRate->isEnabled());
    QVERIFY(samplingPoints->isEnabled());
    QCOMPARE(samplingRate->count(), 11);
    QCOMPARE(samplingPoints->count(), 4);
    QVERIFY(samplingRate->findText(QStringLiteral("100")) >= 0);
    QVERIFY(samplingPoints->findText(QStringLiteral("256")) >= 0);

    gucds::CommunicationResult compactMcuReadback;
    compactMcuReadback.success = true;
    compactMcuReadback.context = QStringLiteral("read_frequency_mcu");
    compactMcuReadback.numericValues = {9600, 3, 0, 1, 1, 8, 2};
    compactMcuReadback.message = QStringLiteral("compact readback ok");
    Q_EMIT controller->resultReady(compactMcuReadback);
    QCOMPARE(samplingRate->currentText(), QStringLiteral("800"));
    QCOMPARE(samplingPoints->currentText(), QStringLiteral("1024"));

    gucds::CommunicationResult mcuReadback;
    mcuReadback.success = true;
    mcuReadback.context = QStringLiteral("read_frequency_mcu");
    mcuReadback.numericValues = {9600, 0, 1, 1, 1, 1, 0, 5, 1, 0, 0};
    mcuReadback.message = QStringLiteral("readback ok");
    Q_EMIT controller->resultReady(mcuReadback);
    QCOMPARE(samplingRate->currentText(), QStringLiteral("200"));
    QCOMPARE(samplingPoints->currentText(), QStringLiteral("512"));
    samplingRate->setCurrentText(QStringLiteral("100"));
    samplingPoints->setCurrentText(QStringLiteral("256"));

    QGroupBox *sensorParameterBox = nullptr;
    for (QGroupBox *group : window.findChildren<QGroupBox *>()) {
        if (group->title() == QStringLiteral("传感器参数")) {
            sensorParameterBox = group;
            break;
        }
    }
    QVERIFY(sensorParameterBox);
    const auto sensorParameterLabels = sensorParameterBox->findChildren<QLabel *>();
    for (QLabel *label : sensorParameterLabels) {
        QVERIFY(label->text() != QStringLiteral("采样频率"));
        QVERIFY(label->text() != QStringLiteral("采样点数"));
    }

    const QString frequencyMcuScreenshot = qEnvironmentVariable(
        "QLIOT_UI_SCREENSHOT_FREQUENCY_MCU");
    if (!frequencyMcuScreenshot.isEmpty()) {
        auto *configSections = window.findChild<QTabWidget *>(QStringLiteral("deviceConfigSections"));
        QVERIFY(configSections);
        tabs->setCurrentIndex(1);
        configSections->setCurrentIndex(1);
        QTest::qWait(80);
        QVERIFY(window.grab().save(frequencyMcuScreenshot));
        tabs->setCurrentIndex(0);
    }

    auto *model = qobject_cast<gucds::MeasurementTableModel *>(table->model());
    QVERIFY(model);
    QCOMPARE(model->mode(), gucds::MeasurementTableModel::Mode::FrequencyTension);
    QCOMPARE(model->columnCount(), 7);
    QCOMPARE(model->headerData(2, Qt::Horizontal).toString(), QStringLiteral("索力(kN)"));
    QCOMPARE(model->headerData(3, Qt::Horizontal).toString(), QStringLiteral("fn(Hz)"));
    QCOMPARE(model->headerData(5, Qt::Horizontal).toString(), QStringLiteral("收敛误差(%)"));
    QCOMPARE(title->text(), QStringLiteral("索力测量数据"));
    QCOMPARE(plot->mode(), SpectrumPlotWidget::Mode::FrequencyTension);

    QCOMPARE(plot->yAxisLabel(), QStringLiteral("索力(kN)"));

    gucds::CommunicationResult spectrum;
    spectrum.success = true;
    spectrum.context = QStringLiteral("spectrum");
    QStringList amplitudes = {
        QStringLiteral("0.011473"), QStringLiteral("0.010714"), QStringLiteral("0.005257"),
        QStringLiteral("0.003923"), QStringLiteral("0.005103"), QStringLiteral("0.008380"),
        QStringLiteral("0.008605"), QStringLiteral("0.002981"), QStringLiteral("0.003138"),
        QStringLiteral("0.001874"),
    };
    while (amplitudes.size() < 256) {
        const int index = amplitudes.size();
        double amplitude = (double(index % 9) - 4.0) * 0.0004;
        if (index == 14)
            amplitude = 0.270000;
        else if (index == 27)
            amplitude = 0.035000;
        else if (index == 40)
            amplitude = 0.020000;
        else if (index == 53)
            amplitude = 0.012000;
        else if (index == 67)
            amplitude = 0.010000;
        amplitudes.append(QString::number(amplitude, 'f', 6));
    }
    QStringList frequencies;
    frequencies.reserve(amplitudes.size());
    for (int index = 0; index < amplitudes.size(); ++index)
        frequencies.append(QString::number(double(index) * 0.390625, 'f', 6));
    spectrum.responseText = QStringLiteral("frequency:%1\namplitude:%2")
                                .arg(frequencies.join(QLatin1Char(',')), amplitudes.join(QLatin1Char(',')));
    spectrum.message = QStringLiteral("频谱数据读取成功");
    Q_EMIT controller->resultReady(spectrum);
    QCOMPARE(model->mode(), gucds::MeasurementTableModel::Mode::Spectrum);
    QCOMPARE(model->columnCount(), 3);
    QCOMPARE(model->rowCount(), 256);
    QCOMPARE(model->headerData(0, Qt::Horizontal).toString(), QStringLiteral("点号"));
    QCOMPARE(model->headerData(1, Qt::Horizontal).toString(), QStringLiteral("频率(Hz)"));
    QCOMPARE(model->data(model->index(1, 1)).toString(), QStringLiteral("0.390625"));
    QCOMPARE(model->data(model->index(10, 2)).toString(), QStringLiteral("-0.001200"));
    QCOMPARE(title->text(), QStringLiteral("频谱数据"));
    QCOMPARE(plot->mode(), SpectrumPlotWidget::Mode::Spectrum);
    QCOMPARE(plot->xAxisLabel(), QStringLiteral("频率(Hz)"));
    QCOMPARE(plot->yAxisLabel(), QStringLiteral("幅值"));
    QCOMPARE(plot->seriesPoints().size(), 256);
    QVERIFY(qAbs(plot->seriesPoints().at(14).x() - 5.46875) < 1e-9);
    QVERIFY(qAbs(plot->seriesPoints().at(14).y() - 0.27) < 1e-9);
    QVERIFY(plot->seriesPoints().at(10).y() < 0.0);
    QVERIFY(plot->axisBounds().top() < 0.0);
    QVERIFY(plot->axisBounds().bottom() >= 0.27);

    tabs->setCurrentIndex(0);
    const QString spectrumScreenshot = qEnvironmentVariable("QLIOT_UI_SCREENSHOT_FREQUENCY_SPECTRUM");
    if (!spectrumScreenshot.isEmpty()) {
        QTest::qWait(80);
        QVERIFY(window.grab().save(spectrumScreenshot));
    }

    const QStringList measurements = {
        QStringLiteral("1,Sen05,2208.776,3.906,1.000,0.002"),
        QStringLiteral("2,Sen05,2288.776,3.906,1.000,0.002"),
        QStringLiteral("3,Sen05,5149.791,5.859,1.000,0.001"),
    };
    for (const QString &response : measurements) {
        gucds::CommunicationResult measurement;
        measurement.success = true;
        measurement.context = QStringLiteral("frequency_tension_measurement");
        measurement.responseText = response;
        measurement.message = QStringLiteral("索力测量完成");
        Q_EMIT controller->resultReady(measurement);
    }
    QCOMPARE(model->mode(), gucds::MeasurementTableModel::Mode::FrequencyTension);
    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(model->data(model->index(0, 2)).toString(), QStringLiteral("2208.776"));
    QCOMPARE(model->data(model->index(0, 3)).toString(), QStringLiteral("3.906"));
    QCOMPARE(plot->mode(), SpectrumPlotWidget::Mode::FrequencyTension);

    auto *deleteMeasurements = window.findChild<QAction *>(QStringLiteral("deleteSelectedMeasurementsAction"));
    auto *saveMeasurements = window.findChild<QAction *>(QStringLiteral("saveMeasurementsCsvAction"));
    auto *deleteMeasurementsButton = window.findChild<QPushButton *>(
        QStringLiteral("deleteSelectedMeasurementsButton"));
    auto *saveMeasurementsButton = window.findChild<QPushButton *>(
        QStringLiteral("saveMeasurementsButton"));
    QVERIFY(deleteMeasurements);
    QVERIFY(saveMeasurements);
    QVERIFY(deleteMeasurementsButton);
    QVERIFY(saveMeasurementsButton);
    QCOMPARE(table->viewport()->contextMenuPolicy(), Qt::CustomContextMenu);
    QVERIFY(table->actions().contains(deleteMeasurements));
    QVERIFY(table->actions().contains(saveMeasurements));
    QVERIFY(saveMeasurements->isEnabled());
    QVERIFY(saveMeasurementsButton->isEnabled());
    QVERIFY(saveMeasurementsButton->isVisible());
    QVERIFY(deleteMeasurementsButton->isVisible());

    if (QGuiApplication::platformName() == QStringLiteral("offscreen")) {
        QTemporaryDir csvDirectory;
        QVERIFY(csvDirectory.isValid());
        const QString csvPath = csvDirectory.filePath(QStringLiteral("frequency-measurements.csv"));
        QTimer::singleShot(0, &window, [&csvPath] {
            auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            if (!dialog)
                return;
            dialog->selectFile(csvPath);
            QVERIFY(QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection));
        });
        QTest::mouseClick(saveMeasurementsButton, Qt::LeftButton);
        QFile csv(csvPath);
        QVERIFY(csv.open(QIODevice::ReadOnly));
        const QByteArray csvData = csv.readAll();
        QVERIFY(csvData.startsWith(QByteArray::fromHex("efbbbf")));
        QVERIFY(csvData.contains(QStringLiteral("索力(kN)").toUtf8()));
        QVERIFY(csvData.contains(QByteArrayLiteral("5149.791")));
    }

    table->selectRow(0);
    QCoreApplication::processEvents();
    QVERIFY(deleteMeasurements->isEnabled());
    QVERIFY(deleteMeasurementsButton->isEnabled());
    const QString contextMenuScreenshot = qEnvironmentVariable(
        "QLIOT_UI_SCREENSHOT_MEASUREMENT_CONTEXT_MENU");
    if (!contextMenuScreenshot.isEmpty()) {
        const QRect firstCell = table->visualRect(model->index(0, 0));
        QVERIFY(firstCell.isValid());
        Q_EMIT table->viewport()->customContextMenuRequested(firstCell.center());
        QTRY_VERIFY_WITH_TIMEOUT(QApplication::activePopupWidget(), 1000);
        QWidget *popup = QApplication::activePopupWidget();
        QVERIFY(popup->grab().save(contextMenuScreenshot));
        popup->close();
        QCoreApplication::processEvents();
    }
    QTimer::singleShot(0, &window, [] {
        if (auto *messageBox = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            if (QAbstractButton *yesButton = messageBox->button(QMessageBox::Yes))
                yesButton->click();
        }
    });
    QTest::mouseClick(deleteMeasurementsButton, Qt::LeftButton);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(model->data(model->index(0, 0)).toInt(), 1);
    QCOMPARE(plot->seriesPoints().size(), 2);
    auto *latestCount = window.findChild<QLineEdit *>(QStringLiteral("latestCount"));
    QVERIFY(latestCount);
    QCOMPARE(latestCount->text(), QStringLiteral("2"));

    const QString measurementScreenshot = qEnvironmentVariable("QLIOT_UI_SCREENSHOT_FREQUENCY_MEASUREMENT");
    if (!measurementScreenshot.isEmpty()) {
        QTest::qWait(80);
        QVERIFY(window.grab().save(measurementScreenshot));
    }
}

void MainWindowUiTest::frequencyStartButtonTriggersCompactCommandMeasurement()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the Start-button test.");

    const QByteArray getMcuRequest = QByteArrayLiteral("AT,get,mcupar\r\n");
    const QByteArray getSensorRequest = QByteArrayLiteral("AT,get,senpar\r\n");
    const QByteArray startRequest = gucds::VirtualModbusClient::buildWriteSingleHoldingRegister(
        1,
        0x0000,
        static_cast<quint16>(gucds::SensorModbusCommand::MeasureStart));
    const QByteArray statusRequest =
        gucds::VirtualModbusClient::buildReadHoldingRegisters(1, 0x0002, 1);
    const QByteArray dataRequest =
        gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 8);

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
    QString serialError;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &serialError), qPrintable(serialError));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        const auto receiveAndReply = [&](const QByteArray &expected, const QByteArray &reply) {
            QByteArray request;
            if (!sensorSession.readFrame(expected.size(), &request, &sensorError, 3000))
                return false;
            if (request != expected) {
                sensorError = QStringLiteral("Unexpected Start-button frame: %1")
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
        if (!receiveAndReply(startRequest, startResponse)
            || !receiveAndReply(statusRequest, pendingStatus)
            || !receiveAndReply(statusRequest, readyStatus)) {
            return;
        }
        receiveAndReply(dataRequest, dataResponse);
    });
    sensorThread->start();

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *tree = window.findChild<QTreeWidget *>(QStringLiteral("deviceLibraryTree"));
    auto *sideTools = window.findChild<QTabWidget *>(QStringLiteral("sideTools"));
    auto *port = window.findChild<QComboBox *>(QStringLiteral("serialPort"));
    auto *baud = window.findChild<QComboBox *>(QStringLiteral("serialBaud"));
    auto *protocol = window.findChild<QComboBox *>(QStringLiteral("serialProtocol"));
    auto *slaveId = window.findChild<QSpinBox *>(QStringLiteral("serialSlaveId"));
    auto *start = window.findChild<QPushButton *>(QStringLiteral("serialStart"));
    auto *controller = window.findChild<gucds::DeviceCommunicationController *>();
    auto *table = window.findChild<QTableView *>(QStringLiteral("measurementTable"));
    QVERIFY(tree);
    QVERIFY(sideTools);
    QVERIFY(port);
    QVERIFY(baud);
    QVERIFY(protocol);
    QVERIFY(slaveId);
    QVERIFY(start);
    QVERIFY(controller);
    QVERIFY(table);

    QTreeWidgetItem *frequencyDevice = nullptr;
    for (int group = 0; group < tree->topLevelItemCount() && !frequencyDevice; ++group) {
        QTreeWidgetItem *root = tree->topLevelItem(group);
        for (int row = 0; row < root->childCount(); ++row) {
            QTreeWidgetItem *candidate = root->child(row);
            if (candidate->text(0).contains(QStringLiteral("QL-FOFS"), Qt::CaseInsensitive)
                || candidate->text(0).contains(QStringLiteral("QL-FOPS"), Qt::CaseInsensitive)) {
                frequencyDevice = candidate;
                break;
            }
        }
    }
    QVERIFY(frequencyDevice);
    tree->setCurrentItem(frequencyDevice);
    Q_EMIT tree->itemClicked(frequencyDevice, 0);
    sideTools->setCurrentIndex(1);
    port->setCurrentText(pcPort);
    baud->setCurrentText(QStringLiteral("9600"));
    protocol->setCurrentText(QStringLiteral("Modbus"));
    slaveId->setValue(1);

    QSignalSpy resultSpy(controller, &gucds::DeviceCommunicationController::resultReady);
    QTest::mouseClick(start, Qt::LeftButton);
    QTRY_COMPARE_WITH_TIMEOUT(resultSpy.size(), 1, 8000);
    const gucds::CommunicationResult result =
        qvariant_cast<gucds::CommunicationResult>(resultSpy.takeFirst().at(0));
    QVERIFY2(result.success, qPrintable(result.message));
    QCOMPARE(result.context, QStringLiteral("frequency_tension_measurement"));
    QVERIFY(result.responseText.contains(QStringLiteral("B1 RX:")));
    QVERIFY(result.responseText.contains(QStringLiteral("SAMPLING: 800 Hz, 1024 points")));
    QCOMPARE(table->model()->rowCount(), 1);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void MainWindowUiTest::frequencyStartButtonStreamsAutomaticMeasurementsUntilStopped()
{
    const QString pcPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_PC");
    const QString sensorPort = qEnvironmentVariable("QLIOT_TEST_SERIAL_SENSOR");
    if (pcPort.isEmpty() || sensorPort.isEmpty())
        QSKIP("Set QLIOT_TEST_SERIAL_PC and QLIOT_TEST_SERIAL_SENSOR to run the automatic frequency UI test.");

    const QByteArray getMcuRequest = QByteArrayLiteral("AT,get,mcupar\r\n");
    const QByteArray getSensorRequest = QByteArrayLiteral("AT,get,senpar\r\n");
    const QByteArray dataRequest =
        gucds::VirtualModbusClient::buildReadInputRegisters(1, 0x0000, 8);
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
    QString openError;
    QVERIFY2(sensorSession.open(sensorPort, 9600, &openError), qPrintable(openError));
    QString sensorError;
    QThread *sensorThread = QThread::create([&] {
        const auto receive = [&](const QByteArray &expected, int timeoutMs = 3000) {
            QByteArray actual;
            if (!sensorSession.readFrame(expected.size(), &actual, &sensorError, timeoutMs))
                return false;
            if (actual != expected) {
                sensorError = QStringLiteral("Unexpected automatic UI frame: %1")
                                  .arg(gucds::VirtualModbusClient::formatHex(actual));
                return false;
            }
            return true;
        };
        const auto replyData = [&] {
            return receive(dataRequest)
                && sensorSession.writeFrame(dataResponse, &sensorError);
        };

        if (!receive(getMcuRequest)
            || !sensorSession.writeFrame(
                QByteArrayLiteral("get,mcupar,9600,0,1,1,1,over\r\n"), &sensorError)) {
            return;
        }
        if (!receive(getSensorRequest)
            || !sensorSession.writeFrame(
                QByteArrayLiteral("get,senpar,4,0,over\r\n"), &sensorError)) {
            return;
        }
        if (!replyData() || !replyData()) {
            return;
        }

        QByteArray finalRequest;
        QString finalReadError;
        if (sensorSession.readFrame(dataRequest.size(), &finalRequest, &finalReadError, 3000)
            && finalRequest == dataRequest) {
            QString ignoredWriteError;
            sensorSession.writeFrame(dataResponse, &ignoredWriteError);
        }
    });
    sensorThread->start();

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.showNormal();
    window.resize(1296, 769);
    QTRY_COMPARE_WITH_TIMEOUT(window.size(), QSize(1296, 769), 1000);
    auto *measurementModel = window.findChild<gucds::MeasurementTableModel *>();
    auto *tree = window.findChild<QTreeWidget *>(QStringLiteral("deviceLibraryTree"));
    auto *mainTabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
    auto *sideTools = window.findChild<QTabWidget *>(QStringLiteral("sideTools"));
    auto *port = window.findChild<QComboBox *>(QStringLiteral("serialPort"));
    auto *start = window.findChild<QPushButton *>(QStringLiteral("serialStart"));
    auto *latestCount = window.findChild<QLineEdit *>(QStringLiteral("latestCount"));
    auto *controller = window.findChild<gucds::DeviceCommunicationController *>();
    QVERIFY(measurementModel);
    QVERIFY(tree);
    QVERIFY(mainTabs);
    QVERIFY(sideTools);
    QVERIFY(port);
    QVERIFY(start);
    QVERIFY(latestCount);
    QVERIFY(controller);

    QTreeWidgetItem *frequencyDevice = nullptr;
    for (int group = 0; group < tree->topLevelItemCount() && !frequencyDevice; ++group) {
        QTreeWidgetItem *root = tree->topLevelItem(group);
        for (int row = 0; row < root->childCount(); ++row) {
            QTreeWidgetItem *candidate = root->child(row);
            if (candidate->text(0).contains(QStringLiteral("QL-FOFS"), Qt::CaseInsensitive)
                || candidate->text(0).contains(QStringLiteral("QL-FOPS"), Qt::CaseInsensitive)) {
                frequencyDevice = candidate;
                break;
            }
        }
    }
    QVERIFY(frequencyDevice);
    tree->setCurrentItem(frequencyDevice);
    Q_EMIT tree->itemClicked(frequencyDevice, 0);
    sideTools->setCurrentIndex(1);
    mainTabs->setCurrentIndex(0);
    port->setCurrentText(pcPort);

    const QString initialText = start->text();
    QTest::mouseClick(start, Qt::LeftButton);
    QTRY_COMPARE_WITH_TIMEOUT(measurementModel->rowCount(), 2, 12000);
    QCOMPARE(latestCount->text(), QStringLiteral("2"));
    QVERIFY(controller->isRunning());
    QCOMPARE(start->text(), window.tr("停止测量"));
    QVERIFY(QApplication::activeModalWidget() == nullptr);

    const QString streamScreenshot =
        qEnvironmentVariable("QLIOT_UI_SCREENSHOT_FREQUENCY_AUTO_STREAM");
    if (!streamScreenshot.isEmpty()) {
        QTest::qWait(80);
        QVERIFY(window.grab().save(streamScreenshot));
    }

    QTest::mouseClick(start, Qt::LeftButton);
    QTRY_VERIFY_WITH_TIMEOUT(!controller->isRunning(), 3000);
    QCOMPARE(start->text(), initialText);
    QTest::qWait(300);
    QCOMPARE(measurementModel->rowCount(), 2);
    QVERIFY(QApplication::activeModalWidget() == nullptr);

    const bool sensorFinished = sensorThread->wait(3000);
    if (!sensorFinished) {
        sensorThread->terminate();
        sensorThread->wait();
    }
    delete sensorThread;
    QVERIFY(sensorFinished);
    QVERIFY2(sensorError.isEmpty(), qPrintable(sensorError));
}

void MainWindowUiTest::frequencyParameterManagerOpensOffline()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QAction *action = window.findChild<QAction *>(QStringLiteral("frequencyTensionParametersAction"));
    QVERIFY(action);

    bool dialogOpened = false;
    QTimer::singleShot(0, &window, [&dialogOpened] {
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            if (widget->objectName() == QStringLiteral("frequencyTensionParameterDialog")) {
                dialogOpened = true;
                widget->close();
                return;
            }
        }
    });
    action->trigger();
    QVERIFY(dialogOpened);
}

void MainWindowUiTest::languageCanSwitchBothDirections()
{
    QSettings settings;
    const QString settingKey = QStringLiteral("ui/language");
    const bool hadPreviousSetting = settings.contains(settingKey);
    const QVariant previousSetting = settings.value(settingKey);

    {
        ApplicationTranslator translator;
        QVERIFY(translator.setLanguage(ApplicationTranslator::englishLanguage()));
        QCOMPARE(QCoreApplication::translate("MainWindow", "文件"), QStringLiteral("File"));
        QVERIFY(translator.setLanguage(ApplicationTranslator::chineseLanguage()));
        QCOMPARE(QCoreApplication::translate("MainWindow", "文件"), QStringLiteral("文件"));
        QCOMPARE(QCoreApplication::translate("QPlatformTheme", "Cancel"), QStringLiteral("取消"));
        QVERIFY(translator.setLanguage(ApplicationTranslator::englishLanguage()));
        QCOMPARE(QCoreApplication::translate("MainWindow", "文件"), QStringLiteral("File"));
    }

    if (hadPreviousSetting)
        settings.setValue(settingKey, previousSetting);
    else
        settings.remove(settingKey);
    qApp->setProperty("uiLanguage", QStringLiteral("zh_CN"));
}

void MainWindowUiTest::languageMenuIsBilingualAndRequiresRestart()
{
    qApp->setProperty("uiLanguage", QStringLiteral("zh_CN"));
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *settingsMenu = window.findChild<QMenu *>(QStringLiteral("settingsMenu"));
    auto *languageMenu = window.findChild<QMenu *>(QStringLiteral("languageMenu"));
    auto *englishAction = window.findChild<QAction *>(QStringLiteral("language:en_US"));
    auto *chineseAction = window.findChild<QAction *>(QStringLiteral("language:zh_CN"));
    QVERIFY(settingsMenu);
    QVERIFY(languageMenu);
    QVERIFY(englishAction);
    QVERIFY(chineseAction);
    QCOMPARE(settingsMenu->title(), QStringLiteral("设置 (Settings)"));
    QCOMPARE(languageMenu->title(), QStringLiteral("语言 (Language)"));
    QCOMPARE(chineseAction->text(), QStringLiteral("简体中文 (Chinese)"));
    QCOMPARE(englishAction->text(), QStringLiteral("English (英语)"));

    QSignalSpy languageSpy(&window, &MainWindow::languageChangeRequested);
    bool cancelDialogWasBilingual = false;
    QTimer::singleShot(0, &window, [&] {
        auto *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        cancelDialogWasBilingual = dialog
            && dialog->windowTitle() == QStringLiteral("切换语言 (Switch Language)")
            && dialog->text().contains(QStringLiteral("程序将关闭"))
            && dialog->text().contains(QStringLiteral("application will close"), Qt::CaseInsensitive);
        if (dialog) {
            const QString screenshot = qEnvironmentVariable("QLIOT_UI_SCREENSHOT_LANGUAGE_DIALOG");
            if (!screenshot.isEmpty())
                QVERIFY(dialog->grab().save(screenshot));
            auto *cancel = dialog->findChild<QPushButton *>(QStringLiteral("languageRestartCancel"));
            if (cancel)
                cancel->click();
        }
    });
    englishAction->trigger();
    QVERIFY(cancelDialogWasBilingual);
    QCOMPARE(languageSpy.count(), 0);
    QVERIFY(chineseAction->isChecked());

    bool confirmationAccepted = false;
    QTimer::singleShot(0, &window, [&] {
        auto *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (dialog) {
            auto *confirm = dialog->findChild<QPushButton *>(QStringLiteral("languageRestartConfirm"));
            confirmationAccepted = confirm;
            if (confirm)
                confirm->click();
        }
    });
    englishAction->trigger();
    QVERIFY(confirmationAccepted);
    QCOMPARE(languageSpy.count(), 1);
    QCOMPARE(languageSpy.takeFirst().at(0).toString(), QStringLiteral("en_US"));
}

void MainWindowUiTest::englishUiKeepsProtocolKeysStable()
{
    TsTranslator translator;
    QVERIFY(translator.loadTs(QStringLiteral(":/i18n/translations/gucds_en_US.ts")));
    QVERIFY(qApp->installTranslator(&translator));
    qApp->setProperty("uiLanguage", QStringLiteral("en_US"));

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *tabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 5);
    QCOMPARE(tabs->tabText(0), QStringLiteral("Test Data"));
    QCOMPARE(tabs->tabText(1), QStringLiteral("Device Configuration"));
    QCOMPARE(tabs->tabText(4), QStringLiteral("CAN Bus"));
    auto *canConnect = window.findChild<QPushButton *>(QStringLiteral("canConnect"));
    auto *canSelfTest = window.findChild<QPushButton *>(QStringLiteral("canBoardSelfTest"));
    QVERIFY(canConnect);
    QVERIFY(canSelfTest);
    QCOMPARE(canConnect->text(), QStringLiteral("Connect"));
    QCOMPARE(canSelfTest->text(), QStringLiteral("Board Self-Test"));

    auto *settingsMenu = window.findChild<QMenu *>(QStringLiteral("settingsMenu"));
    auto *languageMenu = window.findChild<QMenu *>(QStringLiteral("languageMenu"));
    QVERIFY(settingsMenu);
    QVERIFY(languageMenu);
    QCOMPARE(settingsMenu->title(), QStringLiteral("设置 (Settings)"));
    QCOMPARE(languageMenu->title(), QStringLiteral("语言 (Language)"));

    auto *configSections = window.findChild<QTabWidget *>(QStringLiteral("deviceConfigSections"));
    QVERIFY(configSections);
    QCOMPARE(configSections->count(), 4);
    QCOMPARE(configSections->tabText(0), QStringLiteral("Device Parameters"));
    QCOMPARE(configSections->tabText(1), QStringLiteral("MCU Module"));
    QCOMPARE(configSections->tabText(2), QStringLiteral("LoRa Settings"));
    QCOMPARE(configSections->tabText(3), QStringLiteral("DTU Module"));

    auto *sideTools = window.findChild<QTabWidget *>(QStringLiteral("sideTools"));
    QVERIFY(sideTools);
    QCOMPARE(sideTools->tabText(0), QStringLiteral("Device Library"));
    QCOMPARE(sideTools->tabText(1), QStringLiteral("Communication"));

    auto *toolSections = window.findChild<QTabWidget *>(QStringLiteral("toolSections"));
    QVERIFY(toolSections);
    QCOMPARE(toolSections->tabText(0), QStringLiteral("Devices"));
    QCOMPARE(toolSections->tabText(1), QStringLiteral("DTU Network"));
    QCOMPARE(toolSections->tabText(2), QStringLiteral("AT Command"));

    auto *deleteMeasurements = window.findChild<QAction *>(QStringLiteral("deleteSelectedMeasurementsAction"));
    auto *saveMeasurements = window.findChild<QAction *>(QStringLiteral("saveMeasurementsCsvAction"));
    QVERIFY(deleteMeasurements);
    QVERIFY(saveMeasurements);
    QCOMPARE(deleteMeasurements->text(), QStringLiteral("Delete Selected Records"));
    QCOMPARE(saveMeasurements->text(), QStringLiteral("Save Measurement Data as CSV…"));
    auto *deleteMeasurementsButton = window.findChild<QPushButton *>(
        QStringLiteral("deleteSelectedMeasurementsButton"));
    auto *saveMeasurementsButton = window.findChild<QPushButton *>(
        QStringLiteral("saveMeasurementsButton"));
    QVERIFY(deleteMeasurementsButton);
    QVERIFY(saveMeasurementsButton);
    QCOMPARE(deleteMeasurementsButton->text(), QStringLiteral("Delete Selected"));
    QCOMPARE(saveMeasurementsButton->text(), QStringLiteral("Save Data…"));

    auto *samplingRate = window.findChild<QComboBox *>(
        QStringLiteral("parameter:MCU卡:采样频率_FVCF"));
    auto *samplingPoints = window.findChild<QComboBox *>(
        QStringLiteral("parameter:MCU卡:采样点数_FVCF"));
    QVERIFY(samplingRate);
    QVERIFY(samplingPoints);
    QCOMPARE(translator.translate("MainWindow", "采样频率"), QStringLiteral("Sampling Frequency"));
    QCOMPARE(translator.translate("MainWindow", "采样点数"), QStringLiteral("Sample Count"));

    auto *readMcu = window.findChild<QPushButton *>(QStringLiteral("action:MCU卡:读MCU"));
    QVERIFY(readMcu);
    QCOMPARE(readMcu->text(), QStringLiteral("Read MCU"));

    auto *loraMode = window.findChild<QComboBox *>(QStringLiteral("parameter:LoRa设置:工作模式_LR"));
    QVERIFY(loraMode);
    QCOMPARE(loraMode->count(), 3);
    QCOMPARE(loraMode->itemText(2), QStringLiteral("Transparent Mode"));

    auto *loraBaud = window.findChild<QComboBox *>(QStringLiteral("parameter:LoRa设置:波特率_LR"));
    QVERIFY(loraBaud);
    QCOMPARE(loraBaud->count(), 7);
    QCOMPARE(loraBaud->itemText(0), QStringLiteral("9600"));
    QCOMPARE(loraBaud->itemText(6), QStringLiteral("115200"));

    auto *englishAction = window.findChild<QAction *>(QStringLiteral("language:en_US"));
    auto *chineseAction = window.findChild<QAction *>(QStringLiteral("language:zh_CN"));
    QVERIFY(englishAction);
    QVERIFY(chineseAction);
    QVERIFY(englishAction->isChecked());
    QCOMPARE(chineseAction->text(), QStringLiteral("简体中文 (Chinese)"));
    QCOMPARE(translator.translate("MainWindow", "菜单"), QStringLiteral("Menu"));
    QCOMPARE(translator.translate("MainWindow", "配置摘要"), QStringLiteral("Configuration Summary"));
    QCOMPARE(translator.translate("DtuConfigDialog", "网络通信协议"), QStringLiteral("Network Protocol"));

    gucds::MeasurementTableModel frequencyModel;
    frequencyModel.setMode(gucds::MeasurementTableModel::Mode::FrequencyTension);
    QCOMPARE(frequencyModel.headerData(2, Qt::Horizontal).toString(), QStringLiteral("Force (kN)"));
    QCOMPARE(frequencyModel.headerData(5, Qt::Horizontal).toString(), QStringLiteral("Convergence Error (%)"));
    frequencyModel.setSpectrumPoints({{1, 0.0, 0.1}});
    QCOMPARE(frequencyModel.headerData(1, Qt::Horizontal).toString(), QStringLiteral("Frequency (Hz)"));

    SpectrumPlotWidget localizedPlot;
    localizedPlot.setSpectrumPoints({QPointF(0.0, 0.1)});
    QCOMPARE(localizedPlot.xAxisLabel(), QStringLiteral("Frequency (Hz)"));
    QCOMPARE(localizedPlot.yAxisLabel(), QStringLiteral("Amplitude"));

    const bool hasNativeFontMetrics = QGuiApplication::platformName() != QStringLiteral("offscreen");
    if (hasNativeFontMetrics) {
        for (const int tabIndex : {1, 3}) {
            tabs->setCurrentIndex(tabIndex);
            QCoreApplication::processEvents();
            const auto buttons = window.findChildren<QPushButton *>();
            for (QPushButton *button : buttons) {
                if (!button->isVisible() || !button->objectName().startsWith(QStringLiteral("action:")))
                    continue;
                QVERIFY2(button->width() >= button->sizeHint().width(),
                         qPrintable(QStringLiteral("Button text is clipped: %1").arg(button->objectName())));
            }
        }

        tabs->setCurrentIndex(2);
        QCoreApplication::processEvents();
        auto *calibrationTable =
            window.findChild<QTableView *>(QStringLiteral("calibrationTable"));
        QVERIFY(calibrationTable);
        for (int column = 0; column < 5; ++column) {
            const QString text = calibrationTable->model()->headerData(
                column, Qt::Horizontal, Qt::DisplayRole).toString();
            QVERIFY2(calibrationTable->horizontalHeader()->sectionSize(column)
                         >= calibrationTable->horizontalHeader()->sectionSizeHint(column),
                     qPrintable(QStringLiteral("Calibration header is clipped: %1").arg(text)));
        }
    }

    gucds::SerialSession serialSession;
    QCOMPARE(serialSession.statusText(), QStringLiteral("Not Connected"));

    QString databaseError;
    QVERIFY(!gucds::LabviewDatabase::saveDeviceRecord({}, nullptr, &databaseError));
    QCOMPARE(databaseError, QStringLiteral("Product record cannot be empty"));

    gucds::VirtualDevice virtualDevice;
    QCOMPARE(virtualDevice.transact(QStringLiteral("unsupported")).message,
             QStringLiteral("The virtual device does not recognize this command"));

    const QString screenshotBase = qEnvironmentVariable("QLIOT_UI_SCREENSHOT_EN_BASE");
    if (!screenshotBase.isEmpty()) {
        for (int index = 0; index < tabs->count(); ++index) {
            tabs->setCurrentIndex(index);
            QTest::qWait(80);
            QVERIFY(window.grab().save(QStringLiteral("%1-%2.png").arg(screenshotBase).arg(index)));
        }
    }

    ProductManagementDialog productDialog(gucds::LabviewDatabase::defaultDatabasePath());
    productDialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&productDialog));
    QCOMPARE(productDialog.windowTitle(), QStringLiteral("Product Management"));
    auto *categoryFilter = productDialog.findChild<QComboBox *>(QStringLiteral("productCategoryFilter"));
    auto *parameterTable = productDialog.findChild<QTableWidget *>(QStringLiteral("productParameterTable"));
    QVERIFY(categoryFilter);
    QVERIFY(parameterTable);
    QCOMPARE(categoryFilter->itemText(0), QStringLiteral("All Categories"));
    if (hasNativeFontMetrics) {
        for (int column = 0; column < parameterTable->columnCount(); ++column) {
            const QString text = parameterTable->horizontalHeaderItem(column)->text();
            const int requiredWidth = parameterTable->horizontalHeader()->fontMetrics().horizontalAdvance(text) + 16;
            QVERIFY2(parameterTable->columnWidth(column) >= requiredWidth,
                     qPrintable(QStringLiteral("Product parameter header is clipped: %1").arg(text)));
        }
    }
    const QString productScreenshot = qEnvironmentVariable("QLIOT_UI_SCREENSHOT_EN_PRODUCT");
    if (!productScreenshot.isEmpty()) {
        QVERIFY(productDialog.grab().save(productScreenshot));
    }

    FrequencyTensionParameterDialog frequencyDialog(gucds::LabviewDatabase::defaultDatabasePath());
    frequencyDialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&frequencyDialog));
    QCOMPARE(frequencyDialog.windowTitle(),
             QStringLiteral("Frequency/Tension Sensor Extended Parameter Manager"));
    auto *frequencyTable = frequencyDialog.findChild<QTableWidget *>(
        QStringLiteral("frequencyParameterTable"));
    auto *frequencyAddButton = frequencyDialog.findChild<QPushButton *>(
        QStringLiteral("addFrequencyParameterButton"));
    auto *frequencyDescription = frequencyDialog.findChild<QLabel *>(
        QStringLiteral("frequencyParameterDescription"));
    auto *frequencySaveButton = frequencyDialog.findChild<QPushButton *>(
        QStringLiteral("saveFrequencyParameterButton"));
    auto *frequencyWriteButton = frequencyDialog.findChild<QPushButton *>(
        QStringLiteral("writeFrequencyParameterSensorButton"));
    QVERIFY(frequencyTable);
    QVERIFY(frequencyAddButton);
    QVERIFY(frequencyDescription);
    QVERIFY(frequencySaveButton);
    QVERIFY(frequencyWriteButton);
    QCOMPARE(frequencyTable->horizontalHeaderItem(0)->text(), QStringLiteral("Sensor Name"));
    QCOMPARE(frequencyAddButton->text(), QStringLiteral("Add"));
    QCOMPARE(frequencySaveButton->text(), QStringLiteral("Save Locally"));
    QCOMPARE(frequencyWriteButton->text(), QStringLiteral("Write to Sensor and Verify"));
    QCOMPARE(frequencyDescription->text(),
             QStringLiteral("“Save Locally” updates only the local database. “Write to Sensor and Verify” sends AT,set,exppar,... and then confirms the values by reading them back with AT,get,FVCFexppar."));
    const QString frequencyScreenshot = qEnvironmentVariable("QLIOT_UI_SCREENSHOT_EN_FREQUENCY");
    if (!frequencyScreenshot.isEmpty())
        QVERIFY(frequencyDialog.grab().save(frequencyScreenshot));

    qApp->removeTranslator(&translator);
    qApp->setProperty("uiLanguage", QStringLiteral("zh_CN"));
}

void MainWindowUiTest::lightThemeOverridesDarkSystemPalette()
{
    const QPalette originalPalette = qApp->palette();
    const auto restorePalette = qScopeGuard([originalPalette] {
        qApp->setPalette(originalPalette);
    });
    QPalette darkPalette = originalPalette;
    darkPalette.setColor(QPalette::Window, QColor(QStringLiteral("#202020")));
    darkPalette.setColor(QPalette::WindowText, QColor(QStringLiteral("#f0f0f0")));
    darkPalette.setColor(QPalette::Base, QColor(QStringLiteral("#101010")));
    darkPalette.setColor(QPalette::Text, QColor(QStringLiteral("#f0f0f0")));
    darkPalette.setColor(QPalette::Button, QColor(QStringLiteral("#181818")));
    darkPalette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#f0f0f0")));
    qApp->setPalette(darkPalette);

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    if (QGuiApplication::platformName() == QStringLiteral("windows"))
        QCOMPARE(QGuiApplication::styleHints()->colorScheme(), Qt::ColorScheme::Light);
    auto *mainTabs = window.findChild<QTabWidget *>(QStringLiteral("mainTabs"));
    auto *comboBox = window.findChild<QComboBox *>(QStringLiteral("canBitrate"));
    auto *canExtended = window.findChild<QCheckBox *>(QStringLiteral("canExtended"));
    QVERIFY(mainTabs);
    QVERIFY(comboBox);
    QVERIFY(canExtended);
    mainTabs->setCurrentIndex(4);
    QCoreApplication::processEvents();
    QVERIFY(comboBox->isVisible());
    QVERIFY(canExtended->isVisible());

    const QPalette closedPalette = comboBox->palette();
    QCOMPARE(closedPalette.color(QPalette::Base), QColor(QStringLiteral("#ffffff")));
    QCOMPARE(closedPalette.color(QPalette::Text), QColor(QStringLiteral("#172033")));
    QCOMPARE(closedPalette.color(QPalette::Button), QColor(QStringLiteral("#ffffff")));
    QCOMPARE(closedPalette.color(QPalette::ButtonText), QColor(QStringLiteral("#172033")));

    const QString closedScreenshot = qEnvironmentVariable("QLIOT_DARK_COMBO_CLOSED_SCREENSHOT");
    if (!closedScreenshot.isEmpty())
        QVERIFY(comboBox->grab().save(closedScreenshot));

    comboBox->showPopup();
    QTRY_VERIFY_WITH_TIMEOUT(comboBox->view()->isVisible(), 1000);
    const QPalette popupPalette = comboBox->view()->palette();
    QCOMPARE(popupPalette.color(QPalette::Base), QColor(QStringLiteral("#ffffff")));
    QCOMPARE(popupPalette.color(QPalette::Text), QColor(QStringLiteral("#172033")));
    QCOMPARE(popupPalette.color(QPalette::Highlight), QColor(QStringLiteral("#dbeaf7")));
    QCOMPARE(popupPalette.color(QPalette::HighlightedText), QColor(QStringLiteral("#12324f")));

    const QString screenshot = qEnvironmentVariable("QLIOT_DARK_COMBO_SCREENSHOT");
    if (!screenshot.isEmpty())
        QVERIFY(comboBox->view()->window()->grab().save(screenshot));
    comboBox->hidePopup();

    mainTabs->setCurrentIndex(0);
    window.resize(2200, 1000);
    QCoreApplication::processEvents();
    auto *measurementTable = window.findChild<QTableView *>(QStringLiteral("measurementTable"));
    QVERIFY(measurementTable);
    QHeaderView *header = measurementTable->horizontalHeader();
    QVERIFY(header->viewport()->width() > header->length() + 20);
    const QImage headerImage = header->grab().toImage();
    const QColor emptyHeaderColor = headerImage.pixelColor(header->length() + 10,
                                                           header->height() / 2);
    QCOMPARE(emptyHeaderColor, QColor(QStringLiteral("#eef3f7")));
}

QTEST_MAIN(MainWindowUiTest)

#include "test_mainwindow_ui.moc"
