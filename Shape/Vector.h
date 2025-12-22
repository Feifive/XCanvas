#ifndef VECTOR_H
#define VECTOR_H

#include "Shape.h"

namespace xcanvas
{

enum class VectorSemantic
{
    Unknown,
    Polyline,
    Rectangle,
    RoundedRect,
    Circle,
    Ellipse,
    Curve,
    Polygon,
    Complex
};

enum class SegmentType
{
    MoveTo,
    LineTo,
    QuadTo,
    CubicTo
};

struct Segment
{
    SegmentType            type;
    std::array<QPointF, 3> points{};

    Segment() = default;

    int pointCount() const
    {
        switch (type)
        {
        case SegmentType::MoveTo:
            return 1;
        case SegmentType::LineTo:
            return 1;
        case SegmentType::QuadTo:
            return 2;
        case SegmentType::CubicTo:
            return 3;
        default:
            return 0;
        }
    }

    QPointF& point(int i)
    {
        return points[i];
    }
    const QPointF& point(int i) const
    {
        return points[i];
    }

    QPointF& end()
    {
        switch (type)
        {
        case SegmentType::MoveTo:
        case SegmentType::LineTo:
            return points[0];
        case SegmentType::QuadTo:
            return points[1];
        case SegmentType::CubicTo:
            return points[2];
        }
        return points[0];
    }
    const QPointF& end() const
    {
        switch (type)
        {
        case SegmentType::MoveTo:
        case SegmentType::LineTo:
            return points[0];
        case SegmentType::QuadTo:
            return points[1];
        case SegmentType::CubicTo:
            return points[2];
        }
        return points[0];
    }

    QPointF& ctrl1()
    {
        return points[0];
    }
    const QPointF& ctrl1() const
    {
        return points[0];
    }

    QPointF& ctrl2()
    {
        return points[1];
    }
    const QPointF& ctrl2() const
    {
        return points[1];
    }

    static Segment moveTo(const QPointF& p)
    {
        Segment s;
        s.type      = SegmentType::MoveTo;
        s.points[0] = p;
        return s;
    }

    static Segment lineTo(const QPointF& p)
    {
        Segment s;
        s.type      = SegmentType::LineTo;
        s.points[0] = p;
        return s;
    }

    static Segment quadTo(const QPointF& c, const QPointF& end)
    {
        Segment s;
        s.type      = SegmentType::QuadTo;
        s.points[0] = c;
        s.points[1] = end;
        return s;
    }

    static Segment cubicTo(const QPointF& c1, const QPointF& c2, const QPointF& end)
    {
        Segment s;
        s.type      = SegmentType::CubicTo;
        s.points[0] = c1;
        s.points[1] = c2;
        s.points[2] = end;
        return s;
    }
};

class Vector : public Shape
{
  public:
    explicit Vector();
    ~Vector() override;
    void      translate(const QPointF& offset) override;
    ShapeType type() const override;

    void                    transform(const QTransform& transform);
    const QVector<Segment>& segments() const;
    QVector<Segment>&       segments();
    bool                    isEmpty() const;
    VectorSemantic          semantic() const;
    void                    setSemantic(const VectorSemantic semantic);

    void moveTo(const QPointF& p);
    void lineTo(const QPointF& p);
    void quadTo(const QPointF& c, const QPointF& end);
    void cubicTo(const QPointF& c1, const QPointF& c2, const QPointF& end);
    void clear();

  protected:
    void updatePainterPath() override;

  private:
    QVector<Segment> m_segments;
    VectorSemantic   m_semantic;
};

}// namespace xcanvas

#endif//VECTOR_H
