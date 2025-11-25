#include "rulerwidget.h"
#include <QGraphicsView>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

RulerWidget::RulerWidget(Qt::Orientation orientation, QWidget* parent) : QWidget(parent), orientation(orientation), m_pAttachView(nullptr)
{
    orientation == Qt::Horizontal ? setFixedHeight(24) : setFixedWidth(24);
    slidingLineColor.setAlphaF(0.7);
}

RulerWidget::~RulerWidget()
{
}

void RulerWidget::paintEvent(QPaintEvent* event)
{
    auto rect   = event->rect();
    auto height = rect.height();
    auto width  = rect.width();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect, backgroundColor);

    orientation == Qt::Horizontal ? painter.translate(0 - offset, 0) : painter.translate(0, 0 - offset);
    //绘制刻度
    painter.save();
    auto length = (orientation == Qt::Horizontal ? rect.width() : rect.height()) + offset;
    painter.setPen(QPen(textAndLineColor, 1));
    painter.setFont(QFont("Microsoft YaHei UI", 8));
    if (orientation == Qt::Horizontal)
    {
        auto shortLine_y  = height * 0.75;
        auto longLine_y   = height * 0.5;
        auto middleLine_y = height * 0.625;
        auto halfHeight   = height / 2;
        int  temp         = 0;
        for (int i = 0; i < length; i += 10)
        {
            if (temp % 10 == 0)//画长线
            {
                painter.drawLine(QPointF(i, longLine_y), QPointF(i, height));
                painter.drawText(QRectF(i - 50, 0, 100, halfHeight), Qt::AlignCenter | Qt::TextWordWrap, QString::number(i));
            }
            else if (temp % 5 == 0)
            {
                painter.drawLine(QPointF(i, middleLine_y), QPointF(i, height));
            }
            else
            {
                painter.drawLine(QPointF(i, shortLine_y), QPointF(i, height));
            }
            ++temp;
        }
    }
    else
    {
        auto shortLineWidth  = width * 0.25;
        auto longLineWidth   = width * 0.5;
        auto middleLineWidth = width * 0.375;
        auto halfWidth       = width / 2;
        int  temp            = 0;
        for (int i = 0; i < length; i += 10)
        {
            QPointF p0(width, i);

            if (temp % 10 == 0)
            {
                painter.drawLine(p0, QPointF(width - longLineWidth, i));

                painter.save();

                painter.translate(halfWidth, i);
                painter.rotate(-90);
                painter.drawText(QRectF(-50, -halfWidth, 100, halfWidth), Qt::AlignCenter, QString::number(i));

                painter.restore();
            }
            else if (temp % 5 == 0)
            {
                painter.drawLine(p0, QPointF(width - middleLineWidth, i));
            }
            else
            {
                painter.drawLine(p0, QPointF(width - shortLineWidth, i));
            }

            ++temp;
        }
    }
    painter.restore();

    painter.setPen(Qt::transparent);
    painter.setBrush(slidingLineColor);
    if (orientation == Qt::Horizontal)
    {
        painter.drawRect(slidingLinePos, 0, 1, height);
    }
    else
    {
        painter.drawRect(0, slidingLinePos, width, 1);
    }

    QWidget::paintEvent(event);
}
