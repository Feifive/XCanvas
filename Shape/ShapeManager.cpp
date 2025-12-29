#include "ShapeManager.h"
#include "Shape.h"
#include <QDebug>
#include <QGraphicsItem>
#include <QPainterPath>

namespace xcanvas {
    ShapeManager::ShapeManager() : m_isSelectedRectDirty(true)
    {
    }

    ShapeManager::~ShapeManager()
    {
    }

    void ShapeManager::addShape(Shape* shape)
    {
        if (!shape) {
            return;
        }

        m_shapes.append(shape);
        if (shape->isSelected()) {
            m_selectedShapes.insert(shape);
            m_isSelectedRectDirty = true;
        }
    }

    void ShapeManager::append(const ShapeList& shapes)
    {
        if (shapes.isEmpty()) {
            return;
        }

        m_shapes.reserve(m_shapes.size() + shapes.size());
        m_shapes.append(shapes);

        bool hasSelected = false;
        m_selectedShapes.reserve(qMin(1000, shapes.size()));
        for (Shape* shape : shapes)
        {
            if (shape->isSelected())
            {
                m_selectedShapes.insert(shape);
                hasSelected = true;
            }
        }

        if (hasSelected)
        {
            m_isSelectedRectDirty = true;
        }
    }

    void ShapeManager::removeShape(Shape* shape)
    {
        m_selectedShapes.remove(shape);
        m_shapes.removeOne(shape);
        m_isSelectedRectDirty = true;
    }

    void ShapeManager::removeShapes(const ShapeList &shapes) {
        for (Shape* s : shapes) {
            m_selectedShapes.remove(s);
        }

        const QSet<Shape*> toRemoveSet(shapes.begin(), shapes.end());

        m_shapes.erase(std::remove_if(m_shapes.begin(), m_shapes.end(),
            [&](Shape* s) { return toRemoveSet.contains(s); }), m_shapes.end());

        m_isSelectedRectDirty = true;
    }

    void ShapeManager::deleteShape(Shape* shape)
    {
        if (m_shapes.removeOne(shape))
        {
            m_selectedShapes.remove(shape);
            m_isSelectedRectDirty = true;
            delete shape;
        }
    }

    void ShapeManager::clear()
    {
        deleteAllShapes();
        m_shapes.clear();
        m_selectedShapes.clear();
        m_isSelectedRectDirty = true;
    }

    void ShapeManager::deleteAllShapes()
    {
        for (const Shape* shape : m_shapes)
        {
            delete shape;
        }
    }

    void ShapeManager::clearSelectionInternal() {
        for (Shape* shape : m_selectedShapes) {
            shape->setSelected(false);
        }
        m_selectedShapes.clear();
    }

    int ShapeManager::count() const
    {
        return m_shapes.count();
    }

    int ShapeManager::shapeCount() const {
        return count();
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

    Shape * ShapeManager::operator[](int index) const {
        return shapeAt(index);
    }

    Shape* ShapeManager::shapeAt(const QPointF& point) const
    {
        for (int i = m_shapes.count() - 1; i >= 0; --i)
        {
            Shape* shape = m_shapes.at(i);
            if (QRectF rect = shape->boundingRect(); rect.contains(point))
            {
                return shape;
            }
        }

        return nullptr;
    }

    ShapeList ShapeManager::shapes() const {
        return m_shapes;
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
        if (m_shapes.isEmpty()) {
            return;
        }

        if (m_selectedShapes.size() == m_shapes.size()) {
            return;
        }

        m_selectedShapes.reserve(m_shapes.size());
        for (Shape* shape : m_shapes) {
            if (!shape->isSelected()) {
                shape->setSelected(true);
                m_selectedShapes.insert(shape);
            }
        }

        m_isSelectedRectDirty = true;
    }

    void ShapeManager::deselectAll()
    {
        if (m_selectedShapes.isEmpty()) {
            return;
        }

        for (Shape* shape : m_selectedShapes) {
            shape->setSelected(false);
        }
        m_selectedShapes.clear();
        m_isSelectedRectDirty = true;
    }

    void ShapeManager::selectInRect(const QRectF& rect)
    {
        deselectAll();

        m_selectedShapes.reserve(qMin(1000, m_shapes.size()));

        for (Shape* shape : m_shapes)
        {
            if (rect.contains(shape->boundingRect()))
            {
                shape->setSelected(true);
                m_selectedShapes.insert(shape);
            }
        }
        m_isSelectedRectDirty = true;
    }

    void ShapeManager::invertSelection()
    {
        for (Shape* shape : m_shapes) {
            if (shape->isSelected()) {
                shape->setSelected(false);
                m_selectedShapes.remove(shape);
            } else {
                shape->setSelected(true);
                m_selectedShapes.insert(shape);
            }
        }
        m_isSelectedRectDirty = true;
    }

    const QSet<Shape *> & ShapeManager::selectedShapes() const{
        return m_selectedShapes;
    }

    const ShapeList ShapeManager::selectedShapeList() const {
        return {m_selectedShapes.begin(), m_selectedShapes.end()};
    }

    bool ShapeManager::hasSelection() const
    {
        return !m_selectedShapes.isEmpty();
    }

    int ShapeManager::selectedCount() const
    {
        return m_selectedShapes.count();
    }

    void ShapeManager::selectShape(Shape *shape, const bool replace) {
        if (!shape) {
            return;
        }

        if (replace) {
            clearSelectionInternal(); // 如果是单选模式，先清空之前的
        }

        if (!m_selectedShapes.contains(shape)) {
            shape->setSelected(true);
            m_selectedShapes.insert(shape);
            m_isSelectedRectDirty = true;
        }
    }

    void ShapeManager::deselectShape(Shape *shape) {
        if (m_selectedShapes.remove(shape)) {
            shape->setSelected(false);
            m_isSelectedRectDirty = true;
        }
    }

    void ShapeManager::clearSelection() {
        if (m_selectedShapes.isEmpty()) {
            return;
        }
        clearSelectionInternal();
        m_isSelectedRectDirty = true;
    }

    QRectF ShapeManager::selectedBoundingRect() const
    {
        if (!m_isSelectedRectDirty) {
            return m_cachedSelectedRect;
        }

        m_cachedSelectedRect = QRectF();
        if (m_selectedShapes.isEmpty()) {
            m_isSelectedRectDirty = false;
            return m_cachedSelectedRect;
        }

        for (const Shape* shape : m_selectedShapes) {
            m_cachedSelectedRect |= shape->boundingRect();
        }

        m_isSelectedRectDirty = false;
        return m_cachedSelectedRect;
    }

    void ShapeManager::invalidateSelectedRect() const {
        m_isSelectedRectDirty = true;
    }

    void ShapeManager::translate(const QPointF& offset)
    {
        for (Shape* shape : m_shapes)
        {
            shape->translate(offset);
        }
    }

} // namespace xcanvas
