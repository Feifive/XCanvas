#include "RulerWidget.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>

RulerWidget::RulerWidget(Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent)
    , m_orientation(orientation)
{
    if (orientation == Qt::Horizontal)
    {
        setFixedHeight(24);
    }
    else
    {
        setFixedWidth(24);
    }

    m_font.setFamily("PingFang SC");
    m_font.setPixelSize(8);
}

void RulerWidget::attachView(MyGraphicsView* view)
{
    m_view = view;
}

double RulerWidget::tickStep(double scale) const
{
    static const double steps[] = {0.1, 0.5, 1.0, 5.0, 10.0, 50.0, 100.0, 500.0};

    const double minPx = 10.0; // 刻度之间的最小像素间距

    for (double step : steps)
    {
        double px = step * scale;
        if (px >= minPx)
        {
            return step;
        }
    }

    return 500.0;
}

void RulerWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (!m_view || !m_view->scene())
    {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), m_backgroundColor);

    if (m_orientation == Qt::Horizontal)
    {
        drawHorizontal(painter);
    }
    else
    {
        drawVertical(painter);
    }
}

void RulerWidget::drawHorizontal(QPainter &p)
{
    QRect widgetRect = rect();

    QPointF sceneTL = m_view->mapToScene(0, 0);
    QPointF sceneTR = m_view->mapToScene(widgetRect.width(), 0);

    double s0 = sceneTL.x();
    double s1 = sceneTR.x();

    double scale = m_view->zoomValue();
    double step  = tickStep(scale);
    int majorCount = 10;

    qint64 firstIndex = qFloor((s0 + 1e-12) / step);
    double x0 = firstIndex * step;

    QRectF sceneRect = m_view->scene()->sceneRect();
    double originX = sceneRect.center().x();

    p.setPen(QPen(m_textAndLineColor, 0));
    p.setFont(m_font);

    double tickWidth = widgetRect.height() * 0.6;

    for (double x = x0; x <= s1 + step; x += step, firstIndex++)
    {
        double sx = (x - s0) * scale;
        sx = std::round(sx) + 0.5;

        if (firstIndex % majorCount == 0)
        {
            p.drawLine(QPointF(sx, tickWidth), QPointF(sx, widgetRect.height()));

            double  value  = x - originX;
            QString number = QString::number(value, 'f', 0);

            int w  = p.fontMetrics().horizontalAdvance(number);
            double pxMajor = majorCount * step * scale;

            if (pxMajor >= w)
            {
                p.drawText(QRectF(sx - w / 2.0, 0, w, widgetRect.height() / 2.0),
                           Qt::AlignCenter, number);
            }
        }
    }
}

void RulerWidget::drawVertical(QPainter &p)
{
    QRect widgetRect = rect();

    QPointF sceneTL = m_view->mapToScene(0, 0);
    QPointF sceneBL = m_view->mapToScene(0, widgetRect.height());

    double s0 = sceneTL.y();
    double s1 = sceneBL.y();

    double scale = m_view->zoomValue();
    double step  = tickStep(scale);
    int majorCount = 10;

    qint64 firstIndex = qFloor((s0 + 1e-12) / step);
    double y0 = firstIndex * step;

    QRectF sceneRect = m_view->scene()->sceneRect();
    double originY   = sceneRect.center().y();

    p.setPen(QPen(m_textAndLineColor, 0));
    p.setFont(m_font);

    double lineWidth = widgetRect.width() * 0.4;

    for (double y = y0; y <= s1 + step; y += step, firstIndex++)
    {
        double sy = (y - s0) * scale;
        sy = std::round(sy) + 0.5;

        if (firstIndex % majorCount == 0)
        {
            p.drawLine(QPointF(widgetRect.width(), sy),
                       QPointF(widgetRect.width() - lineWidth, sy));

            double value = y - originY;
            QString number = QString::number(value, 'f', 0);

            QFontMetrics fm(p.font());
            int w = fm.horizontalAdvance(number);
            int h = fm.height();

            double pxMajor = majorCount * step * scale;
            if (pxMajor >= h)
            {
                p.save();

                p.translate(widgetRect.width() - lineWidth - h, sy);
                p.rotate(-90);

                QRectF rc(-w / 2.0, -h / 2.0, w, h);
                p.drawText(rc, Qt::AlignCenter, number);

                p.restore();
            }
        }
    }
}
