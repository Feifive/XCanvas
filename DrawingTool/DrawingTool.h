#ifndef BASEDRAWINGTOOL_H
#define BASEDRAWINGTOOL_H

#include <QObject>
#include <QPainterPath>
#include "Global.h"

class QMouseEvent;
class QRectF;
class MyGraphicsView;
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
        DrawingTool(MyGraphicsView* pView);
        virtual ~DrawingTool();

        virtual void mousePressEvent(QMouseEvent* event) {};
        virtual void mouseMoveEvent(QMouseEvent* event) {};
        virtual void mouseReleaseEvent(QMouseEvent* event) {};
        virtual void drawPreview(QPainter* painter);
        virtual DrawingToolType toolType() = 0;

    protected:
        virtual void cancelDrawing();

        void handleRightButtonPress(QMouseEvent* event);
        void handleRightButtonMove(QMouseEvent* event);
        void handleRightButtonRelease(QMouseEvent* event);

    signals:
        void toolFinished();

    protected:
        MyGraphicsView* m_pView;
        QPointF m_startPos;
        QPainterPath m_previewPath;
        State m_state;

        // 右键逻辑
        bool m_isRightPressed;
        bool m_isRightDragged;
        QPointF m_rightPressPos;
    };
}


#endif // BASEDRAWINGTOOL_H
