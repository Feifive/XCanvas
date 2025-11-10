#include "BaseShape.h"
#include "MyMath.h"

void BaseShape::Select(bool selected)
{
    if(m_bSelected == selected)
    {
        return;
    }

    m_bSelected = selected;
    update();
}

bool BaseShape::IsSelected() const
{
    return m_bSelected;
}

bool BaseShape::IsPointNearPath(const QPointF &point, double dScale)
{
    if(!m_pPath)
    {
        UpdatePath();

        if(!m_pPath)
        {
            return false;
        }
    }

    const QList<QPolygonF>& polygons = m_pPath->toSubpathPolygons();
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
            if (MyMath::Point2Segment(polygon[j], polygon[j + 1], point) <= maxDist)
            {
                return true;
            }
        }
    }
    return false;
}
