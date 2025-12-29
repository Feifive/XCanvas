#include "ShapeVector.h"
#include "MyMath.h"

namespace xcanvas
{
ShapeVector::ShapeVector() : m_semantic(VectorSemantic::Unknown)
{
}

ShapeVector::~ShapeVector()
{
}

bool ShapeVector::hitTest(const QPointF &point, const double tolerance) const {
    return isPointNearPath(point, tolerance);
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
    m_originalPath = QPainterPath();
    for (const Segment& s : m_segments)
    {
        switch (s.type)
        {
        case SegmentType::MoveTo:
            m_originalPath.moveTo(s.point(0));
            break;
        case SegmentType::LineTo:
            m_originalPath.lineTo(s.point(0));
            break;
        case SegmentType::QuadTo:
            m_originalPath.quadTo(s.point(0), s.point(1));
            break;
        case SegmentType::CubicTo:
            m_originalPath.cubicTo(s.point(0), s.point(1), s.point(2));
            break;
        }
    }
}

void ShapeVector::bakeTransform()
{
    if (m_transform.isIdentity())
    {
        return;
    }

    for (Segment& s : m_segments)
    {
        const int pointCount = s.pointCount();
        for (int i = 0; i < pointCount; ++i)
        {
            s.point(i) = m_transform.map(s.point(i));
        }
    }

    m_transform.reset();
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
