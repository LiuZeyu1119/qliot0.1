#pragma once

#include "gucds/core/canframe.h"

#include <QAbstractTableModel>
#include <QVector>

namespace gucds {

class CanFrameTableModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        TimeColumn,
        DirectionColumn,
        TypeColumn,
        IdColumn,
        DlcColumn,
        DataColumn,
        AsciiColumn,
        ColumnCount,
    };

    explicit CanFrameTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    const CanFrame &frameAt(int row) const;
    int maximumFrameCount() const;
    void setMaximumFrameCount(int count);

public slots:
    void appendFrame(const gucds::CanFrame &frame);
    void clear();

private:
    QVector<CanFrame> m_frames;
    int m_maximumFrameCount = 5000;
};

} // namespace gucds
