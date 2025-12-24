#include "ShapeVector.h"

namespace xcanvas
{
ShapeVector::ShapeVector() : m_semantic(VectorSemantic::Unknown)
{
}

ShapeVector::~ShapeVector()
{
}

void ShapeVector::translate(const QPointF& offset)
{
    if (offset.isNull())
    {
        return;
    }

    QTransform t;
    t.translate(offset.x(), offset.y());
    transform(t);
}

void ShapeVector::rotate(double angle, const QPointF& customCenter)
{
    if (qFuzzyIsNull(angle))
    {
        return;
    }

    m_rotation = fmod(m_rotation + angle, 360.0);
    if (m_rotation < 0)
    {
        m_rotation += 360.0;
    }

    QTransform t;
    t.translate(customCenter.x(), customCenter.y());
    t.rotate(angle);
    t.translate(-customCenter.x(), -customCenter.y());

    transform(t);
}

ShapeType ShapeVector::type() const
{
    return ShapeType::Vector;
}

void ShapeVector::moveTo(const QPointF& p)
{
    m_segments.append(Segment::moveTo(p));
}

void ShapeVector::lineTo(const QPointF& p)
{
    m_segments.append(Segment::lineTo(p));
}

void ShapeVector::quadTo(const QPointF& c, const QPointF& end)
{
    m_segments.append(Segment::quadTo(c, end));
}

void ShapeVector::cubicTo(const QPointF& c1, const QPointF& c2, const QPointF& end)
{
    m_segments.append(Segment::cubicTo(c1, c2, end));
}

void ShapeVector::clear()
{
    m_segments.clear();
}

void ShapeVector::updatePainterPath()
{
    m_path = QPainterPath();
    for (const Segment& s : m_segments)
    {
        switch (s.type)
        {
        case SegmentType::MoveTo:
            m_path.moveTo(s.point(0));
            break;
        case SegmentType::LineTo:
            m_path.lineTo(s.point(0));
            break;
        case SegmentType::QuadTo:
            m_path.quadTo(s.point(0), s.point(1));
            break;
        case SegmentType::CubicTo:
            m_path.cubicTo(s.point(0), s.point(1), s.point(2));
            break;
        }
    }
}

void ShapeVector::transform(const QTransform& transform)
{
    if (transform.isIdentity())
    {
        return;
    }

    for (Segment& s : m_segments)
    {
        const int pointCount = s.pointCount();
        for (int i = 0; i < pointCount; ++i)
        {
            s.point(i) = transform.map(s.point(i));
        }
    }

    markDirty();
}

const QVector<Segment>& ShapeVector::segments() const
{
    return m_segments;
}

QVector<Segment>& ShapeVector::segments()
{
    return m_segments;
}

bool ShapeVector::isEmpty() const
{
    return m_segments.isEmpty();
}

VectorSemantic ShapeVector::semantic() const
{
    return m_semantic;
}

void ShapeVector::setSemantic(const VectorSemantic semantic)
{
    m_semantic = semantic;
}
}// namespace xcanvas
