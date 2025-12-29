#ifndef SELECTDRAWINGTOOL_H
#define SELECTDRAWINGTOOL_H

#include "DrawingTool.h"
#include <QPointF>

namespace xcanvas
{
class Shape;
struct ShapeState;
}

namespace xcanvas
{
class SelectTool final : public DrawingTool
{
    Q_OBJECT
public:
    explicit SelectTool(MyGraphicsView* view, Canvas* canvas);
    ~SelectTool() override;

    void            mousePressEvent(QMouseEvent* event) override;
    void            mouseMoveEvent(QMouseEvent* event) override;
    void            mouseReleaseEvent(QMouseEvent* event) override;
    void            keyPressEvent(QKeyEvent* event) override;
    void            drawPreview(QPainter* painter) override;
    DrawingToolType toolType() override;

private:
    int    hitTraceHandle(const QPointF& pos) const;
    void   setCanvasCursorShape(int nHitPos);
    Shape* hitUnselectedShape(QPointF pos);
    void   updateHighlight(const Shape& shape);
    void   clearHighlight();
    void   updateSelectionRect(const QRectF& rect);
    void   clearSelectionRect();
    QPointF getAnchorPoint(int handle, QRectF rect) const;

private:
    QPointF      m_dragStartPos;
    bool         m_bMovingItem;
    bool         m_rotating;
    QPointF      m_rotationCenter;
    bool         m_resizing;
    int          m_resizeHandle;
    double       m_startClickDistX;
    double       m_startClickDistY;
    QPointF      m_anchorPoint;
    QRectF       m_initialSelectedRect;
    QPainterPath m_highlightPath;
    QPainterPath m_selectionRectPath;
    int m_lastHitPos;
    Shape* m_lastHighlightedShape;
    std::map<Shape*, QTransform> m_initialTransforms;
};
}// namespace xcanvas

#endif// SELECTDRAWINGTOOL_H
