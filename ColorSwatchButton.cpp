#include "ColorSwatchButton.h"
#include <QPainter>
#include <QMouseEvent>

ColorSwatchButton::ColorSwatchButton(const QColor& color, QWidget* parent)
    : QWidget(parent), m_color(color)
{
    setFixedSize(24,24);
    setCursor(Qt::PointingHandCursor);
}

void ColorSwatchButton::setSelected(bool sel)
{
    if (m_selected == sel)
        return;
    m_selected = sel;
    update();
}

void ColorSwatchButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_hovered && !m_selected) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(243, 243, 247));
        p.drawRoundedRect(rect(), 4, 4);
    }

    const QRectF r = rect().adjusted(3, 3, -3, -3);
    const qreal radius = 4.0;

    p.setBrush(m_color);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(r, radius, radius);

    if (m_selected)
    {
        const qreal padding = 2.5;
        const QPen selectPen(QColor(45, 54, 65), 1);
        p.setPen(selectPen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(-padding, -padding, padding, padding),
                          radius + padding, radius + padding);
    }
}

void ColorSwatchButton::mousePressEvent(QMouseEvent*)
{
    emit clicked(m_color);
}

void ColorSwatchButton::enterEvent(QEnterEvent*)
{
    m_hovered = true;
    update();
}

void ColorSwatchButton::leaveEvent(QEvent*)
{
    m_hovered = false;
    update();
}