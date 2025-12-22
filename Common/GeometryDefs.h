#ifndef GEOMETRYDEFS_H
#define GEOMETRYDEFS_H

#include <QPointF>
#include <array>

namespace xcanvas {
    enum class SegmentType
    {
        MoveTo,
        LineTo,
        QuadTo,
        CubicTo
    };

    struct Segment
    {
        SegmentType            type{SegmentType::MoveTo};
        std::array<QPointF, 3> points{};

        Segment() = default;

        int pointCount() const
        {
            switch (type)
            {
                case SegmentType::MoveTo:  return 1;
                case SegmentType::LineTo:  return 1;
                case SegmentType::QuadTo:  return 2;
                case SegmentType::CubicTo: return 3;
                default:                   return 0;
            }
        }

        QPointF& point(int i) { return points[i]; }
        const QPointF& point(int i) const { return points[i]; }

        QPointF& end()
        {
            switch (type)
            {
                case SegmentType::MoveTo:
                case SegmentType::LineTo:  return points[0];
                case SegmentType::QuadTo:  return points[1];
                case SegmentType::CubicTo: return points[2];
            }
            return points[0];
        }
        const QPointF& end() const { return const_cast<Segment*>(this)->end(); }

        QPointF& ctrl1() { return points[0]; }
        const QPointF& ctrl1() const { return points[0]; }

        QPointF& ctrl2() { return points[1]; }
        const QPointF& ctrl2() const { return points[1]; }

        static Segment moveTo(const QPointF& p)
        {
            Segment s; s.type = SegmentType::MoveTo; s.points[0] = p; return s;
        }

        static Segment lineTo(const QPointF& p)
        {
            Segment s; s.type = SegmentType::LineTo; s.points[0] = p; return s;
        }

        static Segment quadTo(const QPointF& c, const QPointF& end)
        {
            Segment s; s.type = SegmentType::QuadTo; s.points[0] = c; s.points[1] = end; return s;
        }

        static Segment cubicTo(const QPointF& c1, const QPointF& c2, const QPointF& end)
        {
            Segment s; s.type = SegmentType::CubicTo;
            s.points[0] = c1; s.points[1] = c2; s.points[2] = end; return s;
        }
    };
} // namespace xcanvas

#endif // GEOMETRYDEFS_H