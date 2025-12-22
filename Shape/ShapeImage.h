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
