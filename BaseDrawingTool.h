#ifndef BASEDRAWINGTOOL_H
#define BASEDRAWINGTOOL_H

class QMouseEvent;
class QRectF;
class MyGraphicsView;
class QGraphicsPathItem;

class BaseDrawingTool
{
public:
    BaseDrawingTool(MyGraphicsView* pView);
    virtual ~BaseDrawingTool();

    virtual void mousePressEvent(QMouseEvent* event)   = 0;
    virtual void mouseMoveEvent(QMouseEvent* event)    = 0;
    virtual void mouseReleaseEvent(QMouseEvent* event) = 0;
    virtual int  ToolType()                            = 0;

    virtual void GetTraceRects(const QRectF& rect, QRectF rects[9]);

    virtual void DrawTrace();
    virtual void ClearTrace();

protected:
    MyGraphicsView* m_pView;

private:
    QGraphicsPathItem* m_pTraceItem;
};

#endif // BASEDRAWINGTOOL_H
