#include "gucds/core/producttablemodel.h"

#include "gucds/core/deviceparameter.h"

#include <QCoreApplication>
#include <QStringList>

namespace gucds {

namespace {

bool isEnabled(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    return normalized == QStringLiteral("开") || normalized == QStringLiteral("是")
        || normalized == QStringLiteral("true") || normalized == QStringLiteral("1");
}

QStringList dataNames(const DeviceRecord &record)
{
    QStringList names;
    for (const QString &value : {record.data1, record.data2, record.data3, record.data4, record.data5}) {
        const QString trimmed = value.trimmed();
        if (!trimmed.isEmpty() && trimmed != QStringLiteral("未定义"))
            names.append(trimmed);
    }
    return names;
}

QString communicationSummary(const DeviceRecord &record)
{
    QStringList names;
    if (isEnabled(record.modbus))
        names.append(QStringLiteral("Modbus"));
    if (isEnabled(record.lora))
        names.append(QStringLiteral("LoRa"));
    if (isEnabled(record.dtu))
        names.append(QStringLiteral("DTU"));
    return names.isEmpty() ? QCoreApplication::translate("ProductTableModel", "无") : names.join(QStringLiteral(" / "));
}

int parameterCount(const DeviceRecord &record)
{
    int count = 0;
    for (int index = 1; index <= 5; ++index) {
        if (parseDeviceParameterDefinition(deviceParameterDefinition(record, index)).isValid())
            ++count;
    }
    return count;
}

} // namespace

ProductTableModel::ProductTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ProductTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

int ProductTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant ProductTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size())
        return {};

    const DeviceRecord &record = m_records.at(index.row());
    if (role == Qt::UserRole)
        return record.databaseId;
    if (role == Qt::ToolTipRole) {
        return QCoreApplication::translate("ProductTableModel", "%1\n%2\n%3\n数据项：%4")
            .arg(record.name,
                 record.category,
                 record.model,
                 dataNames(record).join(QCoreApplication::translate("ProductTableModel", "、")));
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    switch (index.column()) {
    case NameColumn:
        return record.name;
    case CategoryColumn:
        return record.category;
    case ModelColumn:
        return record.model;
    case DataColumn:
        return dataNames(record).join(QCoreApplication::translate("ProductTableModel", "、"));
    case CommunicationColumn:
        return communicationSummary(record);
    case ParameterColumn:
        return parameterCount(record);
    case CalibrationColumn:
        return record.calibrationPoints;
    default:
        return {};
    }
}

QVariant ProductTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};
    if (orientation == Qt::Vertical)
        return section + 1;

    const QStringList headers = {
        QCoreApplication::translate("ProductTableModel", "产品名称"),
        QCoreApplication::translate("ProductTableModel", "产品类别"),
        QCoreApplication::translate("ProductTableModel", "规格型号"),
        QCoreApplication::translate("ProductTableModel", "数据项"),
        QCoreApplication::translate("ProductTableModel", "通信"),
        QCoreApplication::translate("ProductTableModel", "参数数"),
        QCoreApplication::translate("ProductTableModel", "标定点"),
    };
    return headers.value(section);
}

void ProductTableModel::setRecords(const QVector<DeviceRecord> &records)
{
    beginResetModel();
    m_records = records;
    rebuildRowIndex();
    endResetModel();
}

void ProductTableModel::addRecord(const DeviceRecord &record)
{
    const int row = m_records.size();
    beginInsertRows(QModelIndex(), row, row);
    m_records.append(record);
    if (record.databaseId > 0)
        m_rowByDatabaseId.insert(record.databaseId, row);
    endInsertRows();
}

bool ProductTableModel::updateRecord(const DeviceRecord &record)
{
    const int row = rowForDatabaseId(record.databaseId);
    if (row < 0)
        return false;

    m_records[row] = record;
    emit dataChanged(index(row, 0),
                     index(row, ColumnCount - 1),
                     {Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole, Qt::UserRole});
    return true;
}

bool ProductTableModel::removeRecord(qint64 databaseId)
{
    const int row = rowForDatabaseId(databaseId);
    if (row < 0)
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_rowByDatabaseId.remove(databaseId);
    m_records.removeAt(row);
    rebuildRowIndex(row);
    endRemoveRows();
    return true;
}

DeviceRecord ProductTableModel::recordAt(int row) const
{
    if (row < 0 || row >= m_records.size())
        return {};
    return m_records.at(row);
}

int ProductTableModel::rowForDatabaseId(qint64 databaseId) const
{
    return m_rowByDatabaseId.value(databaseId, -1);
}

const QVector<DeviceRecord> &ProductTableModel::records() const
{
    return m_records;
}

void ProductTableModel::rebuildRowIndex(int firstRow)
{
    if (firstRow <= 0) {
        m_rowByDatabaseId.clear();
        firstRow = 0;
    }
    for (int row = firstRow; row < m_records.size(); ++row) {
        const qint64 databaseId = m_records.at(row).databaseId;
        if (databaseId > 0)
            m_rowByDatabaseId.insert(databaseId, row);
    }
}

} // namespace gucds
