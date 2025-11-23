#ifndef RECTDRAWINGTOOL_H
#define RECTDRAWINGTOOL_H

#include "BaseDrawingTool.h"
#include <QObject>
#include <QPointF>

class MyGraphicsView;
class QGraphicsRectItem;

class RectDrawingTool : public BaseDrawingTool
{
    Q_OBJECT
  public:
    explicit RectDrawingTool(MyGraphicsView* pView);
    virtual ~RectDrawingTool() override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual int  ToolType() override;

protected:
    void CancelDrawing() override;

  private:
    QGraphicsPathItem* m_pTempPolylineItem;
    QPointF            m_startPos;
};

#endif// RECTDRAWINGTOOL_H
