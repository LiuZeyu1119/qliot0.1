#pragma once

#include "gucds/core/records.h"

#include <QByteArray>
#include <QMainWindow>
#include <QHash>
#include <QString>
#include <QVector>

class QComboBox;
class QCheckBox;
class QAction;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QTableView;
class QTreeWidget;
class QWidget;

class SpectrumPlotWidget;
class DtuConfigDialog;

namespace gucds {
class BusDeviceTableModel;
class CalibrationTableModel;
class DeviceTableModel;
class MeasurementTableModel;
class DeviceCommunicationController;
struct CommunicationResult;
struct FrequencyTensionSample;
struct SensorModbusSample;
struct SpectrumSample;
enum class SensorModbusCommand : quint16;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void languageChangeRequested(const QString &language);

private:
    void buildMenus();
    QWidget *buildCentralWidget();
    QWidget *buildDeviceTablePanel();
    QWidget *buildDeviceLibraryPanel();
    QWidget *buildSelectedDeviceEditorPanel();
    QWidget *buildSerialPanel();
    QWidget *buildMainTabs();
    QWidget *buildTestDataTab();
    QWidget *buildDeviceConfigTab();
    QWidget *buildCalibrationTab();
    QWidget *buildBusGatewayTab();
    QWidget *buildParameterGroup(const QString &title, const QStringList &labels, const QStringList &buttons);
    QLabel *sectionTitle(const QString &text) const;
    QComboBox *combo(const QStringList &items) const;
    QComboBox *onOffCombo() const;
    QLineEdit *lineEdit(const QString &text = {}) const;
    QSpinBox *byteSpinBox(int value = 0) const;
    QPushButton *labviewButton(const QString &text) const;
    void configureTable(QTableView *table) const;
    void appendStatus(const QString &message);
    void loadPersistedLabviewData();
    void reloadDeviceLibrary();
    void openProductManagement();
    void openFrequencyTensionParameters();
    void populateDeviceLibraryTree(const QVector<gucds::DeviceRecord> &records);
    void showLibraryDeviceDetails(int libraryIndex);
    void showConfiguredDeviceDetails(int row);
    void populateDeviceEditor(const gucds::DeviceRecord &record, const QString &sourceText);
    void clearDeviceEditor();
    gucds::DeviceRecord editedDeviceRecord() const;
    void rebuildDeviceParameterEditors(const gucds::DeviceRecord &record);
    QWidget *editorForParameterSpec(const QString &spec) const;
    QString parameterValueFromEditor(QWidget *editor) const;
    QString parameterDefinitionSummary(const QString &spec) const;
    void addSelectedLibraryDeviceToConfig();
    void addLibraryDeviceToConfig(int libraryIndex);
    void addEditedDeviceToConfig();
    void applyEditedDeviceToConfig();
    void removeSelectedConfiguredDevice();
    void clearConfiguredDevices();
    void moveSelectedConfiguredDevice(int offset);
    int selectedConfiguredDeviceRow() const;
    void scanDevices();
    void startMeasurement();
    void recordMeasurement(const gucds::SensorModbusSample &sample, const QString &trace);
    void recordFrequencyTensionMeasurement(const gucds::FrequencyTensionSample &sample);
    void readSpectrum();
    bool activeDeviceIsFrequencyTension() const;
    void updateTestDataModeForDevice(const gucds::DeviceRecord &record);
    void configureMeasurementTableColumns();
    void showFrequencyTensionMeasurements();
    void showSpectrum(const QVector<gucds::SpectrumSample> &samples);
    void deleteSelectedMeasurements();
    void saveMeasurementsAsCsv();
    void refreshMeasurementPresentation();
    void updateMeasurementActions();
    void sendDeviceCommand(const QString &request, const QString &context = {}, int repeatCount = 1);
    void sendModbusCommand(gucds::SensorModbusCommand command, const QString &context = {});
    void writeModbusParameters(quint16 startAddress,
                               const QByteArray &bytes,
                               gucds::SensorModbusCommand saveCommand,
                               const QString &context);
    void openDtuConfiguration(const QString &title, const QString &context);
    void testDtuNetwork(const QString &context);
    bool configureCommunication();
    void handleCommunicationResult(const gucds::CommunicationResult &result);
    void handleParameterAction(const QString &title, const QString &action);
    QWidget *parameterEditor(const QString &title, const QString &label) const;
    QString parameterText(const QString &title, const QString &label) const;
    int parameterInt(const QString &title, const QString &label) const;
    void setParameterText(const QString &title, const QString &label, const QString &value);
    void fillParameterGroup(const QString &title, const QVector<double> &values);
    void fillMcuParameters(const QByteArray &bytes);
    void fillLoraParameters(const QByteArray &bytes);
    void saveCalibration(bool update);
    void deleteCalibration();
    void saveBusDevice(bool update);
    void promptExtendedParameters(const QString &title, const QStringList &defaults);

    gucds::DeviceTableModel *m_deviceModel = nullptr;
    gucds::MeasurementTableModel *m_measurementModel = nullptr;
    gucds::CalibrationTableModel *m_calibrationModel = nullptr;
    gucds::BusDeviceTableModel *m_busDeviceModel = nullptr;
    gucds::DeviceCommunicationController *m_communicationController = nullptr;
    SpectrumPlotWidget *m_plot = nullptr;
    QLabel *m_measurementSectionTitle = nullptr;
    QTableView *m_measurementTable = nullptr;
    QAction *m_deleteMeasurementAction = nullptr;
    QAction *m_saveMeasurementsAction = nullptr;
    QPushButton *m_deleteMeasurementButton = nullptr;
    QPushButton *m_saveMeasurementsButton = nullptr;
    QListWidget *m_latestDataList = nullptr;
    QLineEdit *m_latestBand = nullptr;
    QLineEdit *m_latestCount = nullptr;
    QTabWidget *m_mainTabs = nullptr;
    QComboBox *m_serialPort = nullptr;
    QComboBox *m_serialBaud = nullptr;
    QComboBox *m_serialProtocol = nullptr;
    QSpinBox *m_serialSlaveId = nullptr;
    QLineEdit *m_serialConnected = nullptr;
    QLineEdit *m_serialRx = nullptr;
    QLineEdit *m_serialTx = nullptr;
    QPlainTextEdit *m_serialBuffer = nullptr;
    QPushButton *m_serialStartButton = nullptr;
    QCheckBox *m_serialHexDisplay = nullptr;
    QTableView *m_deviceConfigTable = nullptr;
    QTreeWidget *m_deviceLibraryTree = nullptr;
    QGroupBox *m_deviceEditorGroup = nullptr;
    QLabel *m_deviceEditorSourceLabel = nullptr;
    QLineEdit *m_deviceNameEditor = nullptr;
    QLineEdit *m_deviceCategoryEditor = nullptr;
    QLineEdit *m_deviceModelEditor = nullptr;
    QVector<QLineEdit *> m_deviceDataEditors;
    QComboBox *m_deviceModbusEditor = nullptr;
    QComboBox *m_deviceLoraEditor = nullptr;
    QComboBox *m_deviceDtuEditor = nullptr;
    QSpinBox *m_deviceCalibrationPointsEditor = nullptr;
    QFormLayout *m_deviceParameterForm = nullptr;
    QVector<QWidget *> m_deviceParameterEditors;
    QPushButton *m_addEditedDeviceButton = nullptr;
    QPushButton *m_applyEditedDeviceButton = nullptr;
    QVector<gucds::DeviceRecord> m_deviceLibraryRecords;
    QHash<QString, QWidget *> m_parameterEditors;
    QTableView *m_calibrationTable = nullptr;
    QTableView *m_busDeviceTable = nullptr;
    DtuConfigDialog *m_dtuConfigDialog = nullptr;
    gucds::DeviceRecord m_activeDeviceRecord;
    gucds::DeviceRecord m_measurementDeviceRecord;
    bool m_hasActiveDeviceRecord = false;
    int m_activeLibraryIndex = -1;
    int m_activeConfiguredRow = -1;
    int m_pendingSamplingRateIndex = -1;
    bool m_frequencyTensionMeasurementActive = false;
    int m_pendingSamplingPointIndex = -1;
};
