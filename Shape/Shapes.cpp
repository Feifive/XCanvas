#include "Shapes.h"
#include <QGraphicsItem>
#include <QPainterPath>
#include <QDebug>

Shapes::Shapes() {}

Shapes::~Shapes()
{
}

int Shapes::Count()
{
    return m_shapes.count();
}

void Shapes::AddShape(Shape *pShape)
{
    m_shapes.append(pShape);
}

Shape *Shapes::GetShape(int index)
{
    if(index >= m_shapes.size() || index < 0)
    {
        return nullptr;
    }
    return m_shapes.at(index);
}

void Shapes::SelectShapes(bool selected)
{
    for(auto shape : m_shapes)
    {
        shape->Select(selected);
    }
}

void Shapes::SelectShapes(const QRectF& rect)
{
    SelectShapes(false);
    for(auto shape : m_shapes)
    {
        if(rect.contains(shape->boundingRect()))
        {
            shape->Select(true);
        }
    }
}

Shape *Shapes::GetShapeByPoint(const QPointF &pt)
{
    for(auto shape : m_shapes)
    {
        QRectF rect = shape->boundingRect();
        if(rect.contains(pt))
        {
            return shape;
        }
    }

    return nullptr;
}

Shapes* Shapes::GetSelectedShapes()
{
    Shapes* pShapes = new Shapes();

    for (int i = 0; i < m_shapes.count(); i++)
    {
        Shape* pShape = m_shapes[i];
        if (pShape->IsSelected())
        {
            pShapes->AddShape(pShape);
        }
    }

    return pShapes;
}

QRectF Shapes::GetSelectedShapeRect()
{
    QRectF rect(0, 0, 0, 0);
    for(auto shape : m_shapes)
    {
        if(shape->IsSelected())
        {
            rect = rect.united(shape->boundingRect());
        }
    }

    return rect;
}

void Shapes::Offset(const QPointF& offset)
{
    for (int i = 0; i < m_shapes.count(); i++)
    {
        m_shapes[i]->Offset(offset);
    }
}
