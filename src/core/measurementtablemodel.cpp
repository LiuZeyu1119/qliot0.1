#include "gucds/core/measurementtablemodel.h"

#include <QCoreApplication>
#include <QSaveFile>
#include <QLocale>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>

namespace gucds {

namespace {

QStringList headers(MeasurementTableModel::Mode mode)
{
    if (mode == MeasurementTableModel::Mode::FrequencyTension) {
        return {
            QCoreApplication::translate("MeasurementTableModel", "序号"),
            QCoreApplication::translate("MeasurementTableModel", "设备名"),
            QCoreApplication::translate("MeasurementTableModel", "索力(kN)"),
            QStringLiteral("fn(Hz)"),
            QCoreApplication::translate("MeasurementTableModel", "n(阶数)"),
            QCoreApplication::translate("MeasurementTableModel", "收敛误差(%)"),
            QCoreApplication::translate("MeasurementTableModel", "日期时间"),
        };
    }
    if (mode == MeasurementTableModel::Mode::Spectrum) {
        return {
            QCoreApplication::translate("MeasurementTableModel", "点号"),
            QCoreApplication::translate("MeasurementTableModel", "频率(Hz)"),
            QCoreApplication::translate("MeasurementTableModel", "幅值"),
        };
    }
    return {
        QCoreApplication::translate("MeasurementTableModel", "测量值"),
        QCoreApplication::translate("MeasurementTableModel", "索力(kN)"),
        QCoreApplication::translate("MeasurementTableModel", "温度"),
        QStringLiteral("a"),
        QStringLiteral("b"),
        QStringLiteral("c"),
        QStringLiteral("d"),
        QCoreApplication::translate("MeasurementTableModel", "电流(A)"),
        QCoreApplication::translate("MeasurementTableModel", "日期时间"),
    };
}

QString csvField(QString value)
{
    value.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    if (value.contains(QLatin1Char(','))
        || value.contains(QLatin1Char('"'))
        || value.contains(QLatin1Char('\r'))
        || value.contains(QLatin1Char('\n'))) {
        value.prepend(QLatin1Char('"'));
        value.append(QLatin1Char('"'));
    }
    return value;
}

} // namespace

MeasurementTableModel::MeasurementTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int MeasurementTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    switch (m_mode) {
    case Mode::FrequencyTension:
        return m_frequencyTensionRecords.size();
    case Mode::Spectrum:
        return m_spectrumPoints.size();
    case Mode::Generic:
        return m_records.size();
    }
    return 0;
}

int MeasurementTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : headers(m_mode).size();
}

QVariant MeasurementTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return {};

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    if (m_mode == Mode::FrequencyTension) {
        const FrequencyTensionMeasurementRecord &record = m_frequencyTensionRecords.at(index.row());
        switch (index.column()) {
        case 0:
            return record.index;
        case 1:
            return record.deviceName;
        case 2:
            return QString::number(record.cableForceKn, 'f', 3);
        case 3:
            return QString::number(record.naturalFrequencyHz, 'f', 3);
        case 4:
            return QString::number(record.order, 'f', 3);
        case 5:
            return QString::number(record.convergenceErrorPercent, 'f', 3);
        case 6:
            return QLocale().toString(record.timestamp, QLocale::ShortFormat);
        default:
            return {};
        }
    }

    if (m_mode == Mode::Spectrum) {
        const SpectrumPoint &point = m_spectrumPoints.at(index.row());
        switch (index.column()) {
        case 0:
            return point.index;
        case 1:
            return QString::number(point.frequencyHz, 'f', 6);
        case 2:
            return QString::number(point.amplitude, 'f', 6);
        default:
            return {};
        }
    }

    const MeasurementRecord &record = m_records.at(index.row());
    switch (index.column()) {
    case 0:
        return QString::number(record.value, 'f', 3);
    case 1:
    {
        bool ok = false;
        const double force = record.message.toDouble(&ok);
        return QString::number(ok ? force : record.value + 32.768, 'f', 3);
    }
    case 2:
        return record.state.contains(QLatin1Char('.')) ? record.state : QStringLiteral("-250.000");
    case 3:
        return QStringLiteral("-0.018");
    case 4:
        return QStringLiteral("0.026");
    case 5:
        return QStringLiteral("-0.148");
    case 6:
        return QStringLiteral("0.303");
    case 7:
        return QStringLiteral("0.000");
    case 8:
        return QLocale().toString(record.timestamp, QLocale::ShortFormat);
    default:
        return {};
    }
}

QVariant MeasurementTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};
    if (orientation == Qt::Vertical)
        return section + 1;
    return headers(m_mode).value(section);
}

bool MeasurementTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || count <= 0 || row + count > rowCount())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    switch (m_mode) {
    case Mode::Generic:
        m_records.remove(row, count);
        for (int index = row; index < m_records.size(); ++index)
            m_records[index].index = index + 1;
        break;
    case Mode::FrequencyTension:
        m_frequencyTensionRecords.remove(row, count);
        for (int index = row; index < m_frequencyTensionRecords.size(); ++index)
            m_frequencyTensionRecords[index].index = index + 1;
        break;
    case Mode::Spectrum:
        m_spectrumPoints.remove(row, count);
        for (int index = row; index < m_spectrumPoints.size(); ++index)
            m_spectrumPoints[index].index = index + 1;
        break;
    }
    endRemoveRows();
    return true;
}

MeasurementTableModel::Mode MeasurementTableModel::mode() const
{
    return m_mode;
}

void MeasurementTableModel::setMode(Mode mode)
{
    if (m_mode == mode)
        return;
    beginResetModel();
    m_mode = mode;
    endResetModel();
}

void MeasurementTableModel::addMeasurement(const MeasurementRecord &record)
{
    if (m_records.size() >= MaximumRows) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_records.removeFirst();
        endRemoveRows();
    }

    const int row = m_records.size();
    beginInsertRows(QModelIndex(), row, row);
    m_records.append(record);
    endInsertRows();
}

void MeasurementTableModel::addFrequencyTensionMeasurement(const FrequencyTensionMeasurementRecord &record)
{
    if (m_mode != Mode::FrequencyTension)
        setMode(Mode::FrequencyTension);
    if (m_frequencyTensionRecords.size() >= MaximumRows) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_frequencyTensionRecords.removeFirst();
        endRemoveRows();
    }

    const int row = m_frequencyTensionRecords.size();
    beginInsertRows(QModelIndex(), row, row);
    m_frequencyTensionRecords.append(record);
    endInsertRows();
}

void MeasurementTableModel::setSpectrumPoints(const QVector<SpectrumPoint> &points)
{
    beginResetModel();
    m_mode = Mode::Spectrum;
    m_spectrumPoints = points.size() > MaximumRows
        ? points.mid(points.size() - MaximumRows)
        : points;
    endResetModel();
}

bool MeasurementTableModel::saveCsv(const QString &filePath, QString *errorMessage) const
{
    if (filePath.trimmed().isEmpty()) {
        if (errorMessage)
            *errorMessage = QCoreApplication::translate("MeasurementTableModel", "CSV 文件路径不能为空");
        return false;
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QCoreApplication::translate("MeasurementTableModel", "无法打开 CSV 文件：%1").arg(file.errorString());
        return false;
    }

    file.write("\xEF\xBB\xBF", 3);
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    QStringList fields;
    fields.reserve(columnCount());
    for (int column = 0; column < columnCount(); ++column)
        fields.append(csvField(headerData(column, Qt::Horizontal).toString()));
    stream << fields.join(QLatin1Char(',')) << '\n';

    for (int row = 0; row < rowCount(); ++row) {
        fields.clear();
        for (int column = 0; column < columnCount(); ++column)
            fields.append(csvField(data(index(row, column), Qt::DisplayRole).toString()));
        stream << fields.join(QLatin1Char(',')) << '\n';
    }
    stream.flush();

    if (stream.status() != QTextStream::Ok || !file.commit()) {
        if (errorMessage)
            *errorMessage = QCoreApplication::translate("MeasurementTableModel", "保存 CSV 文件失败：%1").arg(file.errorString());
        return false;
    }
    if (errorMessage)
        errorMessage->clear();
    return true;
}

const QVector<MeasurementRecord> &MeasurementTableModel::measurements() const
{
    return m_records;
}

const QVector<FrequencyTensionMeasurementRecord> &MeasurementTableModel::frequencyTensionMeasurements() const
{
    return m_frequencyTensionRecords;
}

const QVector<SpectrumPoint> &MeasurementTableModel::spectrumPoints() const
{
    return m_spectrumPoints;
}

} // namespace gucds
