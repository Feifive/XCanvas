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
    bool hitTest(const QPointF& point, double  tolerance) const override;
    Shape* clone() override;
    ShapeType type() const override;
    void      setSize(const QSizeF& size);

  protected:
    void updatePainterPath() override;

  private:
    QImage m_image;
    QSizeF m_imageSize;
};

}// namespace xcanvas

#endif// IMAGE_H
