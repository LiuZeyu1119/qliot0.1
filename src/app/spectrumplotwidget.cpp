#include "spectrumplotwidget.h"

#include <QLocale>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace {

double niceStep(double span, int targetIntervals)
{
    if (!(span > 0.0) || targetIntervals < 1)
        return 1.0;
    const double rawStep = span / double(targetIntervals);
    const double magnitude = std::pow(10.0, std::floor(std::log10(rawStep)));
    const double normalized = rawStep / magnitude;
    const double factor = normalized < 1.5 ? 1.0
        : normalized < 3.0          ? 2.0
        : normalized < 7.0          ? 5.0
                                     : 10.0;
    return factor * magnitude;
}

int precisionForStep(double step)
{
    if (!(step > 0.0) || step >= 1.0)
        return 0;
    return (std::min)(6, (std::max)(1, int(std::ceil(-std::log10(step))) + 1));
}

QString axisNumber(double value, double step)
{
    if (std::abs(value) < step * 1e-6)
        value = 0.0;
    return QLocale().toString(value, 'f', precisionForStep(step));
}

struct AxisScale
{
    double minimumX = 0.0;
    double maximumX = 1.0;
    double minimumY = 0.0;
    double maximumY = 1.0;
    double xStep = 1.0;
    double yStep = 1.0;
};

AxisScale calculateAxisScale(const QVector<QPointF> &points, SpectrumPlotWidget::Mode mode)
{
    if (points.isEmpty())
        return {};

    double minimumX = points.first().x();
    double maximumX = minimumX;
    double minimumY = points.first().y();
    double maximumY = minimumY;
    for (const QPointF &point : points) {
        minimumX = (std::min)(minimumX, point.x());
        maximumX = (std::max)(maximumX, point.x());
        minimumY = (std::min)(minimumY, point.y());
        maximumY = (std::max)(maximumY, point.y());
    }

    if (qFuzzyCompare(minimumX, maximumX)) {
        minimumX -= 0.5;
        maximumX += 0.5;
    }
    if (mode == SpectrumPlotWidget::Mode::Spectrum) {
        minimumX = (std::min)(minimumX, 0.0);
        minimumY = (std::min)(minimumY, 0.0);
        maximumY = (std::max)(maximumY, 0.0);
    } else if (qFuzzyCompare(minimumY, maximumY)) {
        const double padding = (std::max)(1.0, std::abs(minimumY) * 0.05);
        minimumY -= padding;
        maximumY += padding;
    }

    AxisScale scale;
    scale.xStep = niceStep(maximumX - minimumX, 10);
    scale.yStep = niceStep(maximumY - minimumY, 7);
    scale.minimumX = mode == SpectrumPlotWidget::Mode::Spectrum
        ? 0.0
        : std::floor(minimumX / scale.xStep) * scale.xStep;
    scale.maximumX = std::ceil(maximumX / scale.xStep) * scale.xStep;
    scale.minimumY = std::floor(minimumY / scale.yStep) * scale.yStep;
    scale.maximumY = std::ceil(maximumY / scale.yStep) * scale.yStep;
    if (!(scale.maximumX > scale.minimumX))
        scale.maximumX = scale.minimumX + scale.xStep;
    if (!(scale.maximumY > scale.minimumY))
        scale.maximumY = scale.minimumY + scale.yStep;
    return scale;
}

} // namespace

SpectrumPlotWidget::SpectrumPlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_title(tr("数据曲线"))
    , m_xAxisLabel(tr("测量序号"))
    , m_yAxisLabel(tr("测量值"))
{
    setMinimumHeight(170);
    setAutoFillBackground(true);
}

void SpectrumPlotWidget::setValues(const QVector<double> &values)
{
    QVector<QPointF> points;
    points.reserve(values.size());
    for (qsizetype index = 0; index < values.size(); ++index)
        points.append(QPointF(double(index + 1), values.at(index)));
    setSeries(Mode::Generic, points, tr("数据曲线"), tr("测量序号"), tr("测量值"));
}

void SpectrumPlotWidget::appendValue(double value)
{
    if (m_mode != Mode::Generic)
        setSeries(Mode::Generic, {}, tr("数据曲线"), tr("测量序号"), tr("测量值"));
    if (m_points.size() >= MaximumVisibleValues)
        m_points.removeFirst();
    const double x = m_points.isEmpty() ? 1.0 : m_points.constLast().x() + 1.0;
    m_points.append(QPointF(x, value));
    update();
}

void SpectrumPlotWidget::setFrequencyTensionPoints(const QVector<QPointF> &points)
{
    setSeries(Mode::FrequencyTension, points, tr("索力趋势"), tr("测量序号"), tr("索力(kN)"));
}

void SpectrumPlotWidget::setSpectrumPoints(const QVector<QPointF> &points)
{
    setSeries(Mode::Spectrum, points, tr("频谱曲线"), tr("频率(Hz)"), tr("幅值"));
}

SpectrumPlotWidget::Mode SpectrumPlotWidget::mode() const
{
    return m_mode;
}

QString SpectrumPlotWidget::xAxisLabel() const
{
    return m_xAxisLabel;
}

QString SpectrumPlotWidget::yAxisLabel() const
{
    return m_yAxisLabel;
}

QVector<QPointF> SpectrumPlotWidget::seriesPoints() const
{
    return m_points;
}

QRectF SpectrumPlotWidget::axisBounds() const
{
    const AxisScale scale = calculateAxisScale(m_points, m_mode);
    return QRectF(scale.minimumX,
                  scale.minimumY,
                  scale.maximumX - scale.minimumX,
                  scale.maximumY - scale.minimumY);
}

void SpectrumPlotWidget::setSeries(Mode mode,
                                   const QVector<QPointF> &points,
                                   const QString &title,
                                   const QString &xAxisLabel,
                                   const QString &yAxisLabel)
{
    m_mode = mode;
    m_points = points.size() > MaximumVisibleValues
        ? points.mid(points.size() - MaximumVisibleValues)
        : points;
    m_title = title;
    m_xAxisLabel = xAxisLabel;
    m_yAxisLabel = yAxisLabel;
    update();
}

void SpectrumPlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(248, 252, 255));

    const QRect plot = rect().adjusted(78, 34, -24, -54);
    if (plot.width() < 10 || plot.height() < 10)
        return;

    painter.setPen(QColor(45, 65, 89));
    painter.drawText(QRect(plot.left(), 4, plot.width(), 24), Qt::AlignLeft | Qt::AlignVCenter, m_title);

    if (m_points.isEmpty()) {
        painter.setPen(QPen(QColor(180, 196, 215), 1));
        painter.drawRect(plot);
        painter.setPen(QColor(120, 130, 140));
        painter.drawText(plot, Qt::AlignCenter, tr("等待测量数据"));
        painter.drawText(QRect(plot.left(), plot.bottom() + 28, plot.width(), 22), Qt::AlignCenter, m_xAxisLabel);
        painter.save();
        painter.translate(18, plot.center().y());
        painter.rotate(-90.0);
        painter.drawText(QRect(-plot.height() / 2, -12, plot.height(), 24), Qt::AlignCenter, m_yAxisLabel);
        painter.restore();
        return;
    }

    const AxisScale scale = calculateAxisScale(m_points, m_mode);
    const double xStep = scale.xStep;
    const double yStep = scale.yStep;
    const double axisMinX = scale.minimumX;
    const double axisMaxX = scale.maximumX;
    const double axisMinY = scale.minimumY;
    const double axisMaxY = scale.maximumY;

    painter.setPen(QPen(QColor(180, 196, 215), 1));
    painter.drawRect(plot);

    const double xSpan = axisMaxX - axisMinX;
    const double ySpan = axisMaxY - axisMinY;
    for (double value = axisMinX; value <= axisMaxX + xStep * 0.25; value += xStep) {
        const int x = qRound(plot.left() + (value - axisMinX) / xSpan * plot.width());
        painter.setPen(QPen(QColor(205, 216, 230), 1));
        painter.drawLine(x, plot.top(), x, plot.bottom());
        painter.setPen(QColor(65, 78, 94));
        painter.drawText(QRect(x - 36, plot.bottom() + 4, 72, 20), Qt::AlignHCenter | Qt::AlignTop, axisNumber(value, xStep));
    }
    for (double value = axisMinY; value <= axisMaxY + yStep * 0.25; value += yStep) {
        const int y = qRound(plot.bottom() - (value - axisMinY) / ySpan * plot.height());
        const bool zeroLine = std::abs(value) < yStep * 1e-6;
        painter.setPen(QPen(zeroLine ? QColor(132, 153, 176) : QColor(205, 216, 230),
                            zeroLine ? 2 : 1));
        painter.drawLine(plot.left(), y, plot.right(), y);
        painter.setPen(QColor(65, 78, 94));
        painter.drawText(QRect(4, y - 10, plot.left() - 12, 20), Qt::AlignRight | Qt::AlignVCenter, axisNumber(value, yStep));
    }

    painter.setPen(QColor(45, 65, 89));
    painter.drawText(QRect(plot.left(), plot.bottom() + 28, plot.width(), 22), Qt::AlignCenter, m_xAxisLabel);
    painter.save();
    painter.translate(18, plot.center().y());
    painter.rotate(-90.0);
    painter.drawText(QRect(-plot.height() / 2, -12, plot.height(), 24), Qt::AlignCenter, m_yAxisLabel);
    painter.restore();

    QPainterPath path;
    for (qsizetype index = 0; index < m_points.size(); ++index) {
        const QPointF &sample = m_points.at(index);
        const QPointF point(plot.left() + (sample.x() - axisMinX) / xSpan * plot.width(),
                            plot.bottom() - (sample.y() - axisMinY) / ySpan * plot.height());
        if (index == 0)
            path.moveTo(point);
        else
            path.lineTo(point);
    }

    painter.save();
    painter.setClipRect(plot.adjusted(0, 0, 1, 1));
    painter.setPen(QPen(QColor(0, 94, 184), 2));
    painter.drawPath(path);
    painter.restore();
}
