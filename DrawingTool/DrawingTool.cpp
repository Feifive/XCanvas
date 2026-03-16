#include "DrawingTool.h"
#include "../MyGraphicsView.h"
#include "Global.h"
#include <QDebug>
#include <QGraphicsPathItem>
#include <QMouseEvent>
#include <QRectF>
#include "AppSettings.h"

const double RIGHT_DRAG_THRESHOLD = 3.0;

xcanvas::DrawingTool::DrawingTool(MyGraphicsView* canvasView, Canvas* canvas) : m_canvasView(canvasView), m_canvas(canvas), m_state(State::Idle), m_isRightPressed(false), m_isRightDragged(false)

{
}

xcanvas::DrawingTool::~DrawingTool()
{
}

void xcanvas::DrawingTool::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        cancelDrawing();
    }
}

void xcanvas::DrawingTool::drawPreview(QPainter* painter)
{
    if (m_state == State::Drawing && !m_previewPath.isEmpty())
    {
        painter->save();
        QPen pen(AppSettings::instance().activeColor());
        pen.setWidth(1);
        pen.setCosmetic(true);
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawPath(m_previewPath);
        painter->restore();
    }
}

void xcanvas::DrawingTool::cancelDrawing()
{
    resetDrawingState();
    emit drawingCanceled();
}

void xcanvas::DrawingTool::finishDrawing()
{
    resetDrawingState();
    emit drawingCompleted();
}

void xcanvas::DrawingTool::resetDrawingState()
{
    m_state = State::Idle;
    m_previewPath = QPainterPath();
    m_canvasView->requestFullUpdate();
}

void xcanvas::DrawingTool::handleRightButtonPress(QMouseEvent* event)
{
    m_isRightPressed = true;
    m_isRightDragged = false;
    m_rightPressPos  = event->pos();
}

void xcanvas::DrawingTool::handleRightButtonMove(QMouseEvent* event)
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

void xcanvas::DrawingTool::handleRightButtonRelease(QMouseEvent* event)
{
    if (!m_isRightPressed)
    {
        return;
    }

    // 拖动右键 → 拖动画布，不中断
    // 单击右键 → 不再取消当前绘图工具，仅结束本次右键状态
    Q_UNUSED(event);
    m_isRightPressed = false;
}
