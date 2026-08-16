#include "mainwindow.h"

#include "applicationtranslator.h"
#include "gucds/core/appconfig.h"
#include "gucds/core/atprotocol.h"
#include "gucds/core/busdevicetablemodel.h"
#include "gucds/core/calibrationtablemodel.h"
#include "gucds/core/devicecommunicationcontroller.h"
#include "gucds/core/deviceparameter.h"
#include "gucds/core/devicetablemodel.h"
#include "gucds/core/labviewdatabase.h"
#include "gucds/core/measurementtablemodel.h"
#include "gucds/core/serialsession.h"
#include "gucds/core/virtualmodbusclient.h"
#include "gucds/widgets/dtuconfigdialog.h"
#include "gucds/widgets/canmonitorwidget.h"
#include "gucds/widgets/frequencytensionparameterdialog.h"
#include "gucds/widgets/productmanagementdialog.h"
#include "spectrumplotwidget.h"

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QFrame>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHash>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QSizePolicy>
#include <QSpinBox>
#include <QSplitter>
#include <QTabBar>
#include <QStatusBar>
#include <QStyleHints>
#include <QTabWidget>
#include <QTableView>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace {

constexpr int kLabViewWidth = 1296;
constexpr int kLabViewHeight = 769;
constexpr int kDeviceLibraryRecordRole = Qt::UserRole + 1;

QString translatedMainWindowText(const QString &source)
{
    static const QHash<QString, QString> displayAliases = {
        {QStringLiteral("MCU卡"), QStringLiteral("MCU模块")},
        {QStringLiteral("波特率_MCU"), QStringLiteral("波特率")},
        {QStringLiteral("Gap_MCU"), QStringLiteral("采样间隔")},
        {QStringLiteral("采样频率_FVCF"), QStringLiteral("采样频率")},
        {QStringLiteral("采样点数_FVCF"), QStringLiteral("采样点数")},
        {QStringLiteral("Mode_MCU"), QStringLiteral("工作模式")},
        {QStringLiteral("ModID_MCU"), QStringLiteral("Modbus ID")},
        {QStringLiteral("RS485_MCU"), QStringLiteral("RS485")},
        {QStringLiteral("波特率_LR"), QStringLiteral("波特率")},
        {QStringLiteral("信道_LR"), QStringLiteral("信道")},
        {QStringLiteral("功率_LR"), QStringLiteral("发射功率")},
        {QStringLiteral("空速_LR"), QStringLiteral("空中速率")},
        {QStringLiteral("工作模式_LR"), QStringLiteral("工作模式")},
        {QStringLiteral("主/从_LR"), QStringLiteral("主从模式")},
        {QStringLiteral("本地组号_LR"), QStringLiteral("本地组号")},
        {QStringLiteral("本地地址_LR"), QStringLiteral("本地地址")},
        {QStringLiteral("目标组号_LR"), QStringLiteral("目标组号")},
        {QStringLiteral("目标地址_LR"), QStringLiteral("目标地址")},
        {QStringLiteral("DTU主卡"), QStringLiteral("DTU模块")},
        {QStringLiteral("曲线名_标定"), QStringLiteral("曲线名称")},
        {QStringLiteral("点号_标定"), QStringLiteral("点号")},
        {QStringLiteral("测量值_标定"), QStringLiteral("测量值")},
        {QStringLiteral("标定值_标定"), QStringLiteral("标定值")},
        {QStringLiteral("温度_标定"), QStringLiteral("温度")},
    };
    const QByteArray utf8 = displayAliases.value(source, source).toUtf8();
    return QCoreApplication::translate("MainWindow", utf8.constData());
}

[[maybe_unused]] const char *const kParameterUiTexts[] = {
    QT_TRANSLATE_NOOP("MainWindow", "MCU模块"),
    QT_TRANSLATE_NOOP("MainWindow", "波特率"),
    QT_TRANSLATE_NOOP("MainWindow", "采样间隔"),
    QT_TRANSLATE_NOOP("MainWindow", "采样频率"),
    QT_TRANSLATE_NOOP("MainWindow", "采样点数"),
    QT_TRANSLATE_NOOP("MainWindow", "工作模式"),
    QT_TRANSLATE_NOOP("MainWindow", "发射功率"),
    QT_TRANSLATE_NOOP("MainWindow", "空中速率"),
    QT_TRANSLATE_NOOP("MainWindow", "主从模式"),
    QT_TRANSLATE_NOOP("MainWindow", "本地组号"),
    QT_TRANSLATE_NOOP("MainWindow", "本地地址"),
    QT_TRANSLATE_NOOP("MainWindow", "目标组号"),
    QT_TRANSLATE_NOOP("MainWindow", "目标地址"),
    QT_TRANSLATE_NOOP("MainWindow", "DTU模块"),
    QT_TRANSLATE_NOOP("MainWindow", "曲线名称"),
    QT_TRANSLATE_NOOP("MainWindow", "点号"),
    QT_TRANSLATE_NOOP("MainWindow", "测量值"),
    QT_TRANSLATE_NOOP("MainWindow", "标定值"),
    QT_TRANSLATE_NOOP("MainWindow", "温度"),
    QT_TRANSLATE_NOOP("MainWindow", "MCU卡"),
    QT_TRANSLATE_NOOP("MainWindow", "波特率_MCU"),
    QT_TRANSLATE_NOOP("MainWindow", "Gap_MCU"),
    QT_TRANSLATE_NOOP("MainWindow", "Mode_MCU"),
    QT_TRANSLATE_NOOP("MainWindow", "ModID_MCU"),
    QT_TRANSLATE_NOOP("MainWindow", "RS485_MCU"),
    QT_TRANSLATE_NOOP("MainWindow", "读MCU"),
    QT_TRANSLATE_NOOP("MainWindow", "写MCU"),
    QT_TRANSLATE_NOOP("MainWindow", "重启设备"),
    QT_TRANSLATE_NOOP("MainWindow", "LoRa设置"),
    QT_TRANSLATE_NOOP("MainWindow", "波特率_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "信道_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "功率_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "空速_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "工作模式_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "主/从_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "本地组号_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "本地地址_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "目标组号_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "目标地址_LR"),
    QT_TRANSLATE_NOOP("MainWindow", "读LoRa"),
    QT_TRANSLATE_NOOP("MainWindow", "写LoRa"),
    QT_TRANSLATE_NOOP("MainWindow", "DTU主卡"),
    QT_TRANSLATE_NOOP("MainWindow", "服务器IP"),
    QT_TRANSLATE_NOOP("MainWindow", "服务器端口"),
    QT_TRANSLATE_NOOP("MainWindow", "用户名"),
    QT_TRANSLATE_NOOP("MainWindow", "密码"),
    QT_TRANSLATE_NOOP("MainWindow", "协议类型"),
    QT_TRANSLATE_NOOP("MainWindow", "通道ID"),
    QT_TRANSLATE_NOOP("MainWindow", "写DTU"),
    QT_TRANSLATE_NOOP("MainWindow", "DTU测试"),
    QT_TRANSLATE_NOOP("MainWindow", "检测状态"),
    QT_TRANSLATE_NOOP("MainWindow", "配置摘要"),
    QT_TRANSLATE_NOOP("MainWindow", "标定"),
    QT_TRANSLATE_NOOP("MainWindow", "曲线名_标定"),
    QT_TRANSLATE_NOOP("MainWindow", "点号_标定"),
    QT_TRANSLATE_NOOP("MainWindow", "测量值_标定"),
    QT_TRANSLATE_NOOP("MainWindow", "标定值_标定"),
    QT_TRANSLATE_NOOP("MainWindow", "温度_标定"),
    QT_TRANSLATE_NOOP("MainWindow", "添加"),
    QT_TRANSLATE_NOOP("MainWindow", "删除"),
    QT_TRANSLATE_NOOP("MainWindow", "测量"),
    QT_TRANSLATE_NOOP("MainWindow", "修改"),
    QT_TRANSLATE_NOOP("MainWindow", "总线设备管理器"),
    QT_TRANSLATE_NOOP("MainWindow", "信道"),
    QT_TRANSLATE_NOOP("MainWindow", "组号"),
    QT_TRANSLATE_NOOP("MainWindow", "地址"),
    QT_TRANSLATE_NOOP("MainWindow", "数据数"),
    QT_TRANSLATE_NOOP("MainWindow", "设备名称"),
    QT_TRANSLATE_NOOP("MainWindow", "应答码"),
    QT_TRANSLATE_NOOP("MainWindow", "开始测试"),
    QT_TRANSLATE_NOOP("MainWindow", "DTU网络"),
    QT_TRANSLATE_NOOP("MainWindow", "网络选项卡"),
    QT_TRANSLATE_NOOP("MainWindow", "IP地址"),
    QT_TRANSLATE_NOOP("MainWindow", "端口"),
    QT_TRANSLATE_NOOP("MainWindow", "SSL加密"),
    QT_TRANSLATE_NOOP("MainWindow", "网络配置"),
    QT_TRANSLATE_NOOP("MainWindow", "发送"),
    QT_TRANSLATE_NOOP("MainWindow", "AT管理器"),
    QT_TRANSLATE_NOOP("MainWindow", "AT指令"),
    QT_TRANSLATE_NOOP("MainWindow", "AT次数"),
    QT_TRANSLATE_NOOP("MainWindow", "AT回包"),
    QT_TRANSLATE_NOOP("MainWindow", "AT说明"),
    QT_TRANSLATE_NOOP("MainWindow", "清除"),
    QT_TRANSLATE_NOOP("MainWindow", "菜单"),
    QT_TRANSLATE_NOOP("MainWindow", "字符"),
    QT_TRANSLATE_NOOP("MainWindow", "整数"),
    QT_TRANSLATE_NOOP("MainWindow", "浮点"),
};

QString deviceCategory(const gucds::DeviceRecord &record)
{
    const QString category = record.category.trimmed();
    return category.isEmpty() ? QCoreApplication::translate("MainWindow", "未分类") : category;
}

QString deviceDisplayName(const gucds::DeviceRecord &record)
{
    const QString model = record.model.trimmed();
    if (!model.isEmpty())
        return model;

    const QString name = record.name.trimmed();
    return name.isEmpty() ? QCoreApplication::translate("MainWindow", "未命名设备") : name;
}

bool isFrequencyTensionDevice(const gucds::DeviceRecord &record)
{
    return record.category.contains(QStringLiteral("频振索力"))
        || record.model.contains(QStringLiteral("QL-FOFS"), Qt::CaseInsensitive)
        || record.model.contains(QStringLiteral("QL-FOPS"), Qt::CaseInsensitive);
}

QString deviceIdentityKey(const gucds::DeviceRecord &record)
{
    return QStringList{record.name.trimmed(), record.category.trimmed(), record.model.trimmed()}.join(QChar(0x1f));
}

QString findLabviewProjectRoot()
{
    const QString envRoot = qEnvironmentVariable("QLIOT_LABVIEW_ROOT");
    if (!envRoot.isEmpty() && QDir(envRoot).exists())
        return QFileInfo(envRoot).absoluteFilePath();

    const QString projectName = QStringLiteral("General Upper Computer Debugging Software5.5");
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList basePaths = {
        QDir::currentPath(),
        appDir,
        QDir(appDir).absoluteFilePath(QStringLiteral("..")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../..")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../../..")),
    };

    for (const QString &basePath : basePaths) {
        const QString candidate = QDir(basePath).absoluteFilePath(projectName);
        if (QDir(candidate).exists())
            return QFileInfo(candidate).absoluteFilePath();
    }

    return {};
}

void setButtonRole(QPushButton *button, const char *role)
{
    button->setProperty("buttonRole", QString::fromLatin1(role));
}

void setActionButtonRole(QPushButton *button, const QString &action)
{
    if (action.contains(QStringLiteral("删除")) || action.contains(QStringLiteral("清空"))) {
        setButtonRole(button, "danger");
        return;
    }

    static const QStringList primaryActions = {
        QStringLiteral("保存"),
        QStringLiteral("写"),
        QStringLiteral("开始"),
        QStringLiteral("发送"),
        QStringLiteral("网络配置"),
    };
    if (std::any_of(primaryActions.cbegin(), primaryActions.cend(), [&action](const QString &keyword) {
            return action.contains(keyword);
        })) {
        setButtonRole(button, "primary");
    }
}

QWidget *buildPageHeader(QWidget *parent, const QString &title, const QString &subtitle)
{
    auto *header = new QWidget(parent);
    header->setObjectName(QStringLiteral("pageHeader"));
    auto *layout = new QVBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    auto *titleLabel = new QLabel(title, header);
    titleLabel->setObjectName(QStringLiteral("pageTitle"));
    layout->addWidget(titleLabel);

    auto *subtitleLabel = new QLabel(subtitle, header);
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
    subtitleLabel->setWordWrap(true);
    layout->addWidget(subtitleLabel);
    return header;
}

void markSurface(QWidget *widget)
{
    widget->setProperty("surfaceCard", true);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_deviceModel(new gucds::DeviceTableModel(this))
    , m_measurementModel(new gucds::MeasurementTableModel(this))
    , m_calibrationModel(new gucds::CalibrationTableModel(this))
    , m_busDeviceModel(new gucds::BusDeviceTableModel(this))
    , m_communicationController(new gucds::DeviceCommunicationController(this))
{
    // The application deliberately uses a light visual design. Tell the
    // Windows platform style to draw native subcontrols (checkboxes, spin
    // buttons, scrollbars, and title bar) with its light color scheme too.
    // Otherwise a dark OS theme can leak black native pieces into the light
    // stylesheet below.
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);

    setWindowTitle(gucds::AppConfig::applicationTitle());
    resize(kLabViewWidth, kLabViewHeight);
    setMinimumSize(1180, 690);

    buildMenus();
    setCentralWidget(buildCentralWidget());

    setStyleSheet(QString::fromLatin1(R"QSS(
        QMainWindow {
            background: #edf2f7;
            color: #172033;
        }
        QDialog { background: #f3f6f9; }
        QWidget {
            color: #172033;
            font: 9pt "Microsoft YaHei UI";
        }
        QWidget#centralRoot,
        QFrame#labviewShell,
        QWidget[workspacePage="true"],
        QWidget#deviceConfigViewport {
            background: #f3f6f9;
        }
        QMenuBar {
            background: #ffffff;
            border-bottom: 1px solid #d8e0e8;
            padding: 3px 8px;
        }
        QMenuBar::item {
            background: transparent;
            padding: 5px 12px;
            border-radius: 4px;
        }
        QMenuBar::item:selected {
            background: #eaf2fa;
            color: #0b5f9e;
        }
        QMenu {
            background: #ffffff;
            border: 1px solid #cbd5df;
            padding: 5px;
        }
        QMenu::item {
            padding: 6px 28px 6px 10px;
            border-radius: 3px;
        }
        QMenu::item:selected { background: #e8f2fb; color: #0b5f9e; }
        QStatusBar { background: #ffffff; border-top: 1px solid #d8e0e8; color: #526174; }
        QFrame#labviewShell { border: none; }
        QSplitter::handle { background: #dbe3eb; }
        QSplitter::handle:horizontal { width: 5px; }
        QSplitter::handle:vertical { height: 5px; }
        QSplitter::handle:hover { background: #9fb8cf; }
        QWidget[surfaceCard="true"],
        QGroupBox[surfaceCard="true"] {
            background: #ffffff;
            border: 1px solid #d8e1ea;
            border-radius: 6px;
        }
        QWidget#pageHeader { background: transparent; }
        QLabel#pageTitle {
            color: #172033;
            font-size: 15pt;
            font-weight: 600;
        }
        QLabel#pageSubtitle { color: #65758a; }
        QLabel#sectionTitle {
            background: transparent;
            color: #24344a;
            border-left: 3px solid #1769aa;
            padding-left: 8px;
            font-size: 10pt;
            font-weight: 600;
        }
        QLabel#fieldLabel { background: transparent; color: #526174; }
        QLabel#metricLabel { color: #65758a; }
        QLabel#metricValue { color: #172033; font-size: 12pt; font-weight: 600; }
        QTabWidget#mainTabs::pane {
            background: #f3f6f9;
            border: 1px solid #d8e1ea;
            top: -1px;
        }
        QTabBar#mainNavigationTabs::tab {
            background: #ffffff;
            color: #526174;
            border: none;
            border-bottom: 2px solid transparent;
            padding: 10px 18px;
            min-width: 88px;
        }
        QTabBar#mainNavigationTabs::tab:hover { color: #0b5f9e; background: #f6f9fc; }
        QTabBar#mainNavigationTabs::tab:selected {
            color: #0b5f9e;
            background: #ffffff;
            border-bottom-color: #1769aa;
            font-weight: 600;
        }
        QTabWidget#sideTools::pane,
        QTabWidget#deviceConfigSections::pane,
        QTabWidget#toolSections::pane {
            background: #ffffff;
            border: 1px solid #d8e1ea;
            top: -1px;
        }
        QTabBar#secondaryTabs::tab,
        QTabBar#deviceConfigSectionTabs::tab,
        QTabBar#toolSectionTabs::tab {
            background: #edf2f7;
            color: #526174;
            border: 1px solid #d5dee8;
            border-bottom: none;
            padding: 8px 14px;
            min-width: 76px;
        }
        QTabBar#toolSectionTabs::tab { padding: 8px 10px; min-width: 62px; }
        QTabBar#secondaryTabs::tab:selected,
        QTabBar#deviceConfigSectionTabs::tab:selected,
        QTabBar#toolSectionTabs::tab:selected {
            background: #ffffff;
            color: #0b5f9e;
            border-top: 2px solid #1769aa;
            font-weight: 600;
        }
        QTableView, QTreeWidget, QListWidget, QPlainTextEdit {
            background: #ffffff;
            alternate-background-color: #f7f9fb;
            border: 1px solid #cfd9e3;
            border-radius: 3px;
            gridline-color: #e3e9ef;
            selection-background-color: #dbeaf7;
            selection-color: #12324f;
        }
        QTableView::item, QTreeWidget::item, QListWidget::item { padding: 4px 6px; }
        QTreeWidget::item:hover, QListWidget::item:hover { background: #edf5fb; }
        QHeaderView {
            background: #eef3f7;
            color: #34465c;
        }
        QHeaderView::section {
            background: #eef3f7;
            color: #34465c;
            border: none;
            border-right: 1px solid #d7e0e8;
            border-bottom: 1px solid #cfd9e3;
            padding: 6px 7px;
            font-weight: 600;
        }
        QTableCornerButton::section {
            background: #eef3f7;
            border: none;
            border-right: 1px solid #d7e0e8;
            border-bottom: 1px solid #cfd9e3;
        }
        QLineEdit, QComboBox, QSpinBox {
            background: #ffffff;
            color: #172033;
            border: 1px solid #bdc9d6;
            border-radius: 4px;
            min-height: 28px;
            padding: 0 7px;
            selection-background-color: #1769aa;
            selection-color: #ffffff;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #172033;
            border: 1px solid #bdc9d6;
            selection-background-color: #dbeaf7;
            selection-color: #12324f;
            outline: 0;
        }
        QComboBox QAbstractItemView::item {
            min-height: 28px;
            padding: 3px 8px;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #1769aa; }
        QLineEdit:read-only { background: #f3f6f9; color: #4d5d70; }
        QPushButton {
            background: #ffffff;
            color: #28405a;
            border: 1px solid #b9c7d5;
            border-radius: 4px;
            min-height: 28px;
            padding: 0 12px;
        }
        QPushButton:hover { background: #edf5fb; border-color: #7da7c9; color: #0b5f9e; }
        QPushButton:pressed { background: #dcebf7; }
        QPushButton[buttonRole="primary"] {
            background: #1769aa;
            color: #ffffff;
            border-color: #1769aa;
        }
        QPushButton[buttonRole="primary"]:hover { background: #0e5b96; border-color: #0e5b96; }
        QPushButton[buttonRole="danger"] { color: #b4232c; border-color: #d8a3a7; }
        QPushButton[buttonRole="danger"]:hover { background: #fff0f1; border-color: #c75860; }
        QPushButton:disabled, QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {
            background: #e8edf2;
            color: #8b97a5;
            border-color: #d5dde5;
        }
        QGroupBox {
            background: #ffffff;
            border: 1px solid #d8e1ea;
            border-radius: 5px;
            margin-top: 12px;
            padding: 16px 10px 10px 10px;
            font-weight: 600;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
            color: #34465c;
        }
        QGroupBox QWidget { font-weight: 400; }
        QGroupBox[deviceEditorSection="true"] { background: #f8fafc; border-color: #e0e7ee; }
        QGroupBox[toolPanel="true"] { border: none; border-radius: 0; margin-top: 0; padding: 14px 10px 10px 10px; }
        QLabel#deviceEditorSource {
            background: #edf5fb;
            border: 1px solid #c8dced;
            border-radius: 4px;
            color: #24577e;
            padding: 8px 10px;
        }
        QPlainTextEdit#serialBuffer {
            background: #f8fafc;
            color: #24344a;
            border-color: #cfd9e3;
            font: 9pt "Consolas";
        }
        QScrollArea { background: transparent; border: none; }
        QCheckBox { color: #172033; spacing: 6px; }
        QCheckBox:disabled { color: #8b97a5; }
        QToolTip {
            background: #ffffff;
            color: #172033;
            border: 1px solid #bdc9d6;
            padding: 4px 6px;
        }
        QAbstractScrollArea::corner { background: #edf2f6; }
        QScrollBar:vertical { background: #edf2f6; width: 10px; margin: 0; }
        QScrollBar::handle:vertical { background: #b8c6d3; min-height: 28px; border-radius: 4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal { background: #edf2f6; height: 10px; margin: 0; }
        QScrollBar::handle:horizontal { background: #b8c6d3; min-width: 28px; border-radius: 4px; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
        QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
    )QSS"));

    loadPersistedLabviewData();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildMenus()
{
    QMenu *file = menuBar()->addMenu(tr("文件"));
    file->addAction(tr("新建"), this, [this] { appendStatus(tr("新建：已创建空工程上下文")); });
    file->addAction(tr("保存"), this, [this] { sendDeviceCommand(gucds::AtProtocol::command(gucds::AtCommand::ConfigSave)); });
    file->addSeparator();
    file->addAction(tr("退出"), this, &QWidget::close);

    QMenu *operation = menuBar()->addMenu(tr("操作"));
    operation->addAction(tr("搜索设备"), this, &MainWindow::scanDevices);
    operation->addAction(tr("添加至设备"), this, &MainWindow::addSelectedLibraryDeviceToConfig);
    operation->addSeparator();
    operation->addAction(tr("恢复出厂设置"), this, [this] {
        if (QMessageBox::warning(this,
                                 tr("恢复出厂设置"),
                                 tr("此操作会清除设备通信和传感器参数，确定继续吗？"),
                                 QMessageBox::Yes | QMessageBox::Cancel,
                                 QMessageBox::Cancel)
            != QMessageBox::Yes) {
            return;
        }
        if (m_serialProtocol && m_serialProtocol->currentText() == QStringLiteral("Modbus"))
            sendModbusCommand(gucds::SensorModbusCommand::RestoreFactory, QStringLiteral("restore_factory"));
        else
            sendDeviceCommand(gucds::AtProtocol::command(gucds::AtCommand::RestoreReference), QStringLiteral("restore_factory"));
    });
    operation->addAction(tr("复位和校准"), this, [this] {
        if (QMessageBox::question(this,
                                  tr("复位和校准"),
                                  tr("设备将重新计算零偏，测量期间请保持设备静止。确定继续吗？"),
                                  QMessageBox::Yes | QMessageBox::Cancel,
                                  QMessageBox::Cancel)
            != QMessageBox::Yes) {
            return;
        }
        if (m_serialProtocol && m_serialProtocol->currentText() == QStringLiteral("Modbus"))
            sendModbusCommand(gucds::SensorModbusCommand::Calibrate, QStringLiteral("calibrate"));
        else
            sendDeviceCommand(gucds::AtProtocol::command(gucds::AtCommand::Calibrate), QStringLiteral("calibrate"));
    });
    operation->addAction(tr("设置日期时间"), this, [this] {
        sendDeviceCommand(gucds::AtProtocol::buildSetDateTime(QDateTime::currentDateTime()),
                          QStringLiteral("set_datetime"));
    });

    QMenu *tools = menuBar()->addMenu(tr("工具"));
    tools->addAction(tr("读取频谱"), this, &MainWindow::readSpectrum);
    tools->addAction(tr("读附加数据"), this, [this] {
        sendDeviceCommand(gucds::AtProtocol::command(gucds::AtCommand::GetSecondaryData),
                          QStringLiteral("secondary_data"));
    });
    QAction *frequencyParametersAction = tools->addAction(
        tr("频振索力传感器扩展参数"), this, &MainWindow::openFrequencyTensionParameters);
    frequencyParametersAction->setObjectName(QStringLiteral("frequencyTensionParametersAction"));
    tools->addAction(tr("读取闪存传感器数据"), this, [this] {
        sendDeviceCommand(gucds::AtProtocol::command(gucds::AtCommand::GetStringData),
                          QStringLiteral("string_data"));
    });

    QMenu *developer = menuBar()->addMenu(tr("开发者"));
    developer->addAction(tr("产品管理"), this, &MainWindow::openProductManagement);
    developer->addAction(tr("磁通量传感器开发"), this, [this] {
        promptExtendedParameters(tr("磁通量传感器扩展参数"),
                                 {QStringLiteral("CMFS"), QStringLiteral("D90")});
    });

    QMenu *settings = menuBar()->addMenu(QStringLiteral("设置 (Settings)"));
    settings->setObjectName(QStringLiteral("settingsMenu"));
    QMenu *languageMenu = settings->addMenu(QStringLiteral("语言 (Language)"));
    languageMenu->setObjectName(QStringLiteral("languageMenu"));
    auto *languageGroup = new QActionGroup(languageMenu);
    languageGroup->setObjectName(QStringLiteral("languageActionGroup"));
    languageGroup->setExclusive(true);
    auto restoreLanguageSelection = [languageGroup] {
        const QString currentLanguage = ApplicationTranslator::currentLanguage();
        for (QAction *candidate : languageGroup->actions())
            candidate->setChecked(candidate->data().toString() == currentLanguage);
    };
    auto addLanguageAction = [this, languageMenu, languageGroup, restoreLanguageSelection](const QString &text,
                                                                                           const QString &language) {
        QAction *action = languageMenu->addAction(text);
        action->setObjectName(QStringLiteral("language:%1").arg(language));
        action->setData(language);
        action->setCheckable(true);
        action->setChecked(language == ApplicationTranslator::currentLanguage());
        languageGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, language, restoreLanguageSelection] {
            if (language == ApplicationTranslator::currentLanguage())
                return;
            if (m_communicationController->isRunning()) {
                QMessageBox::information(this,
                                         QStringLiteral("切换语言 (Switch Language)"),
                                         QStringLiteral("请等待当前通信任务完成或先取消任务，再切换语言。\n\n"
                                                        "Please wait for the current communication task to finish, "
                                                        "or cancel it before changing the language."));
                restoreLanguageSelection();
                return;
            }

            QMessageBox confirmation(QMessageBox::Question,
                                     QStringLiteral("切换语言 (Switch Language)"),
                                     QStringLiteral("切换语言后程序将关闭。请重新运行程序以应用新语言。\n\n"
                                                    "The application will close after changing the language.\n"
                                                    "Please restart it to apply the new language."),
                                     QMessageBox::NoButton,
                                     this);
            QPushButton *confirmButton = confirmation.addButton(
                QStringLiteral("确认并关闭 (Confirm and Close)"), QMessageBox::AcceptRole);
            confirmButton->setObjectName(QStringLiteral("languageRestartConfirm"));
            QPushButton *cancelButton = confirmation.addButton(
                QStringLiteral("取消 (Cancel)"), QMessageBox::RejectRole);
            cancelButton->setObjectName(QStringLiteral("languageRestartCancel"));
            confirmation.setDefaultButton(cancelButton);
            confirmation.exec();
            if (confirmation.clickedButton() != confirmButton) {
                restoreLanguageSelection();
                return;
            }
            emit languageChangeRequested(language);
        });
    };
    addLanguageAction(QStringLiteral("简体中文 (Chinese)"), ApplicationTranslator::chineseLanguage());
    addLanguageAction(QStringLiteral("English (英语)"), ApplicationTranslator::englishLanguage());

    QMenu *help = menuBar()->addMenu(tr("帮助"));
    help->addAction(tr("说明"), this, [this] { appendStatus(tr("说明：Qt 重构首版")); });
    help->addAction(tr("数据手册"), this, [this] { appendStatus(tr("数据手册：请查看项目 docs 目录")); });
    help->addAction(tr("关于我们"), this, [this] { appendStatus(gucds::AppConfig::organizationName()); });
}

QWidget *MainWindow::buildCentralWidget()
{
    auto *root = new QWidget(this);
    root->setObjectName(QStringLiteral("centralRoot"));
    auto *rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(12, 10, 12, 12);

    auto *shell = new QSplitter(Qt::Horizontal, root);
    shell->setObjectName(QStringLiteral("labviewShell"));
    shell->setChildrenCollapsible(false);

    auto *sidebar = new QWidget(shell);
    sidebar->setMinimumWidth(410);
    sidebar->setMaximumWidth(520);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(10);
    QWidget *deviceTablePanel = buildDeviceTablePanel();
    sidebarLayout->addWidget(deviceTablePanel, 3);

    auto *sideTools = new QTabWidget(sidebar);
    sideTools->setObjectName(QStringLiteral("sideTools"));
    sideTools->tabBar()->setObjectName(QStringLiteral("secondaryTabs"));
    sideTools->setDocumentMode(true);
    sideTools->addTab(buildDeviceLibraryPanel(), tr("设备库"));
    sideTools->addTab(buildSerialPanel(), tr("通信控制"));
    sidebarLayout->addWidget(sideTools, 4);
    connect(sideTools, &QTabWidget::currentChanged, this, [deviceTablePanel, sideTools](int index) {
        if (index == 1) {
            deviceTablePanel->setFixedHeight(220);
            sideTools->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        } else {
            deviceTablePanel->setMinimumHeight(0);
            deviceTablePanel->setMaximumHeight(QWIDGETSIZE_MAX);
            sideTools->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        }
        deviceTablePanel->updateGeometry();
        sideTools->updateGeometry();
    });

    shell->addWidget(sidebar);
    shell->addWidget(buildMainTabs());
    shell->setStretchFactor(0, 0);
    shell->setStretchFactor(1, 1);
    shell->setSizes({430, 835});

    rootLayout->addWidget(shell);
    return root;
}

QWidget *MainWindow::buildDeviceTablePanel()
{
    auto *panel = new QWidget(this);
    markSurface(panel);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(8);
    layout->addWidget(sectionTitle(tr("设备配置表")));

    auto *table = new QTableView(panel);
    m_deviceConfigTable = table;
    table->setObjectName(QStringLiteral("deviceConfigTable"));
    table->setModel(m_deviceModel);
    configureTable(table);
    table->setMinimumHeight(120);
    layout->addWidget(table);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(6);
    auto *deleteButton = labviewButton(tr("删除设备"));
    auto *moveUpButton = labviewButton(tr("上移"));
    auto *moveDownButton = labviewButton(tr("下移"));
    auto *clearButton = labviewButton(tr("清空配置表"));
    setButtonRole(deleteButton, "danger");
    setButtonRole(clearButton, "danger");
    buttonRow->addWidget(deleteButton);
    buttonRow->addWidget(moveUpButton);
    buttonRow->addWidget(moveDownButton);
    buttonRow->addWidget(clearButton);
    layout->addLayout(buttonRow);

    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::removeSelectedConfiguredDevice);
    connect(moveUpButton, &QPushButton::clicked, this, [this] { moveSelectedConfiguredDevice(-1); });
    connect(moveDownButton, &QPushButton::clicked, this, [this] { moveSelectedConfiguredDevice(1); });
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearConfiguredDevices);
    connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex &current) {
        if (current.isValid())
            showConfiguredDeviceDetails(current.row());
    });
    return panel;
}

QWidget *MainWindow::buildDeviceLibraryPanel()
{
    auto *panel = new QWidget(this);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    layout->addWidget(sectionTitle(tr("可用设备")));

    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(8);
    titleRow->addWidget(new QLabel(tr("设备名称"), panel));
    auto *deviceName = lineEdit();
    titleRow->addWidget(deviceName, 1);
    layout->addLayout(titleRow);

    m_deviceLibraryTree = new QTreeWidget(panel);
    m_deviceLibraryTree->setObjectName(QStringLiteral("deviceLibraryTree"));
    m_deviceLibraryTree->setHeaderHidden(true);
    m_deviceLibraryTree->setMinimumHeight(150);
    layout->addWidget(m_deviceLibraryTree);

    auto *actionRow = new QHBoxLayout;
    actionRow->addStretch();
    auto *addButton = labviewButton(tr("添加至设备"));
    setButtonRole(addButton, "primary");
    actionRow->addWidget(addButton);
    layout->addLayout(actionRow);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addSelectedLibraryDeviceToConfig);
    connect(m_deviceLibraryTree, &QTreeWidget::itemClicked, this, [this, deviceName](QTreeWidgetItem *item, int) {
        if (!item)
            return;
        const QVariant libraryIndex = item->data(0, kDeviceLibraryRecordRole);
        if (!libraryIndex.isValid())
            return;
        deviceName->setText(item->text(0));
        showLibraryDeviceDetails(libraryIndex.toInt());
    });
    return panel;
}

QWidget *MainWindow::buildSelectedDeviceEditorPanel()
{
    m_deviceEditorGroup = new QGroupBox(tr("当前设备参数"), this);
    m_deviceEditorGroup->setProperty("configCard", true);
    m_deviceEditorGroup->setProperty("surfaceCard", true);
    auto *layout = new QVBoxLayout(m_deviceEditorGroup);
    layout->setContentsMargins(14, 20, 14, 14);
    layout->setSpacing(12);

    m_deviceEditorSourceLabel = new QLabel(tr("从左侧设备库选择一个候选设备"), m_deviceEditorGroup);
    m_deviceEditorSourceLabel->setObjectName(QStringLiteral("deviceEditorSource"));
    m_deviceEditorSourceLabel->setWordWrap(true);
    layout->addWidget(m_deviceEditorSourceLabel);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    m_addEditedDeviceButton = labviewButton(tr("添加至设备"));
    m_applyEditedDeviceButton = labviewButton(tr("应用到配置表"));
    setButtonRole(m_addEditedDeviceButton, "primary");
    buttons->addStretch();
    buttons->addWidget(m_addEditedDeviceButton);
    buttons->addWidget(m_applyEditedDeviceButton);
    layout->addLayout(buttons);

    m_deviceNameEditor = lineEdit();
    m_deviceCategoryEditor = lineEdit();
    m_deviceModelEditor = lineEdit();
    m_deviceCalibrationPointsEditor = new QSpinBox;
    m_deviceCalibrationPointsEditor->setRange(0, 999);

    auto addGridField = [](QGridLayout *grid,
                           QWidget *parent,
                           const QString &labelText,
                           QWidget *editor,
                           int row,
                           int column) {
        auto *label = new QLabel(labelText, parent);
        label->setObjectName(QStringLiteral("fieldLabel"));
        label->setBuddy(editor);
        grid->addWidget(label, row, column, Qt::AlignRight | Qt::AlignVCenter);
        grid->addWidget(editor, row, column + 1);
    };

    auto *identityBox = new QGroupBox(tr("设备参数"), m_deviceEditorGroup);
    identityBox->setProperty("deviceEditorSection", true);
    auto *identityGrid = new QGridLayout(identityBox);
    identityGrid->setHorizontalSpacing(10);
    identityGrid->setVerticalSpacing(8);
    addGridField(identityGrid, identityBox, tr("设备名称"), m_deviceNameEditor, 0, 0);
    addGridField(identityGrid, identityBox, tr("设备类别"), m_deviceCategoryEditor, 0, 2);
    addGridField(identityGrid, identityBox, tr("规格型号"), m_deviceModelEditor, 1, 0);
    addGridField(identityGrid, identityBox, tr("标定点数"), m_deviceCalibrationPointsEditor, 1, 2);
    identityGrid->setColumnStretch(1, 1);
    identityGrid->setColumnStretch(3, 1);
    layout->addWidget(identityBox);

    auto *dataBox = new QGroupBox(tr("设备数据"), m_deviceEditorGroup);
    dataBox->setProperty("deviceEditorSection", true);
    auto *dataGrid = new QGridLayout(dataBox);
    dataGrid->setHorizontalSpacing(10);
    dataGrid->setVerticalSpacing(8);

    m_deviceDataEditors.clear();
    for (int index = 1; index <= 5; ++index) {
        auto *edit = lineEdit();
        m_deviceDataEditors.append(edit);
        const int fieldIndex = index - 1;
        addGridField(dataGrid,
                     dataBox,
                     tr("数据%1").arg(index),
                     edit,
                     fieldIndex / 3,
                     (fieldIndex % 3) * 2);
    }
    dataGrid->setColumnStretch(1, 1);
    dataGrid->setColumnStretch(3, 1);
    dataGrid->setColumnStretch(5, 1);
    layout->addWidget(dataBox);

    m_deviceModbusEditor = onOffCombo();
    m_deviceLoraEditor = onOffCombo();
    m_deviceDtuEditor = onOffCombo();

    auto *moduleBox = new QGroupBox(tr("模块功能"), m_deviceEditorGroup);
    moduleBox->setProperty("deviceEditorSection", true);
    auto *moduleGrid = new QGridLayout(moduleBox);
    moduleGrid->setHorizontalSpacing(12);
    const QStringList moduleNames = {QStringLiteral("Modbus"), QStringLiteral("LoRa"), QStringLiteral("DTU")};
    const QList<QWidget *> moduleEditors = {m_deviceModbusEditor, m_deviceLoraEditor, m_deviceDtuEditor};
    for (int index = 0; index < moduleEditors.size(); ++index) {
        auto *label = new QLabel(moduleNames.at(index), moduleBox);
        label->setBuddy(moduleEditors.at(index));
        moduleGrid->addWidget(label, 0, index);
        moduleGrid->addWidget(moduleEditors.at(index), 1, index);
        moduleGrid->setColumnStretch(index, 1);
    }
    layout->addWidget(moduleBox);

    auto *parameterBox = new QGroupBox(tr("传感器参数"), m_deviceEditorGroup);
    parameterBox->setProperty("deviceEditorSection", true);
    m_deviceParameterForm = new QFormLayout(parameterBox);
    m_deviceParameterForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_deviceParameterForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_deviceParameterForm->setHorizontalSpacing(10);
    m_deviceParameterForm->setVerticalSpacing(8);
    layout->addWidget(parameterBox);

    connect(m_addEditedDeviceButton, &QPushButton::clicked, this, &MainWindow::addEditedDeviceToConfig);
    connect(m_applyEditedDeviceButton, &QPushButton::clicked, this, &MainWindow::applyEditedDeviceToConfig);

    clearDeviceEditor();
    return m_deviceEditorGroup;
}

QWidget *MainWindow::buildSerialPanel()
{
    auto *panel = new QWidget(this);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    layout->addWidget(sectionTitle(tr("通信状态")));
    auto *statusGrid = new QGridLayout;
    statusGrid->setHorizontalSpacing(8);
    statusGrid->setVerticalSpacing(6);

    m_serialRx = lineEdit();
    m_serialRx->setReadOnly(true);
    m_serialTx = lineEdit();
    m_serialTx->setReadOnly(true);
    m_serialConnected = lineEdit(tr("未连接"));
    m_serialConnected->setReadOnly(true);
    m_serialProtocol = combo({QStringLiteral("Modbus"), tr("自定义TTL"), tr("自定义LoRa")});
    m_serialProtocol->setObjectName(QStringLiteral("serialProtocol"));

    statusGrid->addWidget(new QLabel(tr("接收数据"), panel), 0, 0);
    statusGrid->addWidget(m_serialRx, 0, 1);
    statusGrid->addWidget(new QLabel(tr("发送数据"), panel), 0, 2);
    statusGrid->addWidget(m_serialTx, 0, 3);
    statusGrid->addWidget(new QLabel(tr("串口连接"), panel), 1, 0);
    statusGrid->addWidget(m_serialConnected, 1, 1);
    statusGrid->addWidget(new QLabel(tr("通信协议"), panel), 1, 2);
    statusGrid->addWidget(m_serialProtocol, 1, 3);
    statusGrid->setColumnStretch(1, 1);
    statusGrid->setColumnStretch(3, 1);
    layout->addLayout(statusGrid);

    layout->addWidget(sectionTitle(tr("串口参数")));
    auto *parameterGrid = new QGridLayout;
    parameterGrid->setHorizontalSpacing(8);
    parameterGrid->setVerticalSpacing(6);

    QStringList ports = gucds::SerialSession::availablePorts();
    if (ports.isEmpty())
        ports = {QStringLiteral("COM1")};
    m_serialPort = combo(ports);
    m_serialPort->setObjectName(QStringLiteral("serialPort"));
    m_serialPort->setEditable(true);
    auto *refreshPortsButton = labviewButton(tr("刷新"));
    refreshPortsButton->setToolTip(tr("重新扫描可用串口"));

    m_serialBaud = combo(gucds::AppConfig::baudRates());
    m_serialBaud->setObjectName(QStringLiteral("serialBaud"));
    m_serialBaud->setEditable(true);
    m_serialBaud->setCurrentText(QStringLiteral("9600"));
    m_serialSlaveId = byteSpinBox(1);
    m_serialSlaveId->setObjectName(QStringLiteral("serialSlaveId"));
    m_serialSlaveId->setMinimum(1);

    parameterGrid->addWidget(new QLabel(tr("串口号"), panel), 0, 0);
    parameterGrid->addWidget(m_serialPort, 0, 1);
    parameterGrid->addWidget(refreshPortsButton, 0, 2, 1, 2);
    parameterGrid->addWidget(new QLabel(tr("波特率"), panel), 1, 0);
    parameterGrid->addWidget(m_serialBaud, 1, 1);
    parameterGrid->addWidget(new QLabel(tr("从站ID"), panel), 1, 2);
    parameterGrid->addWidget(m_serialSlaveId, 1, 3);
    parameterGrid->setColumnStretch(1, 1);
    parameterGrid->setColumnStretch(3, 1);
    layout->addLayout(parameterGrid);

    m_serialBuffer = new QPlainTextEdit(panel);
    m_serialBuffer->setObjectName(QStringLiteral("serialBuffer"));
    m_serialBuffer->setReadOnly(true);
    m_serialBuffer->setMaximumBlockCount(5000);
    m_serialBuffer->setMinimumHeight(64);
    layout->addWidget(m_serialBuffer, 1);

    m_serialHexDisplay = new QCheckBox(tr("十六进制"), panel);
    m_serialHexDisplay->setObjectName(QStringLiteral("serialHexDisplay"));
    m_serialHexDisplay->setToolTip(tr("以空格分隔的十六进制字节显示接收数据"));
    auto *listenButton = labviewButton(tr("监听"));
    listenButton->setObjectName(QStringLiteral("serialListen"));
    listenButton->setToolTip(tr("监听串口数据三秒；收到一帧后立即显示"));
    m_serialStartButton = labviewButton(tr("开始"));
    m_serialStartButton->setObjectName(QStringLiteral("serialStart"));
    setButtonRole(m_serialStartButton, "primary");

    auto *actionRow = new QHBoxLayout;
    actionRow->setSpacing(6);
    actionRow->addWidget(m_serialHexDisplay);
    actionRow->addStretch();
    actionRow->addWidget(listenButton);
    actionRow->addWidget(m_serialStartButton);
    layout->addLayout(actionRow);

    auto refreshPorts = [this] {
        const QString currentPort = m_serialPort->currentText();
        QStringList refreshedPorts = gucds::SerialSession::availablePorts();
        if (refreshedPorts.isEmpty())
            refreshedPorts = {currentPort.isEmpty() ? QStringLiteral("COM1") : currentPort};

        m_serialPort->blockSignals(true);
        m_serialPort->clear();
        m_serialPort->addItems(refreshedPorts);
        const int previousIndex = m_serialPort->findText(currentPort);
        m_serialPort->setCurrentIndex(previousIndex >= 0 ? previousIndex : 0);
        m_serialPort->blockSignals(false);
    };

    connect(refreshPortsButton, &QPushButton::clicked, this, [this, refreshPorts] {
        refreshPorts();
        m_serialBuffer->appendPlainText(tr("串口列表已刷新"));
    });

    connect(m_communicationController,
            &gucds::DeviceCommunicationController::runningChanged,
            this,
            [this, refreshPortsButton](bool running) {
                m_serialPort->setEnabled(!running);
                m_serialBaud->setEnabled(!running);
                m_serialProtocol->setEnabled(!running);
                m_serialSlaveId->setEnabled(!running);
                refreshPortsButton->setEnabled(!running);
                m_serialStartButton->setEnabled(true);
                m_serialStartButton->setText(
                    running && m_frequencyTensionMeasurementActive
                        ? tr("停止测量")
                        : (running ? tr("取消") : tr("开始")));
                if (!running && !m_frequencyTensionMeasurementActive)
                    m_measurementDeviceRecord = {};
                if (running)
                    m_serialBuffer->appendPlainText(tr("正在执行通信任务..."));
            });
    connect(m_communicationController,
            &gucds::DeviceCommunicationController::frequencyTensionSampleReady,
            this,
            &MainWindow::handleCommunicationResult);
    connect(m_communicationController,
            &gucds::DeviceCommunicationController::resultReady,
            this,
            &MainWindow::handleCommunicationResult);
    connect(m_serialStartButton, &QPushButton::clicked, this, [this] {
        if (m_communicationController->isRunning()) {
            m_communicationController->cancel();
            m_serialStartButton->setText(tr("正在取消"));
            m_serialStartButton->setEnabled(false);
            return;
        }
        startMeasurement();
    });
    connect(listenButton, &QPushButton::clicked, this, [this] {
        if (m_communicationController->isRunning()) {
            appendStatus(tr("已有通信任务正在运行，请稍后重试"));
            return;
        }
        if (!configureCommunication())
            return;
        if (!m_communicationController->listen())
            appendStatus(tr("无法启动串口监听"));
    });
    return panel;
}

QWidget *MainWindow::buildMainTabs()
{
    auto *tabs = new QTabWidget(this);
    m_mainTabs = tabs;
    tabs->setObjectName(QStringLiteral("mainTabs"));
    tabs->tabBar()->setObjectName(QStringLiteral("mainNavigationTabs"));
    tabs->setDocumentMode(true);
    const QStringList tabNames = gucds::AppConfig::mainTabs();
    tabs->addTab(buildTestDataTab(), tabNames.value(0));
    tabs->addTab(buildDeviceConfigTab(), tabNames.value(1));
    tabs->addTab(buildCalibrationTab(), tabNames.value(2));
    tabs->addTab(buildBusGatewayTab(), tabNames.value(3));
    tabs->addTab(new gucds::CanMonitorWidget(tabs), tabNames.value(4));
    return tabs;
}

QWidget *MainWindow::buildTestDataTab()
{
    auto *page = new QWidget(this);
    page->setProperty("workspacePage", true);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(12);
    layout->addWidget(buildPageHeader(page,
                                      tr("测试数据"),
                                      tr("集中查看采集数据、最新状态和趋势曲线。")));

    auto *summaryPanel = new QWidget(page);
    markSurface(summaryPanel);
    auto *summaryLayout = new QVBoxLayout(summaryPanel);
    summaryLayout->setContentsMargins(12, 10, 12, 12);
    summaryLayout->setSpacing(8);
    summaryLayout->addWidget(sectionTitle(tr("接收概览")));

    auto *rxBytes = lineEdit();
    rxBytes->setReadOnly(true);
    auto *receiveCursor = lineEdit();
    receiveCursor->setReadOnly(true);
    auto *receiveLength = lineEdit();
    receiveLength->setReadOnly(true);
    auto *remainBytes = lineEdit();
    remainBytes->setReadOnly(true);

    auto *metricGrid = new QGridLayout;
    metricGrid->setHorizontalSpacing(12);
    metricGrid->setVerticalSpacing(4);
    const QStringList metricLabels = {tr("接收数据"), tr("接收游标"), tr("接收字长"), tr("剩余字长")};
    const QList<QLineEdit *> metricEditors = {rxBytes, receiveCursor, receiveLength, remainBytes};
    for (int index = 0; index < metricEditors.size(); ++index) {
        auto *label = new QLabel(metricLabels.at(index), summaryPanel);
        label->setObjectName(QStringLiteral("metricLabel"));
        metricGrid->addWidget(label, 0, index);
        metricGrid->addWidget(metricEditors.at(index), 1, index);
        metricGrid->setColumnStretch(index, index == 0 ? 2 : 1);
    }
    summaryLayout->addLayout(metricGrid);
    layout->addWidget(summaryPanel);

    auto *contentSplitter = new QSplitter(Qt::Vertical, page);
    contentSplitter->setChildrenCollapsible(false);

    auto *dataPanel = new QWidget(contentSplitter);
    markSurface(dataPanel);
    auto *dataLayout = new QVBoxLayout(dataPanel);
    dataLayout->setContentsMargins(12, 10, 12, 12);
    dataLayout->setSpacing(8);
    m_measurementSectionTitle = sectionTitle(tr("测量数据"));
    m_measurementSectionTitle->setObjectName(QStringLiteral("measurementSectionTitle"));
    dataLayout->addWidget(m_measurementSectionTitle);

    auto *dataRow = new QHBoxLayout;
    dataRow->setSpacing(10);

    auto *measurementPanel = new QWidget(page);
    auto *measurementLayout = new QVBoxLayout(measurementPanel);
    measurementLayout->setContentsMargins(0, 0, 0, 0);
    measurementLayout->setSpacing(6);

    auto *measurementTable = new QTableView(measurementPanel);
    m_measurementTable = measurementTable;
    measurementTable->setObjectName(QStringLiteral("measurementTable"));
    measurementTable->setModel(m_measurementModel);
    configureTable(measurementTable);
    measurementTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_deleteMeasurementAction = new QAction(tr("删除选中记录"), measurementTable);
    m_deleteMeasurementAction->setObjectName(QStringLiteral("deleteSelectedMeasurementsAction"));
    m_deleteMeasurementAction->setShortcut(QKeySequence::Delete);
    m_deleteMeasurementAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_saveMeasurementsAction = new QAction(tr("保存测量数据为 CSV…"), measurementTable);
    m_saveMeasurementsAction->setObjectName(QStringLiteral("saveMeasurementsCsvAction"));
    m_saveMeasurementsAction->setShortcut(QKeySequence::SaveAs);
    m_saveMeasurementsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    measurementTable->addAction(m_deleteMeasurementAction);
    measurementTable->addAction(m_saveMeasurementsAction);
    auto *measurementMenu = new QMenu(measurementTable);
    measurementMenu->setObjectName(QStringLiteral("measurementContextMenu"));
    measurementMenu->addAction(m_saveMeasurementsAction);
    measurementMenu->addSeparator();
    measurementMenu->addAction(m_deleteMeasurementAction);
    measurementTable->viewport()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(measurementTable->viewport(),
            &QWidget::customContextMenuRequested,
            this,
            [this, measurementTable, measurementMenu](const QPoint &position) {
                const QModelIndex clickedIndex = measurementTable->indexAt(position);
                if (clickedIndex.isValid()
                    && !measurementTable->selectionModel()->isRowSelected(
                        clickedIndex.row(), clickedIndex.parent())) {
                    measurementTable->selectRow(clickedIndex.row());
                }
                updateMeasurementActions();
                measurementMenu->popup(measurementTable->viewport()->mapToGlobal(position));
            });
    connect(m_deleteMeasurementAction, &QAction::triggered, this, &MainWindow::deleteSelectedMeasurements);
    connect(m_saveMeasurementsAction, &QAction::triggered, this, &MainWindow::saveMeasurementsAsCsv);
    auto *measurementActionRow = new QHBoxLayout;
    measurementActionRow->setSpacing(6);
    measurementActionRow->addStretch();
    m_saveMeasurementsButton = labviewButton(tr("保存数据…"));
    m_saveMeasurementsButton->setObjectName(QStringLiteral("saveMeasurementsButton"));
    m_saveMeasurementsButton->setToolTip(tr("将当前测量列表保存为 CSV；也可在表格内右键保存。"));
    m_deleteMeasurementButton = labviewButton(tr("删除选中"));
    m_deleteMeasurementButton->setObjectName(QStringLiteral("deleteSelectedMeasurementsButton"));
    m_deleteMeasurementButton->setToolTip(tr("删除表格中选中的测量记录；也可按 Delete 键。"));
    setButtonRole(m_deleteMeasurementButton, "danger");
    connect(m_saveMeasurementsButton, &QPushButton::clicked, m_saveMeasurementsAction, &QAction::trigger);
    connect(m_deleteMeasurementButton, &QPushButton::clicked, m_deleteMeasurementAction, &QAction::trigger);
    measurementActionRow->addWidget(m_saveMeasurementsButton);
    measurementActionRow->addWidget(m_deleteMeasurementButton);
    measurementLayout->addWidget(measurementTable, 1);
    measurementLayout->addLayout(measurementActionRow);
    connect(measurementTable->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            [this] { updateMeasurementActions(); });
    connect(m_measurementModel, &QAbstractItemModel::rowsInserted, this, [this] { updateMeasurementActions(); });
    connect(m_measurementModel, &QAbstractItemModel::rowsRemoved, this, [this] { updateMeasurementActions(); });
    connect(m_measurementModel, &QAbstractItemModel::modelReset, this, [this] { updateMeasurementActions(); });
    updateMeasurementActions();
    measurementTable->setMinimumHeight(90);
    dataRow->addWidget(measurementPanel, 5);

    auto *latestPanel = new QWidget(page);
    markSurface(latestPanel);
    auto *latestLayout = new QVBoxLayout(latestPanel);
    latestLayout->setContentsMargins(10, 8, 10, 10);
    latestLayout->setSpacing(6);
    latestLayout->addWidget(sectionTitle(tr("最新数据")));
    auto *latestList = new QListWidget(latestPanel);
    m_latestDataList = latestList;
    latestList->setObjectName(QStringLiteral("latestDataList"));
    latestLayout->addWidget(latestList, 1);
    auto *latestBand = lineEdit(tr("等待测量数据"));
    m_latestBand = latestBand;
    latestBand->setObjectName(QStringLiteral("latestBand"));
    latestBand->setReadOnly(true);
    auto *latestCount = lineEdit(QStringLiteral("0"));
    m_latestCount = latestCount;
    latestCount->setObjectName(QStringLiteral("latestCount"));
    latestCount->setReadOnly(true);
    latestCount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    latestLayout->addWidget(latestBand);
    latestLayout->addWidget(latestCount);
    dataRow->addWidget(latestPanel, 2);
    dataLayout->addLayout(dataRow);

    auto *plotPanel = new QWidget(contentSplitter);
    markSurface(plotPanel);
    auto *plotLayout = new QVBoxLayout(plotPanel);
    plotLayout->setContentsMargins(12, 10, 12, 12);
    plotLayout->setSpacing(8);

    auto *plotHeader = new QHBoxLayout;
    plotHeader->addWidget(sectionTitle(tr("数据曲线")));
    plotHeader->addStretch();
    plotHeader->addWidget(new QLabel(tr("曲线"), plotPanel));
    plotHeader->addWidget(combo({tr("曲线0")}));
    plotLayout->addLayout(plotHeader);

    m_plot = new SpectrumPlotWidget(plotPanel);
    m_plot->setObjectName(QStringLiteral("spectrumPlot"));
    m_plot->setMinimumHeight(120);
    plotLayout->addWidget(m_plot, 1);

    contentSplitter->addWidget(dataPanel);
    contentSplitter->addWidget(plotPanel);
    contentSplitter->setStretchFactor(0, 1);
    contentSplitter->setStretchFactor(1, 1);
    contentSplitter->setSizes({255, 245});
    layout->addWidget(contentSplitter, 1);
    return page;
}

QWidget *MainWindow::buildDeviceConfigTab()
{
    auto *page = new QWidget(this);
    page->setObjectName(QStringLiteral("deviceConfigPage"));
    page->setProperty("workspacePage", true);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(12);
    layout->addWidget(buildPageHeader(page,
                                      tr("设备配置"),
                                      tr("先选择设备，再按模块完成参数读取、编辑和写入。")));

    auto *sections = new QTabWidget(page);
    sections->setObjectName(QStringLiteral("deviceConfigSections"));
    sections->tabBar()->setObjectName(QStringLiteral("deviceConfigSectionTabs"));
    sections->setDocumentMode(true);

    auto addSection = [sections](QWidget *content, const QString &tabText, int maximumContentWidth) {
        auto *viewport = new QWidget(sections);
        viewport->setObjectName(QStringLiteral("deviceConfigViewport"));
        auto *viewportLayout = new QVBoxLayout(viewport);
        viewportLayout->setContentsMargins(16, 14, 16, 16);
        viewportLayout->setSpacing(0);

        auto *contentRow = new QHBoxLayout;
        contentRow->setContentsMargins(0, 0, 0, 0);
        contentRow->addStretch(1);
        content->setMaximumWidth(maximumContentWidth);
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        contentRow->addWidget(content, 6);
        contentRow->addStretch(1);
        viewportLayout->addLayout(contentRow);
        viewportLayout->addStretch(1);

        auto *scrollArea = new QScrollArea(sections);
        scrollArea->setObjectName(QStringLiteral("deviceConfigScrollArea"));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setWidget(viewport);
        sections->addTab(scrollArea, tabText);
    };

    addSection(buildSelectedDeviceEditorPanel(), tr("设备参数"), 760);

    QWidget *mcuGroup = buildParameterGroup(
        QStringLiteral("MCU卡"),
        {QStringLiteral("波特率_MCU"), QStringLiteral("Gap_MCU"), QStringLiteral("Mode_MCU"),
         QStringLiteral("ModID_MCU"), QStringLiteral("RS485_MCU"),
         QStringLiteral("采样频率_FVCF"), QStringLiteral("采样点数_FVCF")},
        {QStringLiteral("读MCU"), QStringLiteral("写MCU"), QStringLiteral("重启设备")});
    mcuGroup->setProperty("configCard", true);
    mcuGroup->setProperty("surfaceCard", true);
    addSection(mcuGroup, translatedMainWindowText(QStringLiteral("MCU卡")), 620);

    QWidget *loraGroup = buildParameterGroup(
        QStringLiteral("LoRa设置"),
        {QStringLiteral("波特率_LR"), QStringLiteral("信道_LR"), QStringLiteral("功率_LR"), QStringLiteral("空速_LR"), QStringLiteral("工作模式_LR"), QStringLiteral("主/从_LR"), QStringLiteral("本地组号_LR"), QStringLiteral("本地地址_LR"), QStringLiteral("目标组号_LR"), QStringLiteral("目标地址_LR")},
        {QStringLiteral("读LoRa"), QStringLiteral("写LoRa")});
    loraGroup->setProperty("configCard", true);
    loraGroup->setProperty("surfaceCard", true);
    addSection(loraGroup, translatedMainWindowText(QStringLiteral("LoRa设置")), 620);

    QWidget *dtuGroup = buildParameterGroup(
        QStringLiteral("DTU主卡"),
        {QStringLiteral("配置摘要")},
        {QStringLiteral("网络配置"), QStringLiteral("检测状态")});
    dtuGroup->setProperty("configCard", true);
    dtuGroup->setProperty("surfaceCard", true);
    addSection(dtuGroup, translatedMainWindowText(QStringLiteral("DTU主卡")), 620);

    layout->addWidget(sections, 1);
    return page;
}

QWidget *MainWindow::buildCalibrationTab()
{
    auto *page = new QWidget(this);
    page->setProperty("workspacePage", true);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(12);
    layout->addWidget(buildPageHeader(page,
                                      tr("设备标定"),
                                      tr("管理标定点，并在右侧完成测量与保存。")));

    auto *splitter = new QSplitter(Qt::Horizontal, page);
    splitter->setChildrenCollapsible(false);

    auto *tablePanel = new QWidget(splitter);
    markSurface(tablePanel);
    auto *tableLayout = new QVBoxLayout(tablePanel);
    tableLayout->setContentsMargins(12, 10, 12, 12);
    tableLayout->setSpacing(8);
    tableLayout->addWidget(sectionTitle(tr("标定记录")));

    m_calibrationTable = new QTableView(tablePanel);
    m_calibrationTable->setObjectName(QStringLiteral("calibrationTable"));
    m_calibrationTable->setModel(m_calibrationModel);
    configureTable(m_calibrationTable);
    QHeaderView *calibrationHeader = m_calibrationTable->horizontalHeader();
    calibrationHeader->setResizeContentsPrecision(50);
    for (int column = 0; column < 5; ++column)
        calibrationHeader->setSectionResizeMode(column, QHeaderView::ResizeToContents);
    calibrationHeader->setSectionResizeMode(5, QHeaderView::Stretch);
    tableLayout->addWidget(m_calibrationTable);

    QWidget *editor = buildParameterGroup(QStringLiteral("标定"),
                                          {QStringLiteral("曲线名_标定"), QStringLiteral("点号_标定"), QStringLiteral("测量值_标定"), QStringLiteral("标定值_标定"), QStringLiteral("温度_标定")},
                                          {QStringLiteral("添加"), QStringLiteral("保存"), QStringLiteral("删除"), QStringLiteral("测量"), QStringLiteral("修改")});
    editor->setMinimumWidth(260);
    editor->setMaximumWidth(340);
    splitter->addWidget(editor);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setSizes({520, 290});
    layout->addWidget(splitter, 1);
    connect(m_calibrationTable, &QTableView::clicked, this, [this](const QModelIndex &index) {
        const gucds::CalibrationRecord record = m_calibrationModel->recordAt(index.row());
        setParameterText(QStringLiteral("标定"), QStringLiteral("曲线名_标定"), record.curveName);
        setParameterText(QStringLiteral("标定"), QStringLiteral("点号_标定"), QString::number(record.point));
        setParameterText(QStringLiteral("标定"), QStringLiteral("测量值_标定"), QString::number(record.measuredValue, 'g', 12));
        setParameterText(QStringLiteral("标定"), QStringLiteral("标定值_标定"), QString::number(record.referenceValue, 'g', 12));
        setParameterText(QStringLiteral("标定"), QStringLiteral("温度_标定"), QString::number(record.temperature, 'g', 12));
    });
    return page;
}

QWidget *MainWindow::buildBusGatewayTab()
{
    auto *page = new QWidget(this);
    page->setProperty("workspacePage", true);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(12);
    layout->addWidget(buildPageHeader(page,
                                      tr("总线设备/网关"),
                                      tr("维护总线设备，并集中处理 DTU 与 AT 指令。")));

    auto *splitter = new QSplitter(Qt::Horizontal, page);
    splitter->setChildrenCollapsible(false);

    auto *tablePanel = new QWidget(splitter);
    markSurface(tablePanel);
    auto *tableLayout = new QVBoxLayout(tablePanel);
    tableLayout->setContentsMargins(12, 10, 12, 12);
    tableLayout->setSpacing(8);
    tableLayout->addWidget(sectionTitle(tr("总线设备列表")));
    m_busDeviceTable = new QTableView(tablePanel);
    m_busDeviceTable->setModel(m_busDeviceModel);
    configureTable(m_busDeviceTable);
    const QList<int> busColumnWidths = {46, 98, 94, 52, 52, 52, 58, 76};
    for (int column = 0; column < busColumnWidths.size(); ++column)
        m_busDeviceTable->horizontalHeader()->resizeSection(column, busColumnWidths.at(column));
    tableLayout->addWidget(m_busDeviceTable);

    auto *toolSections = new QTabWidget(splitter);
    toolSections->setObjectName(QStringLiteral("toolSections"));
    toolSections->tabBar()->setObjectName(QStringLiteral("toolSectionTabs"));
    toolSections->setDocumentMode(true);
    toolSections->setMinimumWidth(300);
    auto addToolSection = [toolSections](QWidget *content, const QString &tabText) {
        content->setProperty("toolPanel", true);
        if (auto *group = qobject_cast<QGroupBox *>(content))
            group->setTitle({});
        toolSections->addTab(content, tabText);
    };
    addToolSection(buildParameterGroup(QStringLiteral("总线设备管理器"),
                                       {QStringLiteral("信道"), QStringLiteral("组号"), QStringLiteral("地址"), QStringLiteral("数据数"), QStringLiteral("设备名称"), QStringLiteral("应答码")},
                                       {QStringLiteral("保存"), QStringLiteral("修改"), QStringLiteral("删除"), QStringLiteral("开始测试")}),
                   tr("设备管理"));
    addToolSection(buildParameterGroup(QStringLiteral("DTU网络"),
                                       {QStringLiteral("配置摘要")},
                                       {QStringLiteral("网络配置"), QStringLiteral("检测状态")}),
                   tr("DTU网络"));
    addToolSection(buildParameterGroup(QStringLiteral("AT管理器"),
                                       {QStringLiteral("AT指令"), QStringLiteral("AT次数"), QStringLiteral("AT回包"), QStringLiteral("AT说明")},
                                       {QStringLiteral("发送"), QStringLiteral("清除")}),
                   tr("AT指令"));
    splitter->addWidget(toolSections);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({470, 340});
    layout->addWidget(splitter, 1);
    connect(m_busDeviceTable, &QTableView::clicked, this, [this](const QModelIndex &index) {
        const gucds::BusDeviceRecord record = m_busDeviceModel->recordAt(index.row());
        setParameterText(QStringLiteral("总线设备管理器"), QStringLiteral("信道"), QString::number(record.channel));
        setParameterText(QStringLiteral("总线设备管理器"), QStringLiteral("组号"), QString::number(record.group));
        setParameterText(QStringLiteral("总线设备管理器"), QStringLiteral("地址"), QString::number(record.address));
        setParameterText(QStringLiteral("总线设备管理器"), QStringLiteral("数据数"), QString::number(record.dataCount));
        setParameterText(QStringLiteral("总线设备管理器"), QStringLiteral("设备名称"), record.sensorName);
        setParameterText(QStringLiteral("总线设备管理器"), QStringLiteral("应答码"), record.responseCode);
    });
    return page;
}

QWidget *MainWindow::buildParameterGroup(const QString &title, const QStringList &labels, const QStringList &buttons)
{
    auto *box = new QGroupBox(translatedMainWindowText(title), this);
    box->setProperty("surfaceCard", true);
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(14, 20, 14, 14);
    layout->setSpacing(10);

    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(8);
    for (const QString &label : labels) {
        QWidget *editor = nullptr;
        if (label == QStringLiteral("波特率_MCU") || label == QStringLiteral("波特率_LR")) {
            editor = combo(gucds::AppConfig::deviceBaudRates());
        } else if (label.contains(QStringLiteral("波特率"))) {
            editor = combo(gucds::AppConfig::baudRates());
        } else if (label == QStringLiteral("Gap_MCU")) {
            editor = combo({QStringLiteral("0s"), QStringLiteral("5s"), QStringLiteral("30s"), QStringLiteral("1min"), QStringLiteral("30min"), QStringLiteral("1h"), QStringLiteral("2h"), QStringLiteral("12h"), QStringLiteral("24h")});
        } else if (label == QStringLiteral("采样频率_FVCF")) {
            editor = combo({QStringLiteral("5"), QStringLiteral("10"), QStringLiteral("20"),
                            QStringLiteral("50"), QStringLiteral("100"), QStringLiteral("200"),
                            QStringLiteral("400"), QStringLiteral("500"), QStringLiteral("800"),
                            QStringLiteral("1000"), QStringLiteral("2000")});
            qobject_cast<QComboBox *>(editor)->setCurrentText(QStringLiteral("100"));
            editor->setEnabled(false);
        } else if (label == QStringLiteral("采样点数_FVCF")) {
            editor = combo({QStringLiteral("256"), QStringLiteral("512"),
                            QStringLiteral("1024"), QStringLiteral("2048")});
            editor->setEnabled(false);
        } else if (label == QStringLiteral("功率_LR")) {
            editor = combo(gucds::AppConfig::loraPowers());
        } else if (label == QStringLiteral("空速_LR")) {
            editor = combo(gucds::AppConfig::loraAirRates());
        } else if (label == QStringLiteral("工作模式_LR")) {
            editor = combo(gucds::AppConfig::loraWorkModes());
        } else if (label == QStringLiteral("主/从_LR")) {
            editor = combo(gucds::AppConfig::loraMasterSlaveModes());
        } else if (label == QStringLiteral("Mode_MCU")) {
            editor = combo(gucds::AppConfig::mcuModes());
        } else if (label == QStringLiteral("RS485_MCU")) {
            editor = onOffCombo();
        } else if (label == QStringLiteral("IPV6")) {
            editor = combo({QStringLiteral("IPV4"), QStringLiteral("IPV6")});
        } else if (label == QStringLiteral("SSL加密")) {
            editor = combo({tr("不加密"), tr("无证书加密"), tr("有证书加密")});
        } else if (label == QStringLiteral("请求方法")) {
            editor = combo({QStringLiteral("POST"), QStringLiteral("GET")});
        } else if (label == QStringLiteral("协议类型")) {
            editor = combo(gucds::AppConfig::networkProtocols());
        } else if (label == QStringLiteral("AT次数")) {
            auto *count = new QSpinBox;
            count->setRange(1, 1000);
            count->setValue(1);
            editor = count;
        } else if (label == QStringLiteral("服务器端口") || label == QStringLiteral("端口")) {
            auto *port = new QSpinBox;
            port->setRange(1, 65535);
            port->setValue(gucds::AppConfig::defaultServerPort());
            editor = port;
        } else if (label == QStringLiteral("通道ID")) {
            auto *channel = byteSpinBox(1);
            channel->setMinimum(1);
            editor = channel;
        } else if (label.contains(QStringLiteral("ID")) || label.contains(QStringLiteral("地址")) || label.contains(QStringLiteral("组号")) || label.contains(QStringLiteral("信道"))) {
            editor = byteSpinBox();
        } else {
            editor = lineEdit();
        }
        if (auto *edit = qobject_cast<QLineEdit *>(editor)) {
            if (label == QStringLiteral("密码"))
                edit->setEchoMode(QLineEdit::Password);
            if (label == QStringLiteral("AT回包") || label == QStringLiteral("AT说明")
                || label == QStringLiteral("应答码") || label == QStringLiteral("配置摘要"))
                edit->setReadOnly(true);
            if (label == QStringLiteral("服务器IP") || label == QStringLiteral("IP地址"))
                edit->setText(gucds::AppConfig::defaultServerHost());
            if (label == QStringLiteral("URL路径"))
                edit->setText(QStringLiteral("/"));
            if (label == QStringLiteral("配置摘要"))
                edit->setText(QStringLiteral("MQTT | 43.139.170.206:1002 | UART"));
        }
        editor->setObjectName(QStringLiteral("parameter:%1:%2").arg(title, label));
        m_parameterEditors.insert(title + QChar(0x1f) + label, editor);
        form->addRow(translatedMainWindowText(label), editor);
    }
    layout->addLayout(form);

    auto *buttonLayout = new QGridLayout;
    buttonLayout->setHorizontalSpacing(6);
    buttonLayout->setVerticalSpacing(6);
    const int buttonColumns = title == QStringLiteral("DTU网络") ? 1 : (buttons.size() > 1 ? 2 : 1);
    for (int index = 0; index < buttons.size(); ++index) {
        const QString &text = buttons.at(index);
        auto *button = labviewButton(translatedMainWindowText(text));
        setActionButtonRole(button, text);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setObjectName(QStringLiteral("action:%1:%2").arg(title, text));
        connect(button, &QPushButton::clicked, this, [this, title, text] {
            handleParameterAction(title, text);
        });
        const bool spanLastRow = buttonColumns == 2 && index == buttons.size() - 1 && buttons.size() % 2 != 0;
        buttonLayout->addWidget(button, index / buttonColumns, index % buttonColumns, 1, spanLastRow ? 2 : 1);
    }
    layout->addLayout(buttonLayout);
    layout->addStretch(1);
    return box;
}

QLabel *MainWindow::sectionTitle(const QString &text) const
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("sectionTitle"));
    label->setMinimumHeight(26);
    return label;
}

QComboBox *MainWindow::combo(const QStringList &items) const
{
    auto *box = new QComboBox;
    box->addItems(items);
    return box;
}

QComboBox *MainWindow::onOffCombo() const
{
    auto *box = new QComboBox;
    box->addItem(tr("关"), QStringLiteral("关"));
    box->addItem(tr("开"), QStringLiteral("开"));
    return box;
}

QLineEdit *MainWindow::lineEdit(const QString &text) const
{
    auto *edit = new QLineEdit;
    edit->setText(text);
    return edit;
}

QSpinBox *MainWindow::byteSpinBox(int value) const
{
    auto *box = new QSpinBox;
    box->setRange(0, 255);
    box->setValue(value);
    return box;
}

QPushButton *MainWindow::labviewButton(const QString &text) const
{
    auto *button = new QPushButton(text);
    button->setMinimumWidth(72);
    return button;
}

void MainWindow::configureTable(QTableView *table) const
{
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setMinimumSectionSize(24);
    table->verticalHeader()->setDefaultSectionSize(28);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->horizontalHeader()->setMinimumSectionSize(18);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setDefaultSectionSize(96);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->setShowGrid(true);
    table->setWordWrap(false);
}

void MainWindow::loadPersistedLabviewData()
{
    const QString sqlitePath = gucds::LabviewDatabase::defaultDatabasePath();
    QString errorMessage;

    if (!QFileInfo::exists(sqlitePath)) {
        const QString labviewRoot = findLabviewProjectRoot();
        if (labviewRoot.isEmpty()) {
            appendStatus(tr("未找到 LabVIEW 数据目录，无法生成 SQLite：General Upper Computer Debugging Software5.5"));
            return;
        }
        if (!gucds::LabviewDatabase::importFromLabviewProject(labviewRoot, sqlitePath, &errorMessage)) {
            appendStatus(tr("LabVIEW 数据导入 SQLite 失败：%1").arg(errorMessage));
            return;
        }
    }

    reloadDeviceLibrary();

    const QVector<gucds::CalibrationRecord> calibrations = gucds::LabviewDatabase::loadCalibrationRecords(sqlitePath, &errorMessage);
    if (!errorMessage.isEmpty()) {
        appendStatus(tr("读取 SQLite 标定曲线失败：%1").arg(errorMessage));
        errorMessage.clear();
    } else {
        m_calibrationModel->setRecords(calibrations);
    }

    const QVector<gucds::BusDeviceRecord> busDevices = gucds::LabviewDatabase::loadBusDeviceRecords(sqlitePath, &errorMessage);
    if (!errorMessage.isEmpty()) {
        appendStatus(tr("读取 SQLite 总线设备失败：%1").arg(errorMessage));
    } else {
        m_busDeviceModel->setRecords(busDevices);
    }
}

void MainWindow::reloadDeviceLibrary()
{
    QString errorMessage;
    const QVector<gucds::DeviceRecord> devices = gucds::LabviewDatabase::loadDeviceRecords(
        gucds::LabviewDatabase::defaultDatabasePath(),
        &errorMessage);
    if (!errorMessage.isEmpty()) {
        appendStatus(tr("读取 SQLite 设备库失败：%1").arg(errorMessage));
        return;
    }

    if (m_activeLibraryIndex >= 0)
        clearDeviceEditor();
    populateDeviceLibraryTree(devices);
}

void MainWindow::openProductManagement()
{
    if (!ProductManagementDialog::requestAuthorization(this))
        return;

    ProductManagementDialog dialog(gucds::LabviewDatabase::defaultDatabasePath(), this);
    dialog.exec();
    if (!dialog.catalogChanged())
        return;

    reloadDeviceLibrary();
    appendStatus(tr("产品库已更新，设备库候选列表已刷新"));
}

void MainWindow::openFrequencyTensionParameters()
{
    FrequencyTensionParameterDialog dialog(
        gucds::LabviewDatabase::defaultDatabasePath(), this);
    dialog.setSensorWriteAvailable(true);
    connect(&dialog,
            &FrequencyTensionParameterDialog::sensorWriteRequested,
            this,
            [this, &dialog](const gucds::FrequencyTensionParameterRecord &record) {
                if (!configureCommunication()) {
                    dialog.finishSensorWrite(false, tr("串口配置无效，参数仅保存在本机。"), {});
                    return;
                }
                if (m_communicationController->isRunning()) {
                    dialog.finishSensorWrite(false, tr("已有通信任务正在运行，请稍后重试。"), {});
                    return;
                }

                const QStringList arguments = {
                    QString::number(record.supportFactor, 'g', 12),
                    QString::number(record.unitMass, 'g', 12),
                    QString::number(record.cableLength, 'g', 12),
                    QString::number(record.area, 'g', 12),
                    QString::number(record.elasticModulus, 'g', 12),
                    QString::number(record.inertia, 'g', 12),
                    QString::number(record.angle, 'g', 12),
                };
                const QString setCommand = gucds::AtProtocol::buildSetExtendedParameters(arguments);
                if (!m_communicationController->sendTextCommands(
                        {setCommand,
                         gucds::AtProtocol::command(gucds::AtCommand::GetFrequencyParameters)},
                        QStringLiteral("frequency_parameters_write"),
                        200)) {
                    dialog.finishSensorWrite(false, tr("无法启动扩展参数写入任务。"), {});
                }
            });
    connect(m_communicationController,
            &gucds::DeviceCommunicationController::resultReady,
            &dialog,
            [&dialog](const gucds::CommunicationResult &result) {
                if (result.context == QStringLiteral("frequency_parameters_write"))
                    dialog.finishSensorWrite(result.success, result.message, result.numericValues);
            });
    dialog.exec();
}

void MainWindow::populateDeviceLibraryTree(const QVector<gucds::DeviceRecord> &records)
{
    if (!m_deviceLibraryTree)
        return;

    m_deviceLibraryRecords = records;
    m_deviceLibraryTree->clear();
    QMap<QString, QVector<int>> groupedDevices;
    QSet<QString> seenDevices;
    for (int index = 0; index < records.size(); ++index) {
        const gucds::DeviceRecord &record = records.at(index);
        const QString identity = deviceIdentityKey(record);
        if (seenDevices.contains(identity))
            continue;
        seenDevices.insert(identity);
        groupedDevices[deviceCategory(record)].append(index);
    }

    for (auto it = groupedDevices.cbegin(); it != groupedDevices.cend(); ++it) {
        auto *root = new QTreeWidgetItem(m_deviceLibraryTree, QStringList{it.key()});
        for (int recordIndex : it.value()) {
            const gucds::DeviceRecord &record = records.at(recordIndex);
            auto *child = new QTreeWidgetItem(root, QStringList{deviceDisplayName(record)});
            child->setData(0, kDeviceLibraryRecordRole, recordIndex);
            child->setToolTip(0,
                              QStringLiteral("%1\n%2\n%3")
                                  .arg(record.name.trimmed(), deviceCategory(record), record.model.trimmed()));
        }
        root->setExpanded(true);
    }
}

void MainWindow::showLibraryDeviceDetails(int libraryIndex)
{
    if (libraryIndex < 0 || libraryIndex >= m_deviceLibraryRecords.size())
        return;

    m_activeLibraryIndex = libraryIndex;
    m_activeConfiguredRow = -1;
    populateDeviceEditor(m_deviceLibraryRecords.at(libraryIndex), tr("设备库候选"));
    if (m_mainTabs)
        m_mainTabs->setCurrentIndex(1);
}

void MainWindow::showConfiguredDeviceDetails(int row)
{
    if (row < 0 || row >= m_deviceModel->rowCount())
        return;

    m_activeLibraryIndex = -1;
    m_activeConfiguredRow = row;
    populateDeviceEditor(m_deviceModel->recordAt(row), tr("配置表第 %1 行").arg(row + 1));
    if (m_mainTabs)
        m_mainTabs->setCurrentIndex(1);
}

void MainWindow::populateDeviceEditor(const gucds::DeviceRecord &record, const QString &sourceText)
{
    m_activeDeviceRecord = record;
    m_hasActiveDeviceRecord = true;

    if (m_deviceEditorSourceLabel)
        m_deviceEditorSourceLabel->setText(tr("%1：%2 / %3")
                                               .arg(sourceText, deviceCategory(record), deviceDisplayName(record)));
    if (m_deviceNameEditor)
        m_deviceNameEditor->setText(record.name);
    if (m_deviceCategoryEditor)
        m_deviceCategoryEditor->setText(record.category);
    if (m_deviceModelEditor)
        m_deviceModelEditor->setText(record.model);

    const QStringList dataNames = {record.data1, record.data2, record.data3, record.data4, record.data5};
    for (int index = 0; index < m_deviceDataEditors.size(); ++index)
        m_deviceDataEditors.at(index)->setText(dataNames.value(index));

    auto setComboText = [](QComboBox *comboBox, const QString &value) {
        if (!comboBox)
            return;
        const QString normalized = value.trimmed().isEmpty() ? QStringLiteral("关") : value.trimmed();
        const int index = comboBox->findData(normalized);
        comboBox->setCurrentIndex(index >= 0 ? index : 0);
    };
    setComboText(m_deviceModbusEditor, record.modbus);
    setComboText(m_deviceLoraEditor, record.lora);
    setComboText(m_deviceDtuEditor, record.dtu);
    if (m_deviceCalibrationPointsEditor)
        m_deviceCalibrationPointsEditor->setValue(record.calibrationPoints);

    rebuildDeviceParameterEditors(record);
    updateTestDataModeForDevice(record);
    if (m_addEditedDeviceButton)
        m_addEditedDeviceButton->setEnabled(true);
    if (m_applyEditedDeviceButton)
        m_applyEditedDeviceButton->setEnabled(m_activeConfiguredRow >= 0);
}

void MainWindow::clearDeviceEditor()
{
    m_hasActiveDeviceRecord = false;
    m_activeLibraryIndex = -1;
    m_activeConfiguredRow = -1;

    if (m_deviceEditorSourceLabel)
        m_deviceEditorSourceLabel->setText(tr("从左侧设备库选择一个候选设备"));
    const QList<QLineEdit *> edits = {m_deviceNameEditor, m_deviceCategoryEditor, m_deviceModelEditor};
    for (QLineEdit *edit : edits) {
        if (edit)
            edit->clear();
    }
    for (QLineEdit *edit : m_deviceDataEditors) {
        if (edit)
            edit->clear();
    }
    if (m_deviceModbusEditor)
        m_deviceModbusEditor->setCurrentIndex(0);
    if (m_deviceLoraEditor)
        m_deviceLoraEditor->setCurrentIndex(0);
    if (m_deviceDtuEditor)
        m_deviceDtuEditor->setCurrentIndex(0);
    if (m_deviceCalibrationPointsEditor)
        m_deviceCalibrationPointsEditor->setValue(0);
    for (const QString &label : {QStringLiteral("采样频率_FVCF"), QStringLiteral("采样点数_FVCF")}) {
        if (QWidget *editor = parameterEditor(QStringLiteral("MCU卡"), label))
            editor->setEnabled(false);
    }
    m_pendingSamplingRateIndex = -1;
    m_pendingSamplingPointIndex = -1;
    rebuildDeviceParameterEditors({});
    m_measurementModel->setMode(gucds::MeasurementTableModel::Mode::Generic);
    if (m_measurementSectionTitle)
        m_measurementSectionTitle->setText(tr("测量数据"));
    if (m_plot)
        m_plot->setValues({});
    if (m_addEditedDeviceButton)
        m_addEditedDeviceButton->setEnabled(false);
    if (m_applyEditedDeviceButton)
        m_applyEditedDeviceButton->setEnabled(false);
}

gucds::DeviceRecord MainWindow::editedDeviceRecord() const
{
    gucds::DeviceRecord record = m_activeDeviceRecord;
    if (m_deviceNameEditor)
        record.name = m_deviceNameEditor->text().trimmed();
    if (m_deviceCategoryEditor)
        record.category = m_deviceCategoryEditor->text().trimmed();
    if (m_deviceModelEditor)
        record.model = m_deviceModelEditor->text().trimmed();
    if (m_deviceDataEditors.size() > 0)
        record.data1 = m_deviceDataEditors.at(0)->text().trimmed();
    if (m_deviceDataEditors.size() > 1)
        record.data2 = m_deviceDataEditors.at(1)->text().trimmed();
    if (m_deviceDataEditors.size() > 2)
        record.data3 = m_deviceDataEditors.at(2)->text().trimmed();
    if (m_deviceDataEditors.size() > 3)
        record.data4 = m_deviceDataEditors.at(3)->text().trimmed();
    if (m_deviceDataEditors.size() > 4)
        record.data5 = m_deviceDataEditors.at(4)->text().trimmed();
    if (m_deviceModbusEditor)
        record.modbus = m_deviceModbusEditor->currentData().toString();
    if (m_deviceLoraEditor)
        record.lora = m_deviceLoraEditor->currentData().toString();
    if (m_deviceDtuEditor)
        record.dtu = m_deviceDtuEditor->currentData().toString();
    if (m_deviceCalibrationPointsEditor)
        record.calibrationPoints = m_deviceCalibrationPointsEditor->value();

    for (QWidget *editor : m_deviceParameterEditors) {
        gucds::setDeviceParameterValue(&record,
                                       editor->property("parameterIndex").toInt(),
                                       parameterValueFromEditor(editor));
    }

    return record;
}

void MainWindow::rebuildDeviceParameterEditors(const gucds::DeviceRecord &record)
{
    if (!m_deviceParameterForm)
        return;

    while (m_deviceParameterForm->count() > 0) {
        QLayoutItem *item = m_deviceParameterForm->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    m_deviceParameterEditors.clear();

    const bool frequencyTension = isFrequencyTensionDevice(record);
    bool movedSamplingParameters = false;
    for (int index = 1; index <= 5; ++index) {
        const QString spec = gucds::deviceParameterDefinition(record, index);
        const gucds::DeviceParameterDefinition definition = gucds::parseDeviceParameterDefinition(spec);
        if (frequencyTension
            && (definition.name == QStringLiteral("采样频率")
                || definition.name == QStringLiteral("采样点数"))) {
            movedSamplingParameters = true;
            continue;
        }
        QWidget *editor = editorForParameterSpec(spec);
        if (!editor)
            continue;
        editor->setProperty("parameterIndex", index);
        const QString configuredValue = gucds::deviceParameterValue(record, index);
        if (!configuredValue.isEmpty()) {
            if (auto *comboBox = qobject_cast<QComboBox *>(editor)) {
                const int valueIndex = comboBox->findText(configuredValue);
                if (valueIndex >= 0)
                    comboBox->setCurrentIndex(valueIndex);
            } else if (auto *line = qobject_cast<QLineEdit *>(editor)) {
                line->setText(configuredValue);
            }
        }
        m_deviceParameterEditors.append(editor);
        m_deviceParameterForm->addRow(parameterDefinitionSummary(spec), editor);
    }

    if (m_deviceParameterEditors.isEmpty()) {
        m_deviceParameterForm->addRow(
            tr("参数"),
            new QLabel(movedSamplingParameters
                           ? tr("采样频率和采样点数请在 MCU 模块中读取和配置。")
                           : tr("该设备没有可配置传感器参数")));
    }
}

QWidget *MainWindow::editorForParameterSpec(const QString &spec) const
{
    const gucds::DeviceParameterDefinition definition = gucds::parseDeviceParameterDefinition(spec);
    if (!definition.isValid())
        return nullptr;

    QWidget *editor = nullptr;
    if (definition.editorMode == QStringLiteral("菜单")) {
        auto *box = combo(definition.options());
        editor = box;
    } else {
        editor = lineEdit(definition.valueText);
    }
    return editor;
}

QString MainWindow::parameterValueFromEditor(QWidget *editor) const
{
    if (!editor)
        return tr("未定义");

    QString value;
    if (auto *comboBox = qobject_cast<QComboBox *>(editor)) {
        value = comboBox->currentText();
    } else if (auto *line = qobject_cast<QLineEdit *>(editor)) {
        value = line->text().trimmed();
    }
    return value;
}

QString MainWindow::parameterDefinitionSummary(const QString &spec) const
{
    const gucds::DeviceParameterDefinition definition = gucds::parseDeviceParameterDefinition(spec);
    if (!definition.isValid())
        return {};
    return tr("%1（%2/%3）")
        .arg(definition.name,
             translatedMainWindowText(definition.editorMode),
             translatedMainWindowText(definition.valueType));
}

void MainWindow::addSelectedLibraryDeviceToConfig()
{
    if (!m_deviceLibraryTree)
        return;

    QTreeWidgetItem *item = m_deviceLibraryTree->currentItem();
    if (!item || !item->data(0, kDeviceLibraryRecordRole).isValid()) {
        appendStatus(tr("请先在设备库选择一个设备"));
        return;
    }

    const int libraryIndex = item->data(0, kDeviceLibraryRecordRole).toInt();
    if (libraryIndex != m_activeLibraryIndex)
        showLibraryDeviceDetails(libraryIndex);
    addEditedDeviceToConfig();
}

void MainWindow::addLibraryDeviceToConfig(int libraryIndex)
{
    if (libraryIndex < 0 || libraryIndex >= m_deviceLibraryRecords.size())
        return;

    const gucds::DeviceRecord record = m_deviceLibraryRecords.at(libraryIndex);
    if (m_deviceModel->containsEquivalent(record)) {
        if (m_deviceConfigTable) {
            for (int row = 0; row < m_deviceModel->rowCount(); ++row) {
                if (deviceIdentityKey(m_deviceModel->recordAt(row)) == deviceIdentityKey(record)) {
                    m_deviceConfigTable->selectRow(row);
                    break;
                }
            }
        }
        return;
    }

    m_deviceModel->addRecord(record);
    if (m_deviceConfigTable) {
        const int row = m_deviceModel->rowCount() - 1;
        m_deviceConfigTable->selectRow(row);
        m_deviceConfigTable->scrollTo(m_deviceModel->index(row, 0));
    }
}

void MainWindow::addEditedDeviceToConfig()
{
    if (!m_hasActiveDeviceRecord) {
        appendStatus(tr("请先在设备库或配置表选择一个设备"));
        return;
    }

    const gucds::DeviceRecord record = editedDeviceRecord();
    if (record.name.trimmed().isEmpty() && record.model.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("设备参数"), tr("设备名称或规格型号不能为空。"));
        return;
    }

    for (int row = 0; row < m_deviceModel->rowCount(); ++row) {
        if (deviceIdentityKey(m_deviceModel->recordAt(row)) == deviceIdentityKey(record)) {
            if (m_deviceConfigTable)
                m_deviceConfigTable->selectRow(row);
            const QMessageBox::StandardButton answer = QMessageBox::question(
                this,
                tr("设备已存在"),
                tr("配置表中已有该设备。是否用右侧表单内容更新该行？"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
            if (answer == QMessageBox::Yes) {
                m_deviceModel->setRecordAt(row, record);
                showConfiguredDeviceDetails(row);
            }
            return;
        }
    }

    m_deviceModel->addRecord(record);
    if (m_deviceConfigTable) {
        const int row = m_deviceModel->rowCount() - 1;
        m_deviceConfigTable->selectRow(row);
        m_deviceConfigTable->scrollTo(m_deviceModel->index(row, 0));
    }
    appendStatus(tr("已添加设备：%1").arg(deviceDisplayName(record)));
}

void MainWindow::applyEditedDeviceToConfig()
{
    const int row = selectedConfiguredDeviceRow();
    if (!m_hasActiveDeviceRecord || row < 0) {
        appendStatus(tr("请先在设备配置表选择要修改的行"));
        return;
    }

    const gucds::DeviceRecord record = editedDeviceRecord();
    if (!m_deviceModel->setRecordAt(row, record))
        return;
    if (m_deviceConfigTable)
        m_deviceConfigTable->selectRow(row);
    showConfiguredDeviceDetails(row);
    appendStatus(tr("已更新配置表第 %1 行").arg(row + 1));
}

void MainWindow::removeSelectedConfiguredDevice()
{
    const int row = selectedConfiguredDeviceRow();
    if (row < 0) {
        appendStatus(tr("请先在设备配置表选择一个设备"));
        return;
    }

    const gucds::DeviceRecord record = m_deviceModel->recordAt(row);
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("删除设备"),
        tr("从配置表删除“%1”？").arg(deviceDisplayName(record)),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    if (!m_deviceModel->removeRecordAt(row))
        return;

    if (m_deviceConfigTable && m_deviceModel->rowCount() > 0)
        m_deviceConfigTable->selectRow((std::min)(row, m_deviceModel->rowCount() - 1));
    else
        clearDeviceEditor();
}

void MainWindow::clearConfiguredDevices()
{
    if (m_deviceModel->rowCount() <= 0)
        return;

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("清空配置表"),
        tr("确认清空当前设备配置表？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    m_deviceModel->clear();
    clearDeviceEditor();
}

void MainWindow::moveSelectedConfiguredDevice(int offset)
{
    const int row = selectedConfiguredDeviceRow();
    const int targetRow = row + offset;
    if (row < 0 || !m_deviceModel->moveRecord(row, targetRow))
        return;

    if (m_deviceConfigTable)
        m_deviceConfigTable->selectRow(targetRow);
}

int MainWindow::selectedConfiguredDeviceRow() const
{
    if (!m_deviceConfigTable)
        return -1;

    const QModelIndex current = m_deviceConfigTable->currentIndex();
    if (current.isValid())
        return current.row();

    const QModelIndexList selectedRows = m_deviceConfigTable->selectionModel()
        ? m_deviceConfigTable->selectionModel()->selectedRows()
        : QModelIndexList{};
    return selectedRows.isEmpty() ? -1 : selectedRows.first().row();
}

void MainWindow::appendStatus(const QString &message)
{
    const QString stampedMessage = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss ")) + message;
    statusBar()->showMessage(stampedMessage, 8000);
}

void MainWindow::scanDevices()
{
    if (!configureCommunication())
        return;
    if (!m_communicationController->scanDevices())
        appendStatus(tr("已有通信任务正在运行，请稍后重试"));
}

void MainWindow::startMeasurement()
{
    if (!configureCommunication())
        return;
    if (m_communicationController->isRunning()) {
        appendStatus(tr("通信任务正在进行，请等待完成或先取消"));
        return;
    }
    const bool frequencyTension = activeDeviceIsFrequencyTension();
    m_frequencyTensionMeasurementActive = frequencyTension;
    m_measurementDeviceRecord = m_activeDeviceRecord;
    if (frequencyTension)
        showFrequencyTensionMeasurements();
    const QString context = frequencyTension
        ? QStringLiteral("frequency_tension_measurement")
        : QStringLiteral("measurement");
    const bool started = frequencyTension
        ? m_communicationController->measureFrequencyTension(context)
        : m_communicationController->measure(context);
    if (!started)
    {
        m_frequencyTensionMeasurementActive = false;
        m_measurementDeviceRecord = {};
        appendStatus(tr("无法启动精确测量任务"));
    }
}

void MainWindow::recordMeasurement(const gucds::SensorModbusSample &sample, const QString &trace)
{
    gucds::MeasurementRecord record;
    record.index = m_measurementModel->rowCount() + 1;
    record.deviceName = tr("F405倾角传感器");
    record.timestamp = QDateTime::currentDateTime();
    record.value = sample.pitch;
    record.state = QString::number(sample.temperature, 'f', 3);
    record.message = QString::number(sample.roll, 'f', 3);
    m_measurementModel->addMeasurement(record);

    if (m_plot)
        m_plot->appendValue(record.value);
    const QString source = tr("串口 %1/%2")
                               .arg(m_communicationController->portName())
                               .arg(m_communicationController->baudRate());
    appendStatus(tr("%1 测量完成：Pitch=%2 Roll=%3 Error=%4 Temp=%5")
                     .arg(source)
                     .arg(double(sample.pitch), 0, 'f', 3)
                     .arg(double(sample.roll), 0, 'f', 3)
                     .arg(double(sample.error), 0, 'f', 3)
                     .arg(double(sample.temperature), 0, 'f', 3));
    Q_UNUSED(trace)
}

void MainWindow::recordFrequencyTensionMeasurement(const gucds::FrequencyTensionSample &sample)
{
    gucds::FrequencyTensionMeasurementRecord record;
    record.index = m_measurementModel->frequencyTensionMeasurements().size() + 1;
    const gucds::DeviceRecord &measurementDevice =
        m_measurementDeviceRecord.name.isEmpty() && m_measurementDeviceRecord.model.isEmpty()
            ? m_activeDeviceRecord
            : m_measurementDeviceRecord;
    record.deviceName = measurementDevice.name.trimmed();
    if (record.deviceName.isEmpty())
        record.deviceName = deviceDisplayName(measurementDevice);
    record.timestamp = QDateTime::currentDateTime();
    record.cableForceKn = sample.cableForceKn;
    record.naturalFrequencyHz = sample.naturalFrequencyHz;
    record.order = sample.order;
    record.convergenceErrorPercent = sample.convergenceErrorPercent;
    m_measurementModel->addFrequencyTensionMeasurement(record);
    showFrequencyTensionMeasurements();

    if (m_latestDataList) {
        m_latestDataList->insertItem(0, QString::number(record.cableForceKn, 'f', 3));
        while (m_latestDataList->count() > 100)
            delete m_latestDataList->takeItem(m_latestDataList->count() - 1);
    }
    if (m_latestBand) {
        m_latestBand->setText(tr("索力 %1 kN，fn %2 Hz")
                                  .arg(record.cableForceKn, 0, 'f', 3)
                                  .arg(record.naturalFrequencyHz, 0, 'f', 3));
    }
    if (m_latestCount)
        m_latestCount->setText(QString::number(m_measurementModel->frequencyTensionMeasurements().size()));
}

void MainWindow::readSpectrum()
{
    if (!activeDeviceIsFrequencyTension()) {
        appendStatus(tr("请先选择频振索力传感器，再读取频谱。"));
        return;
    }
    m_measurementModel->setSpectrumPoints({});
    if (m_measurementSectionTitle)
        m_measurementSectionTitle->setText(tr("频谱数据"));
    if (m_plot)
        m_plot->setSpectrumPoints({});
    sendDeviceCommand(gucds::AtProtocol::command(gucds::AtCommand::GetSpectrum),
                      QStringLiteral("spectrum"));
}

bool MainWindow::activeDeviceIsFrequencyTension() const
{
    return m_hasActiveDeviceRecord && isFrequencyTensionDevice(m_activeDeviceRecord);
}

void MainWindow::updateTestDataModeForDevice(const gucds::DeviceRecord &record)
{
    const bool frequencyTension = isFrequencyTensionDevice(record);
    for (const QString &label : {QStringLiteral("采样频率_FVCF"), QStringLiteral("采样点数_FVCF")}) {
        if (QWidget *editor = parameterEditor(QStringLiteral("MCU卡"), label))
            editor->setEnabled(frequencyTension);
    }
    if (frequencyTension) {
        QString configuredRate;
        QString configuredPoints;
        for (int index = 1; index <= 5; ++index) {
            const gucds::DeviceParameterDefinition definition =
                gucds::parseDeviceParameterDefinition(gucds::deviceParameterDefinition(record, index));
            if (definition.name == QStringLiteral("采样频率"))
                configuredRate = gucds::deviceParameterValue(record, index);
            else if (definition.name == QStringLiteral("采样点数"))
                configuredPoints = gucds::deviceParameterValue(record, index);
        }
        if (!configuredRate.isEmpty())
            setParameterText(QStringLiteral("MCU卡"), QStringLiteral("采样频率_FVCF"), configuredRate);
        if (!configuredPoints.isEmpty())
            setParameterText(QStringLiteral("MCU卡"), QStringLiteral("采样点数_FVCF"), configuredPoints);
        showFrequencyTensionMeasurements();
        return;
    }
    m_measurementModel->setMode(gucds::MeasurementTableModel::Mode::Generic);
    configureMeasurementTableColumns();
    if (m_measurementSectionTitle)
        m_measurementSectionTitle->setText(tr("测量数据"));
    if (m_plot)
        m_plot->setValues({});
}

void MainWindow::configureMeasurementTableColumns()
{
    if (!m_measurementTable)
        return;

    QHeaderView *header = m_measurementTable->horizontalHeader();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(QHeaderView::Interactive);

    const auto headerWidth = [this](int column, int minimum) {
        const QString text = m_measurementModel->headerData(column, Qt::Horizontal).toString();
        return (std::max)(minimum, m_measurementTable->fontMetrics().horizontalAdvance(text) + 28);
    };

    if (m_measurementModel->mode() == gucds::MeasurementTableModel::Mode::FrequencyTension) {
        const QList<int> minimumWidths = {52, 96, 94, 76, 82, 130, 160};
        for (int column = 0; column < minimumWidths.size(); ++column)
            m_measurementTable->setColumnWidth(column, headerWidth(column, minimumWidths.at(column)));
        return;
    }

    if (m_measurementModel->mode() == gucds::MeasurementTableModel::Mode::Spectrum) {
        m_measurementTable->setColumnWidth(0, headerWidth(0, 72));
        m_measurementTable->setColumnWidth(1, headerWidth(1, 120));
        header->setSectionResizeMode(2, QHeaderView::Stretch);
    }
}

void MainWindow::showFrequencyTensionMeasurements()
{
    m_measurementModel->setMode(gucds::MeasurementTableModel::Mode::FrequencyTension);
    configureMeasurementTableColumns();
    if (m_measurementSectionTitle)
        m_measurementSectionTitle->setText(tr("索力测量数据"));
    if (!m_plot)
        return;

    QVector<QPointF> points;
    const auto &records = m_measurementModel->frequencyTensionMeasurements();
    points.reserve(records.size());
    for (const gucds::FrequencyTensionMeasurementRecord &record : records)
        points.append(QPointF(record.index, record.cableForceKn));
    m_plot->setFrequencyTensionPoints(points);
}

void MainWindow::deleteSelectedMeasurements()
{
    if (!m_measurementTable || !m_measurementTable->selectionModel())
        return;
    QModelIndexList rows = m_measurementTable->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        QMessageBox::information(this, tr("删除测量数据"), tr("请先选择要删除的测量记录。"));
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        tr("删除测量数据"),
        tr("确定删除选中的 %1 条测量记录吗？").arg(rows.size()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    std::sort(rows.begin(), rows.end(), [](const QModelIndex &left, const QModelIndex &right) {
        return left.row() > right.row();
    });
    for (const QModelIndex &index : std::as_const(rows))
        m_measurementModel->removeRow(index.row());

    refreshMeasurementPresentation();
    updateMeasurementActions();
    appendStatus(tr("已删除 %1 条测量记录").arg(rows.size()));
}

void MainWindow::saveMeasurementsAsCsv()
{
    if (m_measurementModel->rowCount() == 0) {
        QMessageBox::information(this, tr("保存测量数据"), tr("当前没有可保存的测量数据。"));
        return;
    }

    const QString suggestedName = QDir::home().filePath(
        QStringLiteral("QL-IOT-measurements-%1.csv")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"))));
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("保存测量数据"),
        suggestedName,
        tr("CSV 文件 (*.csv)"));
    if (filePath.isEmpty())
        return;
    if (QFileInfo(filePath).suffix().isEmpty())
        filePath.append(QStringLiteral(".csv"));

    QString errorMessage;
    if (!m_measurementModel->saveCsv(filePath, &errorMessage)) {
        QMessageBox::critical(this, tr("保存测量数据失败"), errorMessage);
        return;
    }
    appendStatus(tr("测量数据已保存：%1").arg(QDir::toNativeSeparators(filePath)));
}

void MainWindow::refreshMeasurementPresentation()
{
    if (m_measurementModel->mode() == gucds::MeasurementTableModel::Mode::FrequencyTension) {
        showFrequencyTensionMeasurements();
        const auto &records = m_measurementModel->frequencyTensionMeasurements();
        if (m_latestDataList) {
            m_latestDataList->clear();
            const int first = (std::max)(0, int(records.size()) - 100);
            for (int index = int(records.size()) - 1; index >= first; --index)
                m_latestDataList->addItem(QString::number(records.at(index).cableForceKn, 'f', 3));
        }
        if (m_latestBand) {
            if (records.isEmpty()) {
                m_latestBand->setText(tr("等待测量数据"));
            } else {
                const auto &record = records.constLast();
                m_latestBand->setText(tr("索力 %1 kN，fn %2 Hz")
                                          .arg(record.cableForceKn, 0, 'f', 3)
                                          .arg(record.naturalFrequencyHz, 0, 'f', 3));
            }
        }
        if (m_latestCount)
            m_latestCount->setText(QString::number(records.size()));
        return;
    }

    if (m_measurementModel->mode() == gucds::MeasurementTableModel::Mode::Spectrum) {
        QVector<QPointF> points;
        const auto &samples = m_measurementModel->spectrumPoints();
        points.reserve(samples.size());
        for (const gucds::SpectrumPoint &sample : samples)
            points.append(QPointF(sample.frequencyHz, sample.amplitude));
        if (m_plot)
            m_plot->setSpectrumPoints(points);
        if (m_latestCount)
            m_latestCount->setText(QString::number(samples.size()));
        return;
    }

    QVector<double> values;
    const auto &records = m_measurementModel->measurements();
    values.reserve(records.size());
    for (const gucds::MeasurementRecord &record : records)
        values.append(record.value);
    if (m_plot)
        m_plot->setValues(values);
    if (m_latestCount)
        m_latestCount->setText(QString::number(records.size()));
}

void MainWindow::updateMeasurementActions()
{
    const bool hasRows = m_measurementModel && m_measurementModel->rowCount() > 0;
    const bool hasSelection = hasRows && m_measurementTable && m_measurementTable->selectionModel()
        && !m_measurementTable->selectionModel()->selectedRows().isEmpty();
    if (m_deleteMeasurementAction)
        m_deleteMeasurementAction->setEnabled(hasSelection);
    if (m_saveMeasurementsAction)
        m_saveMeasurementsAction->setEnabled(hasRows);
    if (m_deleteMeasurementButton)
        m_deleteMeasurementButton->setEnabled(hasSelection);
    if (m_saveMeasurementsButton)
        m_saveMeasurementsButton->setEnabled(hasRows);
}

void MainWindow::showSpectrum(const QVector<gucds::SpectrumSample> &samples)
{
    QVector<gucds::SpectrumPoint> tablePoints;
    QVector<QPointF> plotPoints;
    tablePoints.reserve(samples.size());
    plotPoints.reserve(samples.size());
    for (qsizetype index = 0; index < samples.size(); ++index) {
        const gucds::SpectrumSample &sample = samples.at(index);
        tablePoints.append({int(index + 1), sample.frequencyHz, sample.amplitude});
        plotPoints.append(QPointF(sample.frequencyHz, sample.amplitude));
    }
    m_measurementModel->setSpectrumPoints(tablePoints);
    configureMeasurementTableColumns();
    if (m_measurementSectionTitle)
        m_measurementSectionTitle->setText(tr("频谱数据"));
    if (m_plot)
        m_plot->setSpectrumPoints(plotPoints);

    if (!samples.isEmpty() && m_latestBand) {
        const auto peak = std::max_element(samples.cbegin(), samples.cend(), [](const auto &left, const auto &right) {
            return left.amplitude < right.amplitude;
        });
        m_latestBand->setText(tr("峰值 %1 Hz / %2")
                                  .arg(peak->frequencyHz, 0, 'f', 3)
                                  .arg(peak->amplitude, 0, 'f', 6));
    }
    if (m_latestCount)
        m_latestCount->setText(QString::number(samples.size()));
}

void MainWindow::sendDeviceCommand(const QString &request, const QString &context, int repeatCount)
{
    if (!configureCommunication())
        return;
    if (!m_communicationController->sendTextCommand(request, context, repeatCount))
        appendStatus(tr("命令无效或已有通信任务正在运行"));
}

void MainWindow::sendModbusCommand(gucds::SensorModbusCommand command, const QString &context)
{
    if (!configureCommunication())
        return;
    if (!m_communicationController->sendModbusCommand(command, context))
        appendStatus(tr("已有通信任务正在运行，请稍后重试"));
}

void MainWindow::writeModbusParameters(quint16 startAddress,
                                       const QByteArray &bytes,
                                       gucds::SensorModbusCommand saveCommand,
                                       const QString &context)
{
    if (!configureCommunication())
        return;
    if (!m_communicationController->writeModbusRegisters(startAddress, bytes, context, saveCommand))
        appendStatus(tr("参数帧无效或已有通信任务正在运行"));
}

void MainWindow::openDtuConfiguration(const QString &title, const QString &context)
{
    Q_UNUSED(title)
    if (!m_dtuConfigDialog)
        m_dtuConfigDialog = new DtuConfigDialog(this);
    if (m_dtuConfigDialog->exec() != QDialog::Accepted)
        return;

    const QString command = m_dtuConfigDialog->configCommand();
    if (command.isEmpty()) {
        QMessageBox::warning(this, tr("DTU 参数"), tr("DTU 配置命令无效。"));
        return;
    }

    const QString summary = m_dtuConfigDialog->configurationSummary();
    setParameterText(QStringLiteral("DTU主卡"), QStringLiteral("配置摘要"), summary);
    setParameterText(QStringLiteral("DTU网络"), QStringLiteral("配置摘要"), summary);

    if (!configureCommunication())
        return;
    if (!m_communicationController->sendTextCommands(
            {command, gucds::AtProtocol::command(gucds::AtCommand::ConfigSave)},
            context,
            3000)) {
        appendStatus(tr("已有通信任务正在运行，请稍后重试"));
    }
}

void MainWindow::testDtuNetwork(const QString &context)
{
    if (!configureCommunication())
        return;
    if (!m_communicationController->sendTextCommands(
            {QStringLiteral("config,get,ssta"),
             QStringLiteral("config,get,netstatus,1"),
             QStringLiteral("config,get,netchaninfo,1")},
            context,
            1500)) {
        appendStatus(tr("已有通信任务正在运行，请稍后重试"));
    }
}

bool MainWindow::configureCommunication()
{
    if (!m_serialPort || !m_serialBaud || !m_serialSlaveId || !m_serialProtocol)
        return false;
    bool baudOk = false;
    const int baud = m_serialBaud->currentText().toInt(&baudOk);
    if (m_serialPort->currentText().trimmed().isEmpty() || !baudOk || baud <= 0) {
        QMessageBox::warning(this, tr("串口配置"), tr("请选择有效串口并填写正确波特率。"));
        return false;
    }
    const gucds::DeviceWireProtocol protocol = m_serialProtocol->currentText() == QStringLiteral("Modbus")
        ? gucds::DeviceWireProtocol::Modbus
        : gucds::DeviceWireProtocol::Text;
    m_communicationController->configure(
        m_serialPort->currentText(), baud, m_serialSlaveId->value(), protocol);
    m_serialConnected->setText(tr("连接中"));
    return true;
}

void MainWindow::handleCommunicationResult(const gucds::CommunicationResult &result)
{
    m_serialConnected->setText(m_communicationController->isConnected()
                                   ? tr("已连接")
                                   : tr("未连接"));
    if (!result.request.isEmpty())
        m_serialTx->setText(result.request.simplified().left(180));
    const QString response = (m_serialHexDisplay && m_serialHexDisplay->isChecked())
        ? gucds::VirtualModbusClient::formatHex(result.responseBytes)
        : (result.responseText.isEmpty()
               ? gucds::VirtualModbusClient::formatHex(result.responseBytes)
               : result.responseText);
    m_serialRx->setText(response.simplified().left(180));
    if (!result.request.isEmpty())
        m_serialBuffer->appendPlainText(QStringLiteral("TX | %1").arg(result.request));
    if (!response.isEmpty())
        m_serialBuffer->appendPlainText(QStringLiteral("RX | %1").arg(response));

    if (!result.success) {
        appendStatus(result.message);
        if (!m_communicationController->isRunning()) {
            m_frequencyTensionMeasurementActive = false;
            m_measurementDeviceRecord = {};
        }
        QMessageBox::warning(this,
                             tr("通信失败"),
                             result.message.isEmpty() ? tr("设备没有返回有效响应。") : result.message);
        return;
    }

    if (result.context == QStringLiteral("scan")) {
        for (const gucds::DeviceRecord &device : result.devices) {
            if (!m_deviceModel->containsEquivalent(device))
                m_deviceModel->addRecord(device);
        }
    }
    if (result.context == QStringLiteral("spectrum")) {
        QVector<gucds::SpectrumSample> samples = gucds::AtProtocol::parseSpectrum(result.responseText);
        if (samples.isEmpty())
            appendStatus(tr("频谱回包未包含可配对的频率和幅值数据。"));
        else
            showSpectrum(samples);
    }
    if (result.context == QStringLiteral("frequency_tension_measurement")) {
        gucds::FrequencyTensionSample sample;
        bool parsed = false;
        if (m_communicationController->protocol() == gucds::DeviceWireProtocol::Modbus && result.hasSample) {
            sample.cableForceKn = result.sample.pitch;
            sample.naturalFrequencyHz = result.sample.roll;
            sample.order = result.sample.error;
            sample.convergenceErrorPercent = result.sample.temperature;
            parsed = true;
        } else {
            parsed = gucds::AtProtocol::parseFrequencyTensionMeasurement(result.responseText, &sample);
        }
        if (!parsed && result.hasSample) {
            sample.cableForceKn = result.sample.pitch;
            sample.naturalFrequencyHz = result.sample.roll;
            sample.order = result.sample.error;
            sample.convergenceErrorPercent = result.sample.temperature;
            parsed = true;
        }
        if (!parsed) {
            appendStatus(tr("索力测量回包缺少索力、fn、阶数或收敛误差。"));
            return;
        }

        recordFrequencyTensionMeasurement(sample);
        m_serialRx->setText(QStringLiteral("F=%1 fn=%2 n=%3 E=%4")
                                .arg(sample.cableForceKn, 0, 'f', 3)
                                .arg(sample.naturalFrequencyHz, 0, 'f', 3)
                                .arg(sample.order, 0, 'f', 3)
                                .arg(sample.convergenceErrorPercent, 0, 'f', 3));
        appendStatus(tr("索力测量完成：%1 kN，fn=%2 Hz，n=%3，误差=%4%")
                         .arg(sample.cableForceKn, 0, 'f', 3)
                         .arg(sample.naturalFrequencyHz, 0, 'f', 3)
                         .arg(sample.order, 0, 'f', 3)
                         .arg(sample.convergenceErrorPercent, 0, 'f', 3));
        // Automatic and low-power samples arrive while the worker remains active.
        // A command-mode result is final, so release its device snapshot here.
        if (!m_communicationController->isRunning()) {
            m_frequencyTensionMeasurementActive = false;
            m_measurementDeviceRecord = {};
        }
        return;
    }
    if (result.context == QStringLiteral("frequency_tension_stream_stopped")) {
        appendStatus(result.message);
        m_frequencyTensionMeasurementActive = false;
        m_measurementDeviceRecord = {};
        return;
    }
    if (result.hasSample) {
        recordMeasurement(result.sample, result.responseText);
        m_serialRx->setText(QStringLiteral("P=%1 R=%2 E=%3 T=%4")
                                .arg(double(result.sample.pitch), 0, 'f', 3)
                                .arg(double(result.sample.roll), 0, 'f', 3)
                                .arg(double(result.sample.error), 0, 'f', 4)
                                .arg(double(result.sample.temperature), 0, 'f', 2));
        if (result.context == QStringLiteral("calibration_measurement")) {
            setParameterText(QStringLiteral("标定"), QStringLiteral("测量值_标定"), QString::number(result.sample.pitch, 'g', 9));
            setParameterText(QStringLiteral("标定"), QStringLiteral("温度_标定"), QString::number(result.sample.temperature, 'g', 9));
        }
    }
    if (result.context == QStringLiteral("read_mcu_modbus"))
        fillMcuParameters(result.dataBytes);
    else if (result.context == QStringLiteral("read_lora_modbus"))
        fillLoraParameters(result.dataBytes);
    else if (result.context == QStringLiteral("read_frequency_mcu")
             || result.context == QStringLiteral("write_frequency_mcu")) {
        QVector<double> values = result.numericValues;
        if (result.context == QStringLiteral("read_frequency_mcu") && values.size() == 7) {
            values.insert(5, 0.0);
            values.insert(5, 0.0);
        }
        fillParameterGroup(QStringLiteral("MCU卡"), values);
    }
    else if (result.context == QStringLiteral("read_mcu_at"))
        fillParameterGroup(QStringLiteral("MCU卡"), result.numericValues);
    else if (result.context == QStringLiteral("read_lora_at"))
        fillParameterGroup(QStringLiteral("LoRa设置"), result.numericValues);
    else if (result.context == QStringLiteral("at_manager")) {
        setParameterText(QStringLiteral("AT管理器"), QStringLiteral("AT回包"), response);
        setParameterText(QStringLiteral("AT管理器"), QStringLiteral("AT说明"), result.message);
    } else if (result.context == QStringLiteral("bus_test")) {
        setParameterText(QStringLiteral("总线设备管理器"), QStringLiteral("应答码"), response);
        m_busDeviceModel->updateLastResponse(response);
    }
    appendStatus(result.message);
}

QWidget *MainWindow::parameterEditor(const QString &title, const QString &label) const
{
    return m_parameterEditors.value(title + QChar(0x1f) + label, nullptr);
}

QString MainWindow::parameterText(const QString &title, const QString &label) const
{
    QWidget *editor = parameterEditor(title, label);
    if (const auto *line = qobject_cast<QLineEdit *>(editor))
        return line->text().trimmed();
    if (const auto *box = qobject_cast<QComboBox *>(editor))
        return box->currentText().trimmed();
    if (const auto *spin = qobject_cast<QSpinBox *>(editor))
        return QString::number(spin->value());
    return {};
}

int MainWindow::parameterInt(const QString &title, const QString &label) const
{
    QWidget *editor = parameterEditor(title, label);
    if (const auto *spin = qobject_cast<QSpinBox *>(editor))
        return spin->value();
    if (const auto *box = qobject_cast<QComboBox *>(editor)) {
        bool ok = false;
        const int numericValue = box->currentText().toInt(&ok);
        return ok ? numericValue : box->currentIndex();
    }
    bool ok = false;
    const int value = parameterText(title, label).toInt(&ok);
    return ok ? value : 0;
}

void MainWindow::setParameterText(const QString &title, const QString &label, const QString &value)
{
    QWidget *editor = parameterEditor(title, label);
    if (auto *line = qobject_cast<QLineEdit *>(editor)) {
        line->setText(value);
    } else if (auto *box = qobject_cast<QComboBox *>(editor)) {
        const int exactIndex = box->findText(value);
        if (exactIndex >= 0) {
            box->setCurrentIndex(exactIndex);
        } else {
            bool ok = false;
            const int index = value.toInt(&ok);
            if (ok && index >= 0 && index < box->count())
                box->setCurrentIndex(index);
            else if (box->isEditable())
                box->setCurrentText(value);
        }
    } else if (auto *spin = qobject_cast<QSpinBox *>(editor)) {
        bool ok = false;
        const int numericValue = value.toInt(&ok);
        if (ok)
            spin->setValue(numericValue);
    }
}

void MainWindow::fillParameterGroup(const QString &title, const QVector<double> &values)
{
    if (title == QStringLiteral("MCU卡") && values.size() >= 5) {
        setParameterText(title, QStringLiteral("波特率_MCU"), QString::number(qRound64(values.at(0))));
        setParameterText(title, QStringLiteral("Gap_MCU"), QString::number(qRound(values.at(1))));
        setParameterText(title, QStringLiteral("Mode_MCU"), QString::number(qRound(values.at(2))));
        setParameterText(title, QStringLiteral("ModID_MCU"), QString::number(qRound(values.at(3))));
        setParameterText(title, QStringLiteral("RS485_MCU"), QString::number(qRound(values.at(4))));
        if (values.size() >= 9) {
            if (auto *sampleRate = qobject_cast<QComboBox *>(
                    parameterEditor(title, QStringLiteral("采样频率_FVCF")))) {
                const int index = qRound(values.at(7));
                if (index >= 0 && index < sampleRate->count())
                    sampleRate->setCurrentIndex(index);
            }
            if (auto *samplePoints = qobject_cast<QComboBox *>(
                    parameterEditor(title, QStringLiteral("采样点数_FVCF")))) {
                const int index = qRound(values.at(8));
                if (index >= 0 && index < samplePoints->count())
                    samplePoints->setCurrentIndex(index);
            }
        }
    } else if (title == QStringLiteral("LoRa设置") && values.size() >= 10) {
        setParameterText(title, QStringLiteral("波特率_LR"), QString::number(qRound64(values.at(0))));
        setParameterText(title, QStringLiteral("信道_LR"), QString::number(qRound(values.at(1))));
        setParameterText(title, QStringLiteral("功率_LR"), QString::number(qRound(values.at(2))));
        setParameterText(title, QStringLiteral("空速_LR"), QString::number(qRound(values.at(3))));
        const quint16 upperMode = gucds::VirtualModbusClient::loraModuleModeToUpper(
            quint16(qRound(values.at(4))));
        setParameterText(title,
                         QStringLiteral("工作模式_LR"),
                         QString::number(upperMode == 0x00FF ? 0 : upperMode));
        setParameterText(title, QStringLiteral("主/从_LR"), QString::number(qRound(values.at(5))));
        setParameterText(title, QStringLiteral("本地组号_LR"), QString::number(qRound(values.at(6))));
        setParameterText(title, QStringLiteral("本地地址_LR"), QString::number(qRound(values.at(7))));
        setParameterText(title, QStringLiteral("目标组号_LR"), QString::number(qRound(values.at(8))));
        setParameterText(title, QStringLiteral("目标地址_LR"), QString::number(qRound(values.at(9))));
    }
}

void MainWindow::fillMcuParameters(const QByteArray &bytes)
{
    if (bytes.size() < 10)
        return;
    const quint32 baud = gucds::VirtualModbusClient::decodeMcuBaud(bytes);
    const quint16 gap = (quint16(quint8(bytes.at(4))) << 8) | quint8(bytes.at(5));
    fillParameterGroup(QStringLiteral("MCU卡"),
                       {double(baud), double(gap), double(quint8(bytes.at(6))),
                        double(quint8(bytes.at(8))), double(quint8(bytes.at(9)))});
}

void MainWindow::fillLoraParameters(const QByteArray &bytes)
{
    if (bytes.size() < 14)
        return;
    const quint32 baud = (quint32(quint8(bytes.at(0))) << 24)
        | (quint32(quint8(bytes.at(1))) << 16)
        | (quint32(quint8(bytes.at(2))) << 8)
        | quint8(bytes.at(3));
    const quint16 cpr = (quint16(quint8(bytes.at(4))) << 8) | quint8(bytes.at(5));
    const quint16 workMode = (quint16(quint8(bytes.at(6))) << 8) | quint8(bytes.at(7));
    fillParameterGroup(QStringLiteral("LoRa设置"),
                       {double(baud), double(cpr >> 5), double((cpr >> 3) & 0x03),
                        double(cpr & 0x07),
                        double(gucds::VirtualModbusClient::loraUpperModeToModule(workMode)),
                        double(quint8(bytes.at(8))),
                        double(quint8(bytes.at(10))), double(quint8(bytes.at(11))),
                        double(quint8(bytes.at(12))), double(quint8(bytes.at(13)))});
}

void MainWindow::handleParameterAction(const QString &title, const QString &action)
{
    if (title == QStringLiteral("MCU卡")) {
        if (action == QStringLiteral("读MCU")) {
            if (!configureCommunication())
                return;
            const bool frequencyTension = activeDeviceIsFrequencyTension();
            const bool started = frequencyTension
                ? m_communicationController->sendTextCommands(
                      {gucds::AtProtocol::command(gucds::AtCommand::GetMcuParameters),
                       gucds::AtProtocol::command(gucds::AtCommand::GetSensorParameters)},
                      QStringLiteral("read_frequency_mcu"),
                      100)
                : (m_serialProtocol->currentText() == QStringLiteral("Modbus")
                       ? m_communicationController->readModbusRegisters(
                             4, 5, QStringLiteral("read_mcu_modbus"))
                       : m_communicationController->sendTextCommand(
                             gucds::AtProtocol::command(gucds::AtCommand::GetMcuParameters),
                             QStringLiteral("read_mcu_at")));
            if (!started)
                appendStatus(tr("已有通信任务正在运行，请稍后重试"));
            return;
        }
        if (action == QStringLiteral("写MCU")) {
            const quint32 baud = quint32(parameterInt(title, QStringLiteral("波特率_MCU")));
            const quint16 gap = quint16(parameterInt(title, QStringLiteral("Gap_MCU")));
            QByteArray bytes = gucds::VirtualModbusClient::encodeMcuBaud(baud);
            bytes.append(char((gap >> 8) & 0xFF));
            bytes.append(char(gap & 0xFF));
            bytes.append(char(parameterInt(title, QStringLiteral("Mode_MCU"))));
            bytes.append(char(0));
            bytes.append(char(parameterInt(title, QStringLiteral("ModID_MCU"))));
            bytes.append(char(parameterInt(title, QStringLiteral("RS485_MCU"))));
            if (activeDeviceIsFrequencyTension()) {
                auto *sampleRate = qobject_cast<QComboBox *>(
                    parameterEditor(title, QStringLiteral("采样频率_FVCF")));
                auto *samplePoints = qobject_cast<QComboBox *>(
                    parameterEditor(title, QStringLiteral("采样点数_FVCF")));
                if (!sampleRate || !samplePoints || !configureCommunication())
                    return;
                m_pendingSamplingRateIndex = sampleRate->currentIndex();
                m_pendingSamplingPointIndex = samplePoints->currentIndex();
                if (!m_communicationController->writeFrequencyMcuParameters(
                        bytes,
                        quint8(m_pendingSamplingRateIndex),
                        quint8(m_pendingSamplingPointIndex),
                        QStringLiteral("write_frequency_mcu"))) {
                    appendStatus(tr("采样参数无效或已有通信任务正在运行"));
                }
                return;
            }
            writeModbusParameters(2,
                                   bytes,
                                   gucds::SensorModbusCommand::SaveMcuParameters,
                                   QStringLiteral("write_mcu"));
            return;
        }
        if (action == QStringLiteral("重启设备")) {
            if (QMessageBox::warning(this,
                                     tr("重启设备"),
                                     tr("设备会立即重启并短暂断开串口，确定继续吗？"),
                                     QMessageBox::Yes | QMessageBox::Cancel,
                                     QMessageBox::Cancel)
                == QMessageBox::Yes) {
                sendModbusCommand(gucds::SensorModbusCommand::Reboot, QStringLiteral("reboot"));
            }
            return;
        }
    }

    if (title == QStringLiteral("LoRa设置")) {
        if (action == QStringLiteral("读LoRa")) {
            if (!configureCommunication())
                return;
            const bool started = m_serialProtocol->currentText() == QStringLiteral("Modbus")
                ? m_communicationController->readModbusRegisters(20, 8, QStringLiteral("read_lora_modbus"))
                : m_communicationController->sendTextCommand(
                      gucds::AtProtocol::command(gucds::AtCommand::GetLoraParameters),
                      QStringLiteral("read_lora_at"));
            if (!started)
                appendStatus(tr("已有通信任务正在运行，请稍后重试"));
            return;
        }
        if (action == QStringLiteral("写LoRa")) {
            const quint32 baud = quint32(parameterInt(title, QStringLiteral("波特率_LR")));
            const quint16 cpr = quint16((parameterInt(title, QStringLiteral("信道_LR")) << 5)
                                        | (parameterInt(title, QStringLiteral("功率_LR")) << 3)
                                        | parameterInt(title, QStringLiteral("空速_LR")));
            const quint16 workMode = quint16(parameterInt(title, QStringLiteral("工作模式_LR")));
            QByteArray bytes;
            bytes.append(char((baud >> 24) & 0xFF));
            bytes.append(char((baud >> 16) & 0xFF));
            bytes.append(char((baud >> 8) & 0xFF));
            bytes.append(char(baud & 0xFF));
            bytes.append(char((cpr >> 8) & 0xFF));
            bytes.append(char(cpr & 0xFF));
            bytes.append(char((workMode >> 8) & 0xFF));
            bytes.append(char(workMode & 0xFF));
            bytes.append(char(parameterInt(title, QStringLiteral("主/从_LR"))));
            bytes.append(char(0));
            bytes.append(char(parameterInt(title, QStringLiteral("本地组号_LR"))));
            bytes.append(char(parameterInt(title, QStringLiteral("本地地址_LR"))));
            bytes.append(char(parameterInt(title, QStringLiteral("目标组号_LR"))));
            bytes.append(char(parameterInt(title, QStringLiteral("目标地址_LR"))));
            writeModbusParameters(10,
                                   bytes,
                                   gucds::SensorModbusCommand::SaveLoraParameters,
                                   QStringLiteral("write_lora"));
            return;
        }
    }

    if (title == QStringLiteral("DTU主卡")) {
        if (action == QStringLiteral("网络配置"))
            openDtuConfiguration(title, QStringLiteral("write_dtu"));
        else if (action == QStringLiteral("检测状态"))
            testDtuNetwork(QStringLiteral("dtu_test"));
        return;
    }

    if (title == QStringLiteral("标定")) {
        if (action == QStringLiteral("添加")) {
            setParameterText(title, QStringLiteral("点号_标定"), QString::number(m_calibrationModel->rowCount() + 1));
            setParameterText(title, QStringLiteral("测量值_标定"), QString());
            setParameterText(title, QStringLiteral("标定值_标定"), QString());
            setParameterText(title, QStringLiteral("温度_标定"), QString());
            m_calibrationTable->clearSelection();
        } else if (action == QStringLiteral("保存")) {
            saveCalibration(false);
        } else if (action == QStringLiteral("修改")) {
            saveCalibration(true);
        } else if (action == QStringLiteral("删除")) {
            deleteCalibration();
        } else if (action == QStringLiteral("测量")) {
            if (!configureCommunication())
                return;
            if (!m_communicationController->measure(QStringLiteral("calibration_measurement")))
                appendStatus(tr("已有通信任务正在运行，请稍后重试"));
        }
        return;
    }

    if (title == QStringLiteral("总线设备管理器")) {
        if (action == QStringLiteral("保存"))
            saveBusDevice(false);
        else if (action == QStringLiteral("修改"))
            saveBusDevice(true);
        else if (action == QStringLiteral("删除")) {
            const QModelIndex current = m_busDeviceTable->currentIndex();
            if (!current.isValid()) {
                QMessageBox::information(this, tr("删除总线设备"), tr("请先选择要删除的设备。"));
            } else {
                gucds::BusDeviceRecord record = m_busDeviceModel->recordAt(current.row());
                QString error;
                if ((record.databaseId <= 0 || gucds::LabviewDatabase::deleteBusDeviceRecord(
                                                  gucds::LabviewDatabase::defaultDatabasePath(), record.databaseId, &error))
                    && m_busDeviceModel->removeDevice(current.row())) {
                    appendStatus(tr("总线设备已删除"));
                } else {
                    QMessageBox::warning(this, tr("删除失败"), error);
                }
            }
        } else if (action == QStringLiteral("开始测试")) {
            sendDeviceCommand(gucds::AtProtocol::buildSensorTest(
                                  {parameterText(title, QStringLiteral("信道")),
                                   parameterText(title, QStringLiteral("组号")),
                                   parameterText(title, QStringLiteral("地址")),
                                   parameterText(title, QStringLiteral("数据数"))}),
                              QStringLiteral("bus_test"));
        }
        return;
    }

    if (title == QStringLiteral("DTU网络")) {
        if (action == QStringLiteral("网络配置"))
            openDtuConfiguration(title, QStringLiteral("network_config"));
        else if (action == QStringLiteral("检测状态"))
            testDtuNetwork(QStringLiteral("network_status"));
        return;
    }

    if (title == QStringLiteral("AT管理器")) {
        if (action == QStringLiteral("发送")) {
            const QString command = parameterText(title, QStringLiteral("AT指令"));
            if (command.isEmpty()) {
                QMessageBox::information(this, tr("AT 管理器"), tr("请先填写 AT 指令。"));
                return;
            }
            sendDeviceCommand(command,
                              QStringLiteral("at_manager"),
                              parameterInt(title, QStringLiteral("AT次数")));
        } else if (action == QStringLiteral("清除")) {
            setParameterText(title, QStringLiteral("AT指令"), QString());
            setParameterText(title, QStringLiteral("AT回包"), QString());
            setParameterText(title, QStringLiteral("AT说明"), QString());
            appendStatus(tr("AT 管理器已清除"));
        }
    }
}

void MainWindow::saveCalibration(bool update)
{
    const QModelIndex current = m_calibrationTable->currentIndex();
    if (update && !current.isValid()) {
        QMessageBox::information(this, tr("修改标定点"), tr("请先选择要修改的标定点。"));
        return;
    }
    bool pointOk = false;
    bool measuredOk = false;
    bool referenceOk = false;
    bool temperatureOk = false;
    gucds::CalibrationRecord record = update
        ? m_calibrationModel->recordAt(current.row())
        : gucds::CalibrationRecord{};
    record.curveName = parameterText(QStringLiteral("标定"), QStringLiteral("曲线名_标定"));
    record.point = parameterText(QStringLiteral("标定"), QStringLiteral("点号_标定")).toInt(&pointOk);
    record.measuredValue = parameterText(QStringLiteral("标定"), QStringLiteral("测量值_标定")).toDouble(&measuredOk);
    record.referenceValue = parameterText(QStringLiteral("标定"), QStringLiteral("标定值_标定")).toDouble(&referenceOk);
    record.temperature = parameterText(QStringLiteral("标定"), QStringLiteral("温度_标定")).toDouble(&temperatureOk);
    record.timestamp = QDateTime::currentDateTime();
    if (record.curveName.isEmpty() || !pointOk || !measuredOk || !referenceOk || !temperatureOk) {
        QMessageBox::warning(this, tr("标定数据"), tr("请完整填写曲线名、点号、测量值、标定值和温度。"));
        return;
    }
    QString error;
    if (!gucds::LabviewDatabase::saveCalibrationRecord(
            gucds::LabviewDatabase::defaultDatabasePath(), &record, &error)) {
        QMessageBox::warning(this, tr("保存失败"), error);
        return;
    }
    if (update)
        m_calibrationModel->updatePoint(current.row(), record);
    else
        m_calibrationModel->addPoint(record);
    appendStatus(update ? tr("标定点已修改") : tr("标定点已保存"));
}

void MainWindow::deleteCalibration()
{
    const QModelIndex current = m_calibrationTable->currentIndex();
    if (!current.isValid()) {
        QMessageBox::information(this, tr("删除标定点"), tr("请先选择要删除的标定点。"));
        return;
    }
    const gucds::CalibrationRecord record = m_calibrationModel->recordAt(current.row());
    if (QMessageBox::question(this,
                              tr("删除标定点"),
                              tr("确定删除曲线“%1”的第 %2 点吗？").arg(record.curveName).arg(record.point),
                              QMessageBox::Yes | QMessageBox::Cancel,
                              QMessageBox::Cancel)
        != QMessageBox::Yes) {
        return;
    }
    QString error;
    if (record.databaseId > 0
        && !gucds::LabviewDatabase::deleteCalibrationRecord(
            gucds::LabviewDatabase::defaultDatabasePath(), record.databaseId, &error)) {
        QMessageBox::warning(this, tr("删除失败"), error);
        return;
    }
    m_calibrationModel->removePoint(current.row());
    appendStatus(tr("标定点已删除"));
}

void MainWindow::saveBusDevice(bool update)
{
    const QModelIndex current = m_busDeviceTable->currentIndex();
    if (update && !current.isValid()) {
        QMessageBox::information(this, tr("修改总线设备"), tr("请先选择要修改的设备。"));
        return;
    }
    gucds::BusDeviceRecord record = update
        ? m_busDeviceModel->recordAt(current.row())
        : gucds::BusDeviceRecord{};
    record.index = update ? record.index : m_busDeviceModel->rowCount() + 1;
    record.channel = parameterInt(QStringLiteral("总线设备管理器"), QStringLiteral("信道"));
    record.group = parameterInt(QStringLiteral("总线设备管理器"), QStringLiteral("组号"));
    record.address = parameterInt(QStringLiteral("总线设备管理器"), QStringLiteral("地址"));
    record.dataCount = parameterInt(QStringLiteral("总线设备管理器"), QStringLiteral("数据数"));
    record.sensorName = parameterText(QStringLiteral("总线设备管理器"), QStringLiteral("设备名称"));
    record.responseCode = parameterText(QStringLiteral("总线设备管理器"), QStringLiteral("应答码"));
    if (record.sensorName.isEmpty()) {
        QMessageBox::warning(this, tr("总线设备"), tr("设备名称不能为空。"));
        return;
    }
    QString error;
    if (!gucds::LabviewDatabase::saveBusDeviceRecord(
            gucds::LabviewDatabase::defaultDatabasePath(), &record, &error)) {
        QMessageBox::warning(this, tr("保存失败"), error);
        return;
    }
    if (update)
        m_busDeviceModel->updateDevice(current.row(), record);
    else
        m_busDeviceModel->addDevice(record);
    appendStatus(update ? tr("总线设备已修改") : tr("总线设备已保存"));
}

void MainWindow::promptExtendedParameters(const QString &title, const QStringList &defaults)
{
    bool accepted = false;
    const QString text = QInputDialog::getText(
        this,
        title,
        tr("请输入逗号分隔的扩展参数："),
        QLineEdit::Normal,
        defaults.join(QLatin1Char(',')),
        &accepted);
    if (!accepted)
        return;
    const QStringList values = text.split(QLatin1Char(','), Qt::SkipEmptyParts);
    if (values.isEmpty()) {
        QMessageBox::warning(this, title, tr("扩展参数不能为空。"));
        return;
    }
    sendDeviceCommand(gucds::AtProtocol::buildSetExtendedParameters(values),
                      QStringLiteral("extended_parameters"));
}
