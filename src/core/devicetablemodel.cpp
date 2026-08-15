#include "gucds/core/devicetablemodel.h"

#include <QCoreApplication>
#include <QStringList>

namespace gucds {

namespace {

QStringList headers()
{
    return {
        QCoreApplication::translate("DeviceTableModel", "设备名称"),
        QCoreApplication::translate("DeviceTableModel", "设备类别"),
        QCoreApplication::translate("DeviceTableModel", "规格型号"),
        QCoreApplication::translate("DeviceTableModel", "数据1"),
        QCoreApplication::translate("DeviceTableModel", "数据2"),
        QCoreApplication::translate("DeviceTableModel", "数据3"),
        QCoreApplication::translate("DeviceTableModel", "数据4"),
        QCoreApplication::translate("DeviceTableModel", "数据5"),
    };
}

} // namespace

DeviceTableModel::DeviceTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int DeviceTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

int DeviceTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : headers().size();
}

QVariant DeviceTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size())
        return {};

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    const DeviceRecord &record = m_records.at(index.row());
    switch (index.column()) {
    case 0:
        return record.name;
    case 1:
        return record.category;
    case 2:
        return record.model;
    case 3:
        return record.data1.isEmpty() ? record.protocol : record.data1;
    case 4:
        return record.data2.isEmpty() ? record.calibration : record.data2;
    case 5:
        return record.data3.isEmpty() ? QCoreApplication::translate("DeviceTableModel", "未定义") : record.data3;
    case 6:
        return record.data4.isEmpty() ? QCoreApplication::translate("DeviceTableModel", "未定义") : record.data4;
    case 7:
        return record.data5.isEmpty() ? QCoreApplication::translate("DeviceTableModel", "未定义") : record.data5;
    default:
        return {};
    }
}

QVariant DeviceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};
    if (orientation == Qt::Vertical)
        return section + 1;
    return headers().value(section);
}

void DeviceTableModel::setRecords(const QVector<DeviceRecord> &records)
{
    beginResetModel();
    m_records = records;
    endResetModel();
}

void DeviceTableModel::clear()
{
    if (m_records.isEmpty())
        return;

    beginResetModel();
    m_records.clear();
    endResetModel();
}

void DeviceTableModel::addRecord(const DeviceRecord &record)
{
    const int row = m_records.size();
    beginInsertRows(QModelIndex(), row, row);
    m_records.append(record);
    endInsertRows();
}

bool DeviceTableModel::setRecordAt(int row, const DeviceRecord &record)
{
    if (row < 0 || row >= m_records.size())
        return false;

    m_records[row] = record;
    const QModelIndex first = index(row, 0);
    const QModelIndex last = index(row, columnCount() - 1);
    emit dataChanged(first, last, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

bool DeviceTableModel::removeRecordAt(int row)
{
    if (row < 0 || row >= m_records.size())
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_records.removeAt(row);
    endRemoveRows();
    return true;
}

bool DeviceTableModel::moveRecord(int fromRow, int toRow)
{
    if (fromRow < 0 || fromRow >= m_records.size() || toRow < 0 || toRow >= m_records.size() || fromRow == toRow)
        return false;

    const int destinationRow = toRow > fromRow ? toRow + 1 : toRow;
    if (!beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), destinationRow))
        return false;

    m_records.move(fromRow, toRow);
    endMoveRows();
    return true;
}

DeviceRecord DeviceTableModel::recordAt(int row) const
{
    if (row < 0 || row >= m_records.size())
        return {};
    return m_records.at(row);
}

bool DeviceTableModel::containsEquivalent(const DeviceRecord &record) const
{
    for (const DeviceRecord &candidate : m_records) {
        if (candidate.name.trimmed() == record.name.trimmed()
            && candidate.category.trimmed() == record.category.trimmed()
            && candidate.model.trimmed() == record.model.trimmed()) {
            return true;
        }
    }
    return false;
}

const QVector<DeviceRecord> &DeviceTableModel::records() const
{
    return m_records;
}

} // namespace gucds
