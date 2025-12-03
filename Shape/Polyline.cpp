#include "Polyline.h"
#include "Global.h"
#include <QPainter>
#include <QPainterPath>

xcanvas::Polyline::Polyline()
{
}

xcanvas::Polyline::~Polyline()
{
}

void xcanvas::Polyline::SetPoints(QVector<QPointF> points)
{
    m_points = points;

    markDirty();
}

void xcanvas::Polyline::translate(const QPointF& offset)
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

    m_points = newPoints;

    markDirty();
}

void xcanvas::Polyline::updatePainterPath()
{
    if (m_points.isEmpty())
    {
        return;
    }

    m_path = QPainterPath();
    m_path.moveTo(m_points[0]);
    for (int i = 1; i < m_points.size(); ++i)
    {
        m_path.lineTo(m_points[i]);
    }
}
