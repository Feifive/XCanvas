#include "ShapeManager.h"
#include "Shape.h"
#include <QDebug>
#include <QGraphicsItem>
#include <QPainterPath>

namespace xcanvas {
    ShapeManager::ShapeManager()
    {
    }

    ShapeManager::~ShapeManager()
    {
    }

    void ShapeManager::addShape(Shape* shape)
    {
        if (shape)
        {
            m_shapes.append(shape);
        }
    }

    void ShapeManager::append(const ShapeList& shapes)
    {
        m_shapes.append(shapes);
    }

    void ShapeManager::removeShape(Shape* shape)
    {
        m_shapes.removeOne(shape);
    }

    void ShapeManager::removeAllShapes(const ShapeList &shapes) {
        for (Shape* shape : shapes) {
            m_shapes.removeOne(shape);
        }
    }

    void ShapeManager::deleteShape(Shape* shape)
    {
        if (m_shapes.removeOne(shape))
        {
            delete shape;
        }
    }

    void ShapeManager::clear()
    {
        deleteAllShapes();
        m_shapes.clear();
    }

    void ShapeManager::deleteAllShapes()
    {
        for (Shape* shape : m_shapes)
        {
            delete shape;
        }
    }

    int ShapeManager::count() const
    {
        return m_shapes.count();
    }

    bool ShapeManager::isEmpty() const
    {
        return m_shapes.isEmpty();
    }

    Shape* ShapeManager::shapeAt(int index) const
    {
        if (index >= 0 && index < m_shapes.count())
        {
            return m_shapes.at(index);
        }
        return nullptr;
    }

    Shape* ShapeManager::shapeAt(const QPointF& point) const
    {
        for (int i = m_shapes.count() - 1; i >= 0; --i)
        {
            Shape* shape = m_shapes.at(i);
            QRectF rect = shape->boundingRect();
            if (rect.contains(point))
            {
                return shape;
            }
        }

        return nullptr;
    }

    QRectF ShapeManager::boundingRect() const
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

    void ShapeManager::selectAll()
    {
        setAllSelected(true);
    }

    void ShapeManager::deselectAll()
    {
        setAllSelected(false);
    }

    void ShapeManager::setAllSelected(bool selected)
    {
        for (Shape* shape : m_shapes)
        {
            shape->setSelected(selected);
        }
    }

    void ShapeManager::selectInRect(const QRectF& rect)
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

    void ShapeManager::invertSelection()
    {
        for (Shape* shape : m_shapes)
        {
            shape->setSelected(!shape->isSelected());
        }
    }

    ShapeList ShapeManager::selectedShapes() const
    {
        ShapeList selected;

        for (Shape* shape : m_shapes)
        {
            if (shape->isSelected())
            {
                selected.append(shape);
            }
        }

        return selected;
    }

    bool ShapeManager::hasSelection() const
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

    int ShapeManager::selectedCount() const
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

    QRectF ShapeManager::selectedBoundingRect() const
    {
        QRectF rect;
        bool   first = true;

        for (const Shape* shape : m_shapes)
        {
            if (shape->isSelected())
            {
                if (first)
                {
                    rect = shape->boundingRect();
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

    void ShapeManager::translate(const QPointF& offset)
    {
        for (Shape* shape : m_shapes)
        {
            shape->translate(offset);
        }
    }
} // namespace xcanvas
