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
    void AddShape(BaseShape* pShape);
    BaseShape* GetShape(int index);
    void SelectShapes(bool selected);
    void SelectShapes(const QRectF& rect);
    BaseShape* GetShapeByPoint(const QPointF& pt);
    QRectF GetSelectedShapeRect();

private:
    QVector<BaseShape*> m_shapes;
};

#endif // SHAPES_H
