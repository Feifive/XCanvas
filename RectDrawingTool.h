#ifndef RECTDRAWINGTOOL_H
#define RECTDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include "BaseDrawingTool.h"

class MyGraphicsView;
class QGraphicsRectItem;

class RectDrawingTool : public QObject, public BaseDrawingTool
{
    Q_OBJECT
public:
    explicit RectDrawingTool(MyGraphicsView* pView);
    virtual ~RectDrawingTool() override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual int  ToolType() override;

private:
    QGraphicsRectItem* m_pDrawingItem;
    bool               m_bDrawing;
    QPointF            m_startPos;
};

#endif // RECTDRAWINGTOOL_H
