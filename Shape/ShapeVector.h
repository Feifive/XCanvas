#ifndef VECTOR_H
#define VECTOR_H

#include "GeometryDefs.h"
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

class ShapeVector final : public Shape
{
public:
    explicit ShapeVector();
    ShapeVector(const ShapeVector& other);
    ~ShapeVector() override;
    bool      hitTest(const QPointF& point, double  tolerance) const override;
    Shape* clone() override;
    ShapeType type() const override;
    const QVector<Segment>& segments() const;
    QVector<Segment>&       segments();
    bool                    isEmpty() const;
    VectorSemantic          semantic() const;
    void                    setSemantic(VectorSemantic semantic);
    void bakeTransform();
    void moveTo(const QPointF& p);
    void lineTo(const QPointF& p);
    void quadTo(const QPointF& c, const QPointF& end);
    void cubicTo(const QPointF& c1, const QPointF& c2, const QPointF& end);
    void setSegments(QVector<Segment> segments);
    void clear();

protected:
    void updatePainterPath() override;

private:
    QVector<Segment> m_segments;
    VectorSemantic   m_semantic;
};

}// namespace xcanvas

#endif//VECTOR_H
