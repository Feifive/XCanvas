#ifndef BASEDRAWINGTOOL_H
#define BASEDRAWINGTOOL_H

#include "Global.h"
#include <QObject>
#include <QPainterPath>
#include <qevent.h>

namespace xcanvas
{
class Canvas;
}
class MyGraphicsView;

class QMouseEvent;
class QRectF;
class QGraphicsPathItem;
class QPainter;

namespace xcanvas
{
class DrawingTool : public QObject
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
    DrawingTool(MyGraphicsView* canvasView, Canvas* canvas);
    virtual ~DrawingTool();

    virtual void            mousePressEvent(QMouseEvent* event) {};
    virtual void            mouseMoveEvent(QMouseEvent* event) {};
    virtual void            mouseReleaseEvent(QMouseEvent* event) {};
    virtual void            keyPressEvent(QKeyEvent* event);
    virtual void            drawPreview(QPainter* painter);
    virtual DrawingToolType toolType() = 0;

  protected:
    virtual void cancelDrawing();

    void handleRightButtonPress(QMouseEvent* event);
    void handleRightButtonMove(QMouseEvent* event);
    void handleRightButtonRelease(QMouseEvent* event);

  signals:
    void finished();

  protected:
    MyGraphicsView* m_canvasView;
    Canvas*         m_canvas;
    QPointF         m_mousePos;
    QPainterPath    m_previewPath;
    State           m_state;

    // 右键逻辑
    bool    m_isRightPressed;
    bool    m_isRightDragged;
    QPointF m_rightPressPos;
};
}// namespace xcanvas

#endif// BASEDRAWINGTOOL_H
