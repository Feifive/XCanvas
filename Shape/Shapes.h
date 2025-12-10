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
class Shapes
{
  public:
    Shapes();
    ~Shapes();

    // 禁止拷贝（因为涉及指针管理）
    Shapes(const Shapes&)            = delete;
    Shapes& operator=(const Shapes&) = delete;

    void addShape(Shape* shape);
    void removeShape(Shape* shape);
    void deleteShape(Shape* shape);
    void clear();
    void append(const QVector<Shape*>& shapes);

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

    QVector<Shape*> shapes() const
    {
        return m_shapes;
    }

    QRectF boundingRect() const;

    void            selectAll();
    void            deselectAll();
    void            setAllSelected(bool selected);
    void            selectInRect(const QRectF& rect);
    void            invertSelection();
    QVector<Shape*> selectedShapes() const;
    bool            hasSelection() const;
    int             selectedCount() const;
    QRectF          selectedBoundingRect() const;

    void translate(const QPointF& offset);

  private:
    void deleteAllShapes();

  private:
    QVector<Shape*> m_shapes;
};

}// namespace xcanvas

#endif// SHAPES_H
