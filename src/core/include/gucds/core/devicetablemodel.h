#pragma once

#include "gucds/core/records.h"

#include <QAbstractTableModel>
#include <QVector>

namespace gucds {

class DeviceTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DeviceTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setRecords(const QVector<DeviceRecord> &records);
    void clear();
    void addRecord(const DeviceRecord &record);
    bool setRecordAt(int row, const DeviceRecord &record);
    bool removeRecordAt(int row);
    bool moveRecord(int fromRow, int toRow);
    DeviceRecord recordAt(int row) const;
    bool containsEquivalent(const DeviceRecord &record) const;
    const QVector<DeviceRecord> &records() const;

private:
    QVector<DeviceRecord> m_records;
};

} // namespace gucds
