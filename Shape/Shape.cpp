#include "Shape.h"
#include "Global.h"
#include "MyMath.h"
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

namespace xcanvas
{
Shape::~Shape()
{
}

void Shape::draw(QPainter* painter) const
{
    painter->save();

    painter->setPen(m_selected ? selectedPen() : normalPen(m_color));

    painter->drawPath(path());

    painter->restore();
}

void Shape::setSelected(bool selected)
{
    m_selected = selected;
}

void Shape::setColor(const QColor& color)
{
    m_color = color;
    markDirty();
}

QColor Shape::color() const
{
    return m_color;
}

double Shape::rotation() const
{
    return m_rotation;
}

QPainterPath& Shape::path() const
{
    if (m_dirty)
    {
        m_dirty = false;
        const_cast<Shape*>(this)->updatePainterPath();
    }

    return m_path;
}

QRectF Shape::boundingRect() const
{
    if (!m_boundingRectDirty)
    {
        return m_cachedBoundingRect;
    }

    QRectF rawRect = path().boundingRect();

    qreal left   = rawRect.left();
    qreal top    = rawRect.top();
    qreal right  = rawRect.right();
    qreal bottom = rawRect.bottom();

    const double delta = BOUNDING_BOX_TOLERANCE;

    if (qFuzzyIsNull(right - left))
    {
        left -= delta / 2.0;
        right += delta / 2.0;
    }

    if (qFuzzyIsNull(bottom - top))
    {
        top -= delta / 2.0;
        bottom += delta / 2.0;
    }

    m_boundingRectDirty  = false;
    m_cachedBoundingRect = QRectF(QPointF(left, top), QPointF(right, bottom));

    return m_cachedBoundingRect;
}

bool Shape::isPointNearPath(const QPointF& point, double dScale)
{
    const QList<QPolygonF>& polygons = path().toSubpathPolygons();
    const double            maxDist  = 6 / dScale;

    for (const QPolygonF& polygon : polygons)
    {
        QRectF rect = polygon.boundingRect();
        rect.adjust(-maxDist, -maxDist, maxDist, maxDist);
        if (!rect.contains(point))
        {
            continue;
        }
        for (int j = 0; j < polygon.size() - 1; ++j)
        {
            if (geometryMath::point2Segment(polygon[j], polygon[j + 1], point) <= maxDist)
            {
                return true;
            }
        }
    }
    return false;
}

bool Shape::isDirty() const
{
    return m_dirty;
}

void Shape::setDirty(const bool dirty) const
{
    m_dirty = dirty;
}

void xcanvas::Shape::markDirty() const
{
    m_dirty             = true;
    m_boundingRectDirty = true;
}
}// namespace xcanvas
