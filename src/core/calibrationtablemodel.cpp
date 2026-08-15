#include "gucds/core/calibrationtablemodel.h"

#include <QCoreApplication>
#include <QLocale>
#include <QStringList>

namespace gucds {

namespace {

QStringList headers()
{
    return {
        QCoreApplication::translate("CalibrationTableModel", "曲线名"),
        QCoreApplication::translate("CalibrationTableModel", "点号"),
        QCoreApplication::translate("CalibrationTableModel", "测量值"),
        QCoreApplication::translate("CalibrationTableModel", "标定值"),
        QCoreApplication::translate("CalibrationTableModel", "温度(℃)"),
        QCoreApplication::translate("CalibrationTableModel", "日期时间"),
    };
}

} // namespace

CalibrationTableModel::CalibrationTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int CalibrationTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

int CalibrationTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : headers().size();
}

QVariant CalibrationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size())
        return {};
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    const CalibrationRecord &record = m_records.at(index.row());
    switch (index.column()) {
    case 0:
        return record.curveName;
    case 1:
        return record.point;
    case 2:
        return QString::number(record.measuredValue, 'f', 3);
    case 3:
        return QString::number(record.referenceValue, 'f', 3);
    case 4:
        return QString::number(record.temperature, 'f', 1);
    case 5:
        return QLocale().toString(record.timestamp, QLocale::ShortFormat);
    default:
        return {};
    }
}

QVariant CalibrationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};
    if (orientation == Qt::Vertical)
        return section + 1;
    return headers().value(section);
}

void CalibrationTableModel::addPoint(const CalibrationRecord &record)
{
    const int row = m_records.size();
    beginInsertRows(QModelIndex(), row, row);
    m_records.append(record);
    endInsertRows();
}

bool CalibrationTableModel::updatePoint(int row, const CalibrationRecord &record)
{
    if (row < 0 || row >= m_records.size())
        return false;
    m_records[row] = record;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1), {Qt::DisplayRole});
    return true;
}

bool CalibrationTableModel::removePoint(int row)
{
    if (row < 0 || row >= m_records.size())
        return false;
    beginRemoveRows(QModelIndex(), row, row);
    m_records.removeAt(row);
    endRemoveRows();
    return true;
}

void CalibrationTableModel::removeLast()
{
    if (m_records.isEmpty())
        return;
    const int row = m_records.size() - 1;
    beginRemoveRows(QModelIndex(), row, row);
    m_records.removeLast();
    endRemoveRows();
}

void CalibrationTableModel::clear()
{
    beginResetModel();
    m_records.clear();
    endResetModel();
}

void CalibrationTableModel::setRecords(const QVector<CalibrationRecord> &records)
{
    beginResetModel();
    m_records = records;
    endResetModel();
}

const QVector<CalibrationRecord> &CalibrationTableModel::records() const
{
    return m_records;
}

CalibrationRecord CalibrationTableModel::recordAt(int row) const
{
    return row >= 0 && row < m_records.size() ? m_records.at(row) : CalibrationRecord{};
}

} // namespace gucds
