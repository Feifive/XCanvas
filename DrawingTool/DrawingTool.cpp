#include "DrawingTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "EventBus.h"
#include <QRectF>
#include <QGraphicsPathItem>
#include <QMouseEvent>
#include <QDebug>

const double RIGHT_DRAG_THRESHOLD = 3.0;

xcanvas::DrawingTool::DrawingTool(MyGraphicsView *pView) :
    m_pView(pView),
    m_state(State::Idle),
    m_isRightPressed(false),
    m_isRightDragged(false)

{}

xcanvas::DrawingTool::~DrawingTool()
{
}

void xcanvas::DrawingTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        if (m_state == State::Drawing)
        {
            m_state = State::Interrupted;
            cancelDrawing();
        }
        else
        {
            emit EventBus::instance().finishDrawing();
        }
    }
}

void xcanvas::DrawingTool::drawPreview(QPainter *painter)
{
    if (m_state == State::Drawing && !m_previewPath.isEmpty())
    {
        painter->save();
        painter->setPen(normalPen());
        painter->drawPath(m_previewPath);
        painter->restore();
    }
}

void xcanvas::DrawingTool::cancelDrawing()
{
    m_state       = State::Idle;
    m_previewPath = QPainterPath();
    m_pView->updateCanvas();
}

void xcanvas::DrawingTool::handleRightButtonPress(QMouseEvent *event)
{
    m_isRightPressed  = true;
    m_isRightDragged  = false;
    m_rightPressPos  = event->pos();
}

void xcanvas::DrawingTool::handleRightButtonMove(QMouseEvent *event)
{
    if (!m_isRightPressed)
    {
        return;
    }

    QPointF delta = event->pos() - m_rightPressPos;

    if (!m_isRightDragged && (qAbs(delta.x()) > RIGHT_DRAG_THRESHOLD || qAbs(delta.y()) > RIGHT_DRAG_THRESHOLD))
    {
        m_isRightDragged = true;
    }
}

void xcanvas::DrawingTool::handleRightButtonRelease(QMouseEvent *event)
{
    if (!m_isRightPressed)
    {
        return;
    }

    // 拖动右键 → 拖动画布，不中断
    if (m_isRightDragged)
    {
        m_isRightPressed = false;
        return;
    }

    // 未拖动 → 右键单击逻辑
    if (m_state == State::Drawing)
    {
        // 中断绘制
        m_state = State::Interrupted;
        cancelDrawing();
    }
    else
    {
        // 第二次右键 → 完全结束工具
        emit EventBus::instance().finishDrawing();
    }

    m_isRightPressed = false;
}
