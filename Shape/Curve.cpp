#include "Curve.h"

namespace xcanvas
{

Curve::Curve()
{
}

Curve::~Curve()
{
}

void Curve::SetPoints(QVector<QPointF> points)
{
    m_points = points;

    markDirty();
}

void Curve::translate(const QPointF& offset)
{
    if (m_points.isEmpty())
    {
        return;
    }

    for (QPointF& point : m_points)
    {
        point += offset;
    }

    markDirty();
}

ShapeType Curve::type() const
{
    return ShapeType::Curve;
}

void Curve::updatePainterPath()
{
    m_path = QPainterPath();

    if (m_points.isEmpty())
    {
        return;
    }

    if (m_points.size() < 4)
    {
        m_path.moveTo(m_points[0]);
        for (int i = 1; i < m_points.size(); ++i)
        {
            m_path.lineTo(m_points[i]);
        }

        return;
    }

    m_path.moveTo(m_points[0]);

    for (int i = 0; i + 3 < m_points.size(); i += 3)
    {
        const QPointF& c1  = m_points[i + 1];
        const QPointF& c2  = m_points[i + 2];
        const QPointF& end = m_points[i + 3];

        m_path.cubicTo(c1, c2, end);
    }
}

}// namespace xcanvas
