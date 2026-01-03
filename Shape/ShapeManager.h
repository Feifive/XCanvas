#ifndef SHAPES_H
#define SHAPES_H

#include "SelectionSummary.h"
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QSet>
#include <map>
#include <QObject>

namespace xcanvas
{
    class Shape;
    struct ShapeState;
}

namespace xcanvas
{
    using ShapeList = QVector<Shape*>;

class ShapeManager final : public QObject
{
    Q_OBJECT
  public:
    explicit ShapeManager(QObject* parent = nullptr);
    ~ShapeManager() override;

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
    SelectionSummary selectionSummary() const;
    void invalidateSelectedRect() const;
    void translate(const QPointF& offset);

signals:
    void selectionChanged();

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
