#ifndef SELECTDRAWINGTOOL_H
#define SELECTDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include "BaseDrawingTool.h"

class MyGraphicsView;
class QGraphicsRectItem;
class BaseShape;

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
    BaseShape* HitUnselectedShape(QPointF pos);
    void AddHighlightItemToCanvas(const QRectF& rect);
    void RemoveHighlightItemFromCanvas();

private:
    QGraphicsRectItem* m_pDrawingItem;
    bool m_bDrawing;
    QPointF m_startPos;
    QGraphicsRectItem* m_pHighlightItem;
};

#endif // SELECTDRAWINGTOOL_H
