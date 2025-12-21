#include "Shape.h"
#include "MyMath.h"
#include "Global.h"
#include <QPolygonF>
#include <QPainterPath>
#include <QPainter>

xcanvas::Shape::~Shape()
{
}

void xcanvas::Shape::draw(QPainter* painter) const
{
    painter->save();

    painter->setPen(m_selected ? selectedPen() : normalPen(m_color));

    painter->drawPath(path());

    painter->restore();
}

void xcanvas::Shape::setSelected(bool selected)
{
    m_selected = selected;
}

void xcanvas::Shape::setColor(const QColor& color)
{
    m_color = color; 
    markDirty();
}

QPainterPath& xcanvas::Shape::path() const
{
    if (m_dirty) 
    {
        m_dirty = false;
        const_cast<Shape*>(this)->updatePainterPath();
    }

    return m_path;
}

QRectF xcanvas::Shape::boundingRect() const
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
        left  -= delta / 2.0;
        right += delta / 2.0;
    }

    if (qFuzzyIsNull(bottom - top))
    {
        top    -= delta / 2.0;
        bottom += delta / 2.0;
    }

    m_boundingRectDirty  = false;
    m_cachedBoundingRect = QRectF(QPointF(left, top), QPointF(right, bottom));

	return m_cachedBoundingRect;
}

bool xcanvas::Shape::isPointNearPath(const QPointF &point, double dScale)
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
            if (MyMath::point2Segment(polygon[j], polygon[j + 1], point) <= maxDist)
            {
                return true;
            }
        }
    }
    return false;
}

void xcanvas::Shape::markDirty()
{
    m_dirty             = true;
	m_boundingRectDirty = true;
}
