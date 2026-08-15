#include "gucds/core/labviewdatabase.h"
#include "gucds/widgets/frequencytensionparameterdialog.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest/QtTest>

class FrequencyTensionParametersUiTest : public QObject
{
    Q_OBJECT

private slots:
    void localParameterWorkflowPersistsWithoutCommunication();
};

void FrequencyTensionParametersUiTest::localParameterWorkflowPersistsWithoutCommunication()
{
    const QString sourceDatabase = gucds::LabviewDatabase::defaultDatabasePath();
    QVERIFY2(QFile::exists(sourceDatabase), qPrintable(sourceDatabase));
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString databasePath = temporaryDirectory.filePath(QStringLiteral("gucds.sqlite"));
    QVERIFY(QFile::copy(sourceDatabase, databasePath));

    QString errorMessage;
    const QVector<gucds::FrequencyTensionParameterRecord> originalRecords =
        gucds::LabviewDatabase::loadFrequencyTensionParameters(databasePath, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(!originalRecords.isEmpty());

    QVERIFY(!gucds::LabviewDatabase::saveFrequencyTensionParameters(
        databasePath, nullptr, &errorMessage));
    QVERIFY(!errorMessage.isEmpty());
    QVector<gucds::FrequencyTensionParameterRecord> invalidRecords = originalRecords;
    invalidRecords.first().supportFactor = 0.0;
    QVERIFY(!gucds::LabviewDatabase::saveFrequencyTensionParameters(
        databasePath, &invalidRecords, &errorMessage));
    QVERIFY(!errorMessage.isEmpty());

    FrequencyTensionParameterDialog dialog(databasePath);
    dialog.show();
    QCoreApplication::processEvents();

    auto *table = dialog.findChild<QTableWidget *>(QStringLiteral("frequencyParameterTable"));
    auto *sensorName = dialog.findChild<QLineEdit *>(QStringLiteral("frequencyParameterSensorName"));
    auto *supportFactor = dialog.findChild<QLineEdit *>(QStringLiteral("frequencyParameterSupportFactor"));
    auto *addButton = dialog.findChild<QPushButton *>(QStringLiteral("addFrequencyParameterButton"));
    auto *modifyButton = dialog.findChild<QPushButton *>(QStringLiteral("modifyFrequencyParameterButton"));
    auto *saveButton = dialog.findChild<QPushButton *>(QStringLiteral("saveFrequencyParameterButton"));
    auto *writeSensorButton = dialog.findChild<QPushButton *>(
        QStringLiteral("writeFrequencyParameterSensorButton"));
    QVERIFY(table);
    QVERIFY(sensorName);
    QVERIFY(supportFactor);
    QVERIFY(addButton);
    QVERIFY(modifyButton);
    QVERIFY(saveButton);
    QVERIFY(writeSensorButton);

    QCOMPARE(table->rowCount(), originalRecords.size());
    QCOMPARE(table->columnCount(), 8);
    QCOMPARE(table->horizontalHeaderItem(0)->text(), QStringLiteral("传感器名"));
    table->selectRow(0);
    table->setCurrentCell(0, 0);
    QCoreApplication::processEvents();
    QCOMPARE(sensorName->text(), originalRecords.first().sensorName);

    const double updatedFactor = originalRecords.first().supportFactor + 0.125;
    supportFactor->setText(QLocale().toString(updatedFactor, 'g', 12));
    modifyButton->click();
    QVERIFY(dialog.hasUnsavedChanges());
    QVERIFY(saveButton->isEnabled());

    const QString newSensorName = QStringLiteral("UI-Test-Frequency-Parameter");
    sensorName->setText(newSensorName);
    addButton->click();
    QCOMPARE(table->rowCount(), originalRecords.size() + 1);
    saveButton->click();
    QVERIFY(!dialog.hasUnsavedChanges());
    QVERIFY(!saveButton->isEnabled());

    gucds::FrequencyTensionParameterRecord requestedRecord;
    bool writeRequested = false;
    connect(&dialog,
            &FrequencyTensionParameterDialog::sensorWriteRequested,
            &dialog,
            [&](const gucds::FrequencyTensionParameterRecord &record) {
                requestedRecord = record;
                writeRequested = true;
            });
    dialog.setSensorWriteAvailable(true);
    QVERIFY(!saveButton->isEnabled());
    QVERIFY(writeSensorButton->isEnabled());
    writeSensorButton->click();
    QVERIFY(writeRequested);
    QCOMPARE(requestedRecord.sensorName, newSensorName);
    QCOMPARE(requestedRecord.supportFactor, updatedFactor);
    dialog.finishSensorWrite(
        true,
        QStringLiteral("ok"),
        {requestedRecord.supportFactor,
         requestedRecord.unitMass,
         requestedRecord.cableLength,
         requestedRecord.area,
         requestedRecord.elasticModulus,
         requestedRecord.inertia,
         requestedRecord.angle});
    auto *status = dialog.findChild<QLabel *>(QStringLiteral("frequencyParameterStatus"));
    QVERIFY(status);
    QVERIFY(status->text().contains(QStringLiteral("回读校验一致")));

    const QVector<gucds::FrequencyTensionParameterRecord> savedRecords =
        gucds::LabviewDatabase::loadFrequencyTensionParameters(databasePath, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(savedRecords.size(), originalRecords.size() + 1);

    bool updatedRecordFound = false;
    bool newRecordFound = false;
    for (const gucds::FrequencyTensionParameterRecord &record : savedRecords) {
        if (record.databaseId == originalRecords.first().databaseId) {
            updatedRecordFound = true;
            QCOMPARE(record.supportFactor, updatedFactor);
        }
        if (record.sensorName == newSensorName) {
            newRecordFound = true;
            QVERIFY(record.databaseId > 0);
        }
    }
    QVERIFY(updatedRecordFound);
    QVERIFY(newRecordFound);

    const QString screenshotPath = qEnvironmentVariable("QLIOT_FREQUENCY_UI_SCREENSHOT");
    if (!screenshotPath.isEmpty())
        QVERIFY(dialog.grab().save(screenshotPath));
}

QTEST_MAIN(FrequencyTensionParametersUiTest)
#include "test_frequency_tension_parameters_ui.moc"
