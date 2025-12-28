#ifndef IMAGE_H
#define IMAGE_H

#include "Shape.h"

#include <QImage>

namespace xcanvas
{

class ShapeImage final : public Shape
{
  public:
    explicit ShapeImage(QImage image);
    ~ShapeImage() override;
    void      draw(QPainter* painter) const override;
    void      translate(const QPointF& offset) override;
    void      rotate(double angle, const QPointF& customCenter) override;
    bool hitTest(const QPointF& point, double  tolerance) const override;
    void scale(double sx, double sy, std::optional<QPointF> center) override;
    void resize(const QSizeF& targetSize, bool keepAspectRatio) override;
    std::unique_ptr<ShapeState> createSnapshot() const override;
    void restoreSnapshot(const ShapeState* state) override;
    ShapeType type() const override;
    void      setRect(const QRectF& rect);

  protected:
    void updatePainterPath() override;

  private:
    QImage m_image;
    QRectF m_rect;
};

}// namespace xcanvas

#endif// IMAGE_H
