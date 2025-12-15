#ifndef SHAPES_H
#define SHAPES_H

#include <QPointF>
#include <QRectF>
#include <QVector>

namespace xcanvas
{
    class Shape;
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
    void removeShape(Shape* shape);
    void deleteShape(Shape* shape);
    void clear();
    void append(const ShapeList& shapes);

    int count() const;
    int shapeCount() const
    {
        return count();
    }
    bool isEmpty() const;

    Shape* shapeAt(int index) const;
    Shape* operator[](int index) const
    {
        return shapeAt(index);
    }

    Shape* shapeAt(const QPointF& point) const;

    ShapeList shapes() const
    {
        return m_shapes;
    }

    QRectF boundingRect() const;

    void            selectAll();
    void            deselectAll();
    void            setAllSelected(bool selected);
    void            selectInRect(const QRectF& rect);
    void            invertSelection();
    ShapeList       selectedShapes() const;
    bool            hasSelection() const;
    int             selectedCount() const;
    QRectF          selectedBoundingRect() const;

    void translate(const QPointF& offset);

  private:
    void deleteAllShapes();

  private:
      ShapeList m_shapes;
};

}// namespace xcanvas

#endif// SHAPES_H
