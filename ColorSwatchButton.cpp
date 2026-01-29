#include "ColorSwatchButton.h"
#include <QPainter>
#include <QMouseEvent>

ColorSwatchButton::ColorSwatchButton(const QColor& color, QWidget* parent)
    : QWidget(parent), m_color(color)
{
    setFixedSize(28, 28);
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

    QRectF r = rect().adjusted(2, 2, -2, -2);

    // 填充
    p.setBrush(m_color);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(r, 4, 4);

    // Hover 描边
    if (m_hovered)
    {
        p.setPen(QPen(QColor("#DDDDDD"), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r, 4, 4);
    }

    // 选中态
    if (m_selected)
    {
        p.setPen(QPen(QColor("#C0C0C0"), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r, 4, 4);
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