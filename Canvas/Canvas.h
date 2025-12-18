#ifndef CANVAS_H
#define CANVAS_H

#include "ShapeManager.h"

class QUndoStack;

#include <QObject>
#include <QRectF>

namespace xcanvas
{

class Canvas final : public QObject
{
    Q_OBJECT
  public:
    explicit Canvas(QObject* parent = nullptr);
    ~Canvas() override;

    QRectF canvasRect() const;
    void   setCanvasRect(const QRectF& rect);

    ShapeManager* shapeManager() const;
    QUndoStack*   undoStack() const;
    void          addShape(xcanvas::Shape* shape);
    void          addShapes(const xcanvas::ShapeList& shapeList);
    void          removeShape(xcanvas::Shape* shape);
    void          removeShapes(const xcanvas::ShapeList& shapeList);

  private:
    QRectF        m_canvasRect;
    ShapeManager* m_shapeManager;
    QUndoStack*   m_undoStack;
};

}// namespace xcanvas

#endif//CANVAS_H
