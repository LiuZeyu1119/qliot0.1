#pragma once

#include "gucds/core/records.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QVector>

namespace gucds {

class ProductTableModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ProductTableModel)

public:
    enum Column : int
    {
        NameColumn,
        CategoryColumn,
        ModelColumn,
        DataColumn,
        CommunicationColumn,
        ParameterColumn,
        CalibrationColumn,
        ColumnCount,
    };

    explicit ProductTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setRecords(const QVector<DeviceRecord> &records);
    void addRecord(const DeviceRecord &record);
    bool updateRecord(const DeviceRecord &record);
    bool removeRecord(qint64 databaseId);
    DeviceRecord recordAt(int row) const;
    int rowForDatabaseId(qint64 databaseId) const;
    const QVector<DeviceRecord> &records() const;

private:
    void rebuildRowIndex(int firstRow = 0);

    QVector<DeviceRecord> m_records;
    QHash<qint64, int> m_rowByDatabaseId;
};

} // namespace gucds
