#include "RectShape.h"
#include "Global.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

RectShape::RectShape()
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
}

RectShape::~RectShape()
{

}

void RectShape::SetRect(const QRectF &rect)
{
    if(m_rect == rect)
    {
        return;
    }

    m_rect = rect;
    UpdatePath();
    prepareGeometryChange();
    update();
}

void RectShape::Offset(const QPointF& offset)
{
    QRectF rect = m_rect.translated(offset);
    SetRect(rect);
}

QPainterPath RectShape::shape() const
{
    return *m_pPath;
}

QRectF RectShape::boundingRect() const
{
    return m_pPath ? m_pPath->boundingRect() : QRectF();
}

void RectShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // qDebugTime()<<__FUNCTION__;
    if(!m_pPath)
    {
        return;
    }

    qreal scale = painter->transform().m11();

    if(IsSelected())
    {
        painter->setPen(SELECTED_LINE_PEN(scale));
    }
    else
    {
        painter->setPen(DRAWED_LINE_PEN(scale));
    }
    painter->drawPath(*m_pPath);
}

void RectShape::UpdatePath()
{
    if(!m_rect.isValid())
    {
        return;
    }

    if(!m_pPath)
    {
        m_pPath = new QPainterPath;
    }

    m_pPath->clear();
    m_pPath->addRect(m_rect);
}
