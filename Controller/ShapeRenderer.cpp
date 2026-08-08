#include "ShapeRenderer.h"

#include "Shape/Shape.h"
#include "Shape/ShapeImage.h"

#include <QPainter>

namespace xcanvas
{
void ShapeRenderer::render(QPainter *painter, const Shape &shape) const
{
    if (!painter)
    {
        return;
    }

    switch (shape.type())
    {
    case ShapeType::Image:
        renderImage(painter, static_cast<const ShapeImage &>(shape));
        break;
    default:
        painter->drawPath(shape.path());
    }
}

void ShapeRenderer::renderImage(QPainter *painter, const ShapeImage &shape) const
{
    if (shape.image().isNull())
    {
        return;
    }

    painter->save();
    painter->setTransform(shape.transform(), true);
    painter->drawImage(QRectF(QPointF(0.0, 0.0), shape.imageSize()), shape.image());
    painter->restore();
}

} // namespace xcanvas
