#include "Shapes.h"
#include "Shape.h"
#include <QDebug>
#include <QGraphicsItem>
#include <QPainterPath>

xcanvas::Shapes::Shapes()
{
}

xcanvas::Shapes::~Shapes()
{
}

void xcanvas::Shapes::addShape(Shape* shape)
{
    if (shape)
    {
        m_shapes.append(shape);
    }
}

void xcanvas::Shapes::removeShape(Shape* shape)
{
    m_shapes.removeOne(shape);
}

void xcanvas::Shapes::deleteShape(Shape* shape)
{
    if (m_shapes.removeOne(shape))
    {
        delete shape;
    }
}

void xcanvas::Shapes::clear()
{
    deleteAllShapes();
    m_shapes.clear();
}

void xcanvas::Shapes::append(const QVector<Shape*>& shapes)
{
    m_shapes.append(shapes);
}

void xcanvas::Shapes::deleteAllShapes()
{
    for (Shape* shape : m_shapes)
    {
        delete shape;
    }
}

int xcanvas::Shapes::count() const
{
    return m_shapes.count();
}

bool xcanvas::Shapes::isEmpty() const
{
    return m_shapes.isEmpty();
}

xcanvas::Shape* xcanvas::Shapes::shapeAt(int index) const
{
    if (index >= 0 && index < m_shapes.count())
    {
        return m_shapes.at(index);
    }
    return nullptr;
}

xcanvas::Shape* xcanvas::Shapes::shapeAt(const QPointF& point) const
{
    for (int i = m_shapes.count() - 1; i >= 0; --i)
    {
        Shape* shape = m_shapes.at(i);
        QRectF rect  = shape->boundingRect();
        if (rect.contains(point))
        {
            return shape;
        }
    }

    return nullptr;
}

QRectF xcanvas::Shapes::boundingRect() const
{
    if (m_shapes.isEmpty())
    {
        return QRectF();
    }

    QRectF rect = m_shapes.first()->boundingRect();
    for (int i = 1; i < m_shapes.count(); ++i)
    {
        rect = rect.united(m_shapes.at(i)->boundingRect());
    }
    return rect;
}

void xcanvas::Shapes::selectAll()
{
    setAllSelected(true);
}

void xcanvas::Shapes::deselectAll()
{
    setAllSelected(false);
}

void xcanvas::Shapes::setAllSelected(bool selected)
{
    for (Shape* shape : m_shapes)
    {
        shape->setSelected(selected);
    }
}

void xcanvas::Shapes::selectInRect(const QRectF& rect)
{
    deselectAll();

    for (Shape* shape : m_shapes)
    {
        if (rect.contains(shape->boundingRect()))
        {
            shape->setSelected(true);
        }
    }
}

void xcanvas::Shapes::invertSelection()
{
    for (Shape* shape : m_shapes)
    {
        shape->setSelected(!shape->isSelected());
    }
}

QVector<xcanvas::Shape*> xcanvas::Shapes::selectedShapes() const
{
    QVector<Shape*> selected;

    for (Shape* shape : m_shapes)
    {
        if (shape->isSelected())
        {
            selected.append(shape);
        }
    }

    return selected;
}

bool xcanvas::Shapes::hasSelection() const
{
    for (Shape* shape : m_shapes)
    {
        if (shape->isSelected())
        {
            return true;
        }
    }

    return false;
}

int xcanvas::Shapes::selectedCount() const
{
    int count = 0;

    for (Shape* shape : m_shapes)
    {
        if (shape->isSelected())
        {
            ++count;
        }
    }

    return count;
}

QRectF xcanvas::Shapes::selectedBoundingRect() const
{
    QRectF rect;
    bool   first = true;

    for (Shape* shape : m_shapes)
    {
        if (shape->isSelected())
        {
            if (first)
            {
                rect  = shape->boundingRect();
                first = false;
            }
            else
            {
                rect = rect.united(shape->boundingRect());
            }
        }
    }

    return rect;
}

void xcanvas::Shapes::translate(const QPointF& offset)
{
    for (Shape* shape : m_shapes)
    {
        shape->translate(offset);
    }
}
