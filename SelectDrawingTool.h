#ifndef SELECTDRAWINGTOOL_H
#define SELECTDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include "BaseDrawingTool.h"

class MyGraphicsView;
class QGraphicsRectItem;
class QGraphicsPathItem;
class Shape;

class SelectDrawingTool : public QObject, public BaseDrawingTool
{
    Q_OBJECT
public:
    explicit SelectDrawingTool(MyGraphicsView* pView);
    virtual ~SelectDrawingTool() override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual int  ToolType() override;

private:
    int HitSelectedShapeTrace(QPointF pos);
    void SetCanvasCursorShape(int nHitPos);
    Shape* HitUnselectedShape(QPointF pos);
    void AddHighlightItemToCanvas(const Shape* pShape);
    void RemoveHighlightItemFromCanvas();

private:
    QGraphicsRectItem* m_pDrawingItem;
    bool m_bDrawing;
    bool m_bMovingItem;
    QPointF m_startPos;
    QGraphicsPathItem* m_pHighlightItem;
};

#endif // SELECTDRAWINGTOOL_H
