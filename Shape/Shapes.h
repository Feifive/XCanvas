#ifndef SHAPES_H
#define SHAPES_H

#include "BaseShape.h"
#include <QVector>
#include <QPointF>

class Shapes
{
public:
    Shapes();
    ~Shapes();
    int Count();
    void AddShape(Shape* pShape);
    Shape* GetShape(int index);
    void SelectShapes(bool selected);
    void SelectShapes(const QRectF& rect);
    Shape* GetShapeByPoint(const QPointF& pt);
    Shapes* GetSelectedShapes();
    QRectF GetSelectedShapeRect();
    void Offset(const QPointF& offset);

private:
    QVector<Shape*> m_shapes;
};

#endif // SHAPES_H
