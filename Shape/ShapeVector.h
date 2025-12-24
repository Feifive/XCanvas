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
    ~ShapeVector() override;
    void      translate(const QPointF& offset) override;
    void      rotate(double angle, const QPointF& customCenter) override;
    ShapeType type() const override;

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
    void transform(const QTransform& transform);

  private:
    QVector<Segment> m_segments;
    VectorSemantic   m_semantic;
};

}// namespace xcanvas

#endif//VECTOR_H
