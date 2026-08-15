#pragma once

#include "gucds/core/records.h"

#include <QAbstractTableModel>
#include <QVector>

namespace gucds {

class BusDeviceTableModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit BusDeviceTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addDevice(const BusDeviceRecord &record);
    bool updateDevice(int row, const BusDeviceRecord &record);
    bool removeDevice(int row);
    void setRecords(const QVector<BusDeviceRecord> &records);
    void updateLastResponse(const QString &responseCode);
    const QVector<BusDeviceRecord> &records() const;
    BusDeviceRecord recordAt(int row) const;

private:
    QVector<BusDeviceRecord> m_records;
};

} // namespace gucds
