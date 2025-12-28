#ifndef SHAPES_H
#define SHAPES_H

#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QSet>
#include <map>
#include <memory>

namespace xcanvas
{
    class Shape;
    struct ShapeState;
}

namespace xcanvas
{
    using ShapeList = QVector<Shape*>;

class ShapeManager
{
  public:
    ShapeManager();
    ~ShapeManager();

    // 禁止拷贝（因为涉及指针管理）
    ShapeManager(const ShapeManager&)            = delete;
    ShapeManager& operator=(const ShapeManager&) = delete;

    void addShape(Shape* shape);
    void append(const ShapeList& shapes);
    void removeShape(Shape* shape);
    void removeShapes(const ShapeList& shapes);
    void deleteShape(Shape* shape);
    void clear();

    int count() const;
    int shapeCount() const;
    bool isEmpty() const;

    Shape* shapeAt(int index) const;
    Shape* operator[](int index) const;

    Shape* shapeAt(const QPointF& point) const;

    ShapeList shapes() const;

    QRectF boundingRect() const;

    void            selectShape(Shape* shape, bool replace = false);
    void            deselectShape(Shape* shape);
    void            selectInRect(const QRectF& rect);
    void            invertSelection();
    void            selectAll();
    void            deselectAll();
    const QSet<Shape*>& selectedShapes() const;
    const ShapeList selectedShapeList() const;
    bool            hasSelection() const;
    int             selectedCount() const;
    void            clearSelection();
    QRectF          selectedBoundingRect() const;
    void invalidateSelectedRect();

    void translate(const QPointF& offset);
    void translateSelected(const QPointF& offset, const std::map<Shape*, std::unique_ptr<ShapeState>>& initialStates);
    void rotateSelected(double angle, const QPointF& center, const std::map<Shape*, std::unique_ptr<ShapeState>>& initialStates);
    void scaleSelected(double sx, double sy, const QPointF &anchor, const std::map<Shape*, std::unique_ptr<ShapeState>>& initialStates);

  private:
    void deleteAllShapes();
    void clearSelectionInternal();

  private:
    ShapeList m_shapes;
    mutable QRectF m_cachedSelectedRect;
    mutable bool m_isSelectedRectDirty;
    QSet<Shape*> m_selectedShapes;
};

}// namespace xcanvas

#endif// SHAPES_H
