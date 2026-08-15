#pragma once

#include "gucds/core/records.h"

#include <QDialog>
#include <QVector>

class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QWidget;

class FrequencyTensionParameterDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FrequencyTensionParameterDialog)

public:
    explicit FrequencyTensionParameterDialog(const QString &databasePath, QWidget *parent = nullptr);

    bool hasUnsavedChanges() const;
    void setSensorWriteAvailable(bool available);
    void finishSensorWrite(bool success, const QString &message, const QVector<double> &readbackValues);

signals:
    void sensorWriteRequested(const gucds::FrequencyTensionParameterRecord &record);

protected:
    void reject() override;

private:
    void buildUi();
    bool loadRecords();
    void rebuildTable();
    void selectRow(int row);
    void loadEditor(int row);
    void clearEditor();
    bool readEditor(gucds::FrequencyTensionParameterRecord *record);
    bool parseNumber(QLineEdit *edit, const QString &label, bool positive, double *value);
    bool sensorNameExists(const QString &sensorName, int excludedRow = -1) const;
    void addParameter();
    void modifyParameter();
    void writeCurrentParameterToSensor();
    bool saveParameters(bool offerSensorWrite = true);
    void startSensorWrite(const gucds::FrequencyTensionParameterRecord &record);
    bool resolveUnsavedChanges();
    void setDirty(bool dirty);

    QString m_databasePath;
    QVector<gucds::FrequencyTensionParameterRecord> m_records;
    QTableWidget *m_table = nullptr;
    QLineEdit *m_sensorNameEdit = nullptr;
    QLineEdit *m_supportFactorEdit = nullptr;
    QLineEdit *m_unitMassEdit = nullptr;
    QLineEdit *m_cableLengthEdit = nullptr;
    QLineEdit *m_areaEdit = nullptr;
    QLineEdit *m_elasticModulusEdit = nullptr;
    QLineEdit *m_inertiaEdit = nullptr;
    QLineEdit *m_angleEdit = nullptr;
    QPushButton *m_modifyButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_writeSensorButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    gucds::FrequencyTensionParameterRecord m_pendingSensorRecord;
    bool m_dirty = false;
    bool m_sensorWriteAvailable = false;
    bool m_sensorWritePending = false;
};
