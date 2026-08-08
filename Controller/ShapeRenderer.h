#ifndef SHAPERENDERER_H
#define SHAPERENDERER_H

class QPainter;

namespace xcanvas
{
class Shape;
class ShapeImage;

class ShapeRenderer final
{
public:
    void render(QPainter *painter, const Shape &shape) const;

private:
    void renderImage(QPainter *painter, const ShapeImage &shape) const;
};

} // namespace xcanvas

#endif // SHAPERENDERER_H
