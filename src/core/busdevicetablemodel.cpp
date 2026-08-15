#include "gucds/core/busdevicetablemodel.h"

#include <QCoreApplication>
#include <QStringList>

namespace gucds {

namespace {

QStringList headers()
{
    return {
        QCoreApplication::translate("BusDeviceTableModel", "序号"),
        QCoreApplication::translate("BusDeviceTableModel", "传感器名"),
        QCoreApplication::translate("BusDeviceTableModel", "规格型号"),
        QCoreApplication::translate("BusDeviceTableModel", "信道"),
        QCoreApplication::translate("BusDeviceTableModel", "组号"),
        QCoreApplication::translate("BusDeviceTableModel", "地址"),
        QCoreApplication::translate("BusDeviceTableModel", "数据数"),
        QCoreApplication::translate("BusDeviceTableModel", "应答码"),
    };
}

} // namespace

BusDeviceTableModel::BusDeviceTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int BusDeviceTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

int BusDeviceTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : headers().size();
}

QVariant BusDeviceTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size())
        return {};
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    const BusDeviceRecord &record = m_records.at(index.row());
    switch (index.column()) {
    case 0:
        return record.index;
    case 1:
        return record.sensorName;
    case 2:
        return record.model;
    case 3:
        return record.channel;
    case 4:
        return record.group;
    case 5:
        return record.address;
    case 6:
        return record.dataCount;
    case 7:
        return record.responseCode;
    default:
        return {};
    }
}

QVariant BusDeviceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};
    if (orientation == Qt::Vertical)
        return section + 1;
    return headers().value(section);
}

void BusDeviceTableModel::addDevice(const BusDeviceRecord &record)
{
    const int row = m_records.size();
    beginInsertRows(QModelIndex(), row, row);
    m_records.append(record);
    endInsertRows();
}

bool BusDeviceTableModel::updateDevice(int row, const BusDeviceRecord &record)
{
    if (row < 0 || row >= m_records.size())
        return false;
    m_records[row] = record;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1), {Qt::DisplayRole});
    return true;
}

bool BusDeviceTableModel::removeDevice(int row)
{
    if (row < 0 || row >= m_records.size())
        return false;
    beginRemoveRows(QModelIndex(), row, row);
    m_records.removeAt(row);
    endRemoveRows();
    return true;
}

void BusDeviceTableModel::setRecords(const QVector<BusDeviceRecord> &records)
{
    beginResetModel();
    m_records = records;
    endResetModel();
}

void BusDeviceTableModel::updateLastResponse(const QString &responseCode)
{
    if (m_records.isEmpty())
        return;
    m_records.last().responseCode = responseCode;
    const QModelIndex topLeft = index(m_records.size() - 1, 7);
    emit dataChanged(topLeft, topLeft, {Qt::DisplayRole});
}

const QVector<BusDeviceRecord> &BusDeviceTableModel::records() const
{
    return m_records;
}

BusDeviceRecord BusDeviceTableModel::recordAt(int row) const
{
    return row >= 0 && row < m_records.size() ? m_records.at(row) : BusDeviceRecord{};
}

} // namespace gucds
