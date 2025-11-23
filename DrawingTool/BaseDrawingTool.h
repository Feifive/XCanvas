#ifndef BASEDRAWINGTOOL_H
#define BASEDRAWINGTOOL_H

#include <QObject>
#include "Global.h"

class QMouseEvent;
class QRectF;
class MyGraphicsView;
class QGraphicsPathItem;

class BaseDrawingTool : public QObject
{
    Q_OBJECT

public:

    enum class State
    {
        Idle,
        Drawing,
        Interrupted
    };

public:
    BaseDrawingTool(MyGraphicsView* pView);
    virtual ~BaseDrawingTool();

    virtual void mousePressEvent(QMouseEvent* event){};
    virtual void mouseMoveEvent(QMouseEvent* event){};
    virtual void mouseReleaseEvent(QMouseEvent* event){};
    virtual int  ToolType()      = 0;

    virtual void GetTraceRects(const QRectF& rect, QRectF rects[9]);

    virtual void DrawTrace();
    virtual void ClearTrace();

protected:
    virtual void CancelDrawing() {};


    void HandleRightButtonPress(QMouseEvent* event);
    void HandleRightButtonMove(QMouseEvent* event);
    void HandleRightButtonRelease(QMouseEvent* event);

signals:
    void ToolFinished();

protected:
    MyGraphicsView*    m_pView;
    State m_state;

    // 右键逻辑
    bool m_bRightPressed;
    bool m_bRightDragged;
    QPointF m_rightPressPos;

private:
    QGraphicsPathItem* m_pTraceItem;
};

#endif // BASEDRAWINGTOOL_H
