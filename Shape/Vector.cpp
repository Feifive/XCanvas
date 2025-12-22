#include "Vector.h"

namespace xcanvas
{
Vector::Vector() : m_semantic(VectorSemantic::Unknown)
{
}

Vector::~Vector()
{
}

void Vector::translate(const QPointF& offset)
{
    if (offset.isNull())
    {
        return;
    }

    QTransform t;
    t.translate(offset.x(), offset.y());
    transform(t);
}

ShapeType Vector::type() const
{
    return ShapeType::Vector;
}

void Vector::moveTo(const QPointF& p)
{
    m_segments.append(Segment::moveTo(p));
}

void Vector::lineTo(const QPointF& p)
{
    m_segments.append(Segment::lineTo(p));
}

void Vector::quadTo(const QPointF& c, const QPointF& end)
{
    m_segments.append(Segment::quadTo(c, end));
}

void Vector::cubicTo(const QPointF& c1, const QPointF& c2, const QPointF& end)
{
    m_segments.append(Segment::cubicTo(c1, c2, end));
}

void Vector::clear()
{
    m_segments.clear();
}

void Vector::updatePainterPath()
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

void Vector::transform(const QTransform& transform)
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

const QVector<Segment>& Vector::segments() const
{
    return m_segments;
}

QVector<Segment>& Vector::segments()
{
    return m_segments;
}

bool Vector::isEmpty() const
{
    return m_segments.isEmpty();
}

VectorSemantic Vector::semantic() const
{
    return m_semantic;
}

void Vector::setSemantic(const VectorSemantic semantic)
{
    m_semantic = semantic;
}
}// namespace xcanvas
