#pragma once

#include "gucds/core/records.h"

#include <QAbstractTableModel>
#include <QVector>

namespace gucds {

class CalibrationTableModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CalibrationTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addPoint(const CalibrationRecord &record);
    bool updatePoint(int row, const CalibrationRecord &record);
    bool removePoint(int row);
    void removeLast();
    void clear();
    void setRecords(const QVector<CalibrationRecord> &records);
    const QVector<CalibrationRecord> &records() const;
    CalibrationRecord recordAt(int row) const;

private:
    QVector<CalibrationRecord> m_records;
};

} // namespace gucds
