#ifndef SELECTDRAWINGTOOL_H
#define SELECTDRAWINGTOOL_H

#include "DrawingTool.h"
#include <QPointF>

namespace xcanvas
{
class Shape;
}

namespace xcanvas
{
class SelectTool : public DrawingTool
{
    Q_OBJECT
  public:
    explicit SelectTool(MyGraphicsView* view, Canvas* canvas);
    virtual ~SelectTool() override;

    virtual void            mousePressEvent(QMouseEvent* event) override;
    virtual void            mouseMoveEvent(QMouseEvent* event) override;
    virtual void            mouseReleaseEvent(QMouseEvent* event) override;
    virtual void            keyPressEvent(QKeyEvent* event) override;
    virtual void            drawPreview(QPainter* painter) override;
    virtual DrawingToolType toolType() override;

  private:
    int    hitTraceHandle(const QPointF& pos) const;
    void   setCanvasCursorShape(int nHitPos);
    Shape* hitUnselectedShape(QPointF pos);
    void   updateHighlight(const Shape& shape);
    void   clearHighlight();
    void   updateSelectionRect(const QRectF& rect);
    void   clearSelectionRect();

  private:
    QPointF      m_dragStartPos;
    bool         m_bMovingItem;
    bool         m_rotating;
    QPointF      m_rotationCenter;
    QPainterPath m_highlightPath;
    QPainterPath m_selectionRectPath;
};
}// namespace xcanvas

#endif// SELECTDRAWINGTOOL_H
