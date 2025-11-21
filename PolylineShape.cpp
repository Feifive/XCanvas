#include "PolylineShape.h"
#include "Global.h"
#include <QPainter>
#include <QPainterPath>

PolylineShape::PolylineShape()
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

PolylineShape::~PolylineShape()
{
}

void PolylineShape::SetPoints(QVector<QPointF> points)
{
    m_points = points;

    UpdatePath();
    prepareGeometryChange();
    update();
}

void PolylineShape::Offset(const QPointF& offset)
{
    if (m_points.isEmpty())
    {
        return;
    }

    QVector<QPointF> newPoints = m_points;
    for (auto& pt : newPoints)
    {
        pt += offset;
    }

    SetPoints(newPoints);
}

QPainterPath PolylineShape::shape() const
{
    return m_pPath ? *m_pPath : QPainterPath();
}

QRectF PolylineShape::boundingRect() const
{
    return m_pPath ? m_pPath->boundingRect() : QRectF();
}

void PolylineShape::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (!m_pPath)
    {
        return;
    }

    qreal scale = painter->transform().m11();

    if (IsSelected())
    {
        painter->setPen(SELECTED_LINE_PEN(scale));
    }
    else
    {
        painter->setPen(DRAWED_LINE_PEN(scale));
    }
    painter->drawPath(*m_pPath);
}

void PolylineShape::UpdatePath()
{
    if (m_points.isEmpty())
    {
        return;
    }

    if (!m_pPath)
    {
        m_pPath = new QPainterPath;
    }

    m_pPath->clear();
    m_pPath->moveTo(m_points[0]);
    for (int i = 1; i < m_points.size(); ++i)
    {
        m_pPath->lineTo(m_points[i]);
    }
}
