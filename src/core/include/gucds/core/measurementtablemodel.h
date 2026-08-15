#pragma once

#include "gucds/core/records.h"

#include <QAbstractTableModel>
#include <QVector>

namespace gucds {

class MeasurementTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class Mode
    {
        Generic,
        FrequencyTension,
        Spectrum,
    };
    Q_ENUM(Mode)

    explicit MeasurementTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    Mode mode() const;
    void setMode(Mode mode);
    void addMeasurement(const MeasurementRecord &record);
    void addFrequencyTensionMeasurement(const FrequencyTensionMeasurementRecord &record);
    void setSpectrumPoints(const QVector<SpectrumPoint> &points);
    bool saveCsv(const QString &filePath, QString *errorMessage = nullptr) const;
    const QVector<MeasurementRecord> &measurements() const;
    const QVector<FrequencyTensionMeasurementRecord> &frequencyTensionMeasurements() const;
    const QVector<SpectrumPoint> &spectrumPoints() const;

private:
    static constexpr qsizetype MaximumRows = 10000;
    Mode m_mode = Mode::Generic;
    QVector<MeasurementRecord> m_records;
    QVector<FrequencyTensionMeasurementRecord> m_frequencyTensionRecords;
    QVector<SpectrumPoint> m_spectrumPoints;
};

} // namespace gucds
