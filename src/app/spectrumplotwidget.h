#pragma once

#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QWidget>

class SpectrumPlotWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SpectrumPlotWidget)

public:
    enum class Mode
    {
        Generic,
        FrequencyTension,
        Spectrum,
    };
    Q_ENUM(Mode)

    explicit SpectrumPlotWidget(QWidget *parent = nullptr);

    void setValues(const QVector<double> &values);
    void appendValue(double value);
    void setFrequencyTensionPoints(const QVector<QPointF> &points);
    void setSpectrumPoints(const QVector<QPointF> &points);
    Mode mode() const;
    QString xAxisLabel() const;
    QString yAxisLabel() const;
    QVector<QPointF> seriesPoints() const;
    QRectF axisBounds() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setSeries(Mode mode,
                   const QVector<QPointF> &points,
                   const QString &title,
                   const QString &xAxisLabel,
                   const QString &yAxisLabel);

    static constexpr qsizetype MaximumVisibleValues = 4096;
    Mode m_mode = Mode::Generic;
    QVector<QPointF> m_points;
    QString m_title;
    QString m_xAxisLabel;
    QString m_yAxisLabel;
};
