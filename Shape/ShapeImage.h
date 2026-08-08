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
    ShapeImage(const ShapeImage& other);
    ~ShapeImage() override;
    bool isImage() const override;
    bool hitTest(const QPointF& point, double  tolerance) const override;
    Shape* clone() override;
    ShapeType type() const override;
    void      setSize(const QSizeF& size);
    const QImage& image() const;
    QSizeF        imageSize() const;

  protected:
    void updatePainterPath() override;

  private:
    QImage m_image;
    QSizeF m_imageSize;
};

}// namespace xcanvas

#endif// IMAGE_H
