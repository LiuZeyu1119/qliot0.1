#include "gucds/core/canframetablemodel.h"

#include <QCoreApplication>
#include <QDateTime>

#include <algorithm>

namespace gucds {

CanFrameTableModel::CanFrameTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int CanFrameTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_frames.size();
}

int CanFrameTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant CanFrameTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_frames.size())
        return {};

    const CanFrame &frame = m_frames.at(index.row());
    if (role == Qt::TextAlignmentRole) {
        if (index.column() == TimeColumn || index.column() == DirectionColumn
            || index.column() == TypeColumn || index.column() == IdColumn
            || index.column() == DlcColumn) {
            return Qt::AlignCenter;
        }
        return QVariant::fromValue(Qt::AlignVCenter | Qt::AlignLeft);
    }
    if (role != Qt::DisplayRole)
        return {};

    switch (index.column()) {
    case TimeColumn:
        return QDateTime::fromMSecsSinceEpoch(frame.wallClockMs).toString(QStringLiteral("HH:mm:ss.zzz"));
    case DirectionColumn:
        return frame.transmitted
            ? QCoreApplication::translate("CanFrameTableModel", "发送")
            : QCoreApplication::translate("CanFrameTableModel", "接收");
    case TypeColumn:
        return QStringLiteral("%1 %2")
            .arg(frame.extended ? QStringLiteral("EXT") : QStringLiteral("STD"),
                 frame.remote ? QStringLiteral("RTR") : QStringLiteral("DATA"));
    case IdColumn:
        return QStringLiteral("0x")
            + QStringLiteral("%1")
                  .arg(frame.id, frame.extended ? 8 : 3, 16, QLatin1Char('0')).toUpper();
    case DlcColumn:
        return frame.dlc;
    case DataColumn:
        return frame.payload.toHex(' ').toUpper();
    case AsciiColumn: {
        QByteArray ascii = frame.payload;
        std::replace_if(ascii.begin(), ascii.end(), [](char value) {
            const uchar byte = static_cast<uchar>(value);
            return byte < 0x20U || byte > 0x7EU;
        }, '.');
        return QString::fromLatin1(ascii);
    }
    default:
        return {};
    }
}

QVariant CanFrameTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    switch (section) {
    case TimeColumn: return QCoreApplication::translate("CanFrameTableModel", "时间");
    case DirectionColumn: return QCoreApplication::translate("CanFrameTableModel", "方向");
    case TypeColumn: return QCoreApplication::translate("CanFrameTableModel", "帧类型");
    case IdColumn: return QStringLiteral("ID");
    case DlcColumn: return QStringLiteral("DLC");
    case DataColumn: return QCoreApplication::translate("CanFrameTableModel", "数据 (HEX)");
    case AsciiColumn: return QStringLiteral("ASCII");
    default: return {};
    }
}

const CanFrame &CanFrameTableModel::frameAt(int row) const
{
    Q_ASSERT(row >= 0 && row < m_frames.size());
    return m_frames.at(row);
}

int CanFrameTableModel::maximumFrameCount() const
{
    return m_maximumFrameCount;
}

void CanFrameTableModel::setMaximumFrameCount(int count)
{
    m_maximumFrameCount = std::max(1, count);
    if (m_frames.size() <= m_maximumFrameCount)
        return;

    const int removeCount = m_frames.size() - m_maximumFrameCount;
    beginRemoveRows({}, 0, removeCount - 1);
    m_frames.remove(0, removeCount);
    endRemoveRows();
}

void CanFrameTableModel::appendFrame(const CanFrame &frame)
{
    if (m_frames.size() >= m_maximumFrameCount) {
        beginRemoveRows({}, 0, 0);
        m_frames.removeFirst();
        endRemoveRows();
    }
    const int row = m_frames.size();
    beginInsertRows({}, row, row);
    m_frames.append(frame);
    endInsertRows();
}

void CanFrameTableModel::clear()
{
    if (m_frames.isEmpty())
        return;
    beginResetModel();
    m_frames.clear();
    endResetModel();
}

} // namespace gucds
