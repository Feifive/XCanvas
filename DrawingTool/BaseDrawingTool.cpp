#include "BaseDrawingTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include <QRectF>
#include <QGraphicsPathItem>
#include <QMouseEvent>
#include <QDebug>

const double RIGHT_DRAG_THRESHOLD = 3.0;

BaseDrawingTool::BaseDrawingTool(MyGraphicsView *pView) :
    m_pView(pView),
    m_pTraceItem(nullptr),
    m_state(State::Idle),
    m_bRightPressed(false),
    m_bRightDragged(false)

{}

BaseDrawingTool::~BaseDrawingTool()
{
    ClearTrace();
}

void BaseDrawingTool::GetTraceRects(const QRectF &rect, QRectF rects[])
{
#define SIZE 3
    double dScale     = m_pView->GetScaleFactory();
    double dRectSize  = SIZE / dScale;
    double dRectWidth = dRectSize * 2;

    rects[ERECT_TOP_LEFT]     = QRectF(rect.left() - dRectWidth, rect.top() - dRectWidth, dRectWidth, dRectWidth);
    rects[ERECT_TOP_MID]      = QRectF(rect.center().x() - dRectSize, rect.top() - dRectWidth, dRectWidth, dRectWidth);
    rects[ERECT_TOP_RIGHT]    = QRectF(rect.right(), rect.top() - dRectWidth, dRectWidth, dRectWidth);
    rects[ERECT_MID_LEFT]     = QRectF(rect.left() - dRectWidth, rect.center().y() - dRectSize, dRectWidth, dRectWidth);
    rects[ERECT_CENTER]       = QRectF(rect.center().x() - dRectSize, rect.center().y() - dRectSize, dRectWidth, dRectWidth);
    rects[ERECT_MID_RIGHT]    = QRectF(rect.right(), rect.center().y() - dRectSize, dRectWidth, dRectWidth);
    rects[ERECT_BOTTOM_LEFT]  = QRectF(rect.left() - dRectWidth, rect.bottom(), dRectWidth, dRectWidth);
    rects[ERECT_BOTTOM_MID]   = QRectF(rect.center().x() - dRectSize, rect.bottom(), dRectWidth, dRectWidth);
    rects[ERECT_BOTTOM_RIGHT] = QRectF(rect.right(), rect.bottom(), dRectWidth, dRectWidth);
}

void BaseDrawingTool::DrawTrace()
{
    ClearTrace();

    QRectF rect   = m_pView->GetCurrentShapes()->GetSelectedShapeRect();
    double dScale = m_pView->GetScaleFactory();

    if(!rect.isValid())
    {
        return;
    }

    double dLineLength = 6 / dScale;
    QRectF rects[9];

    GetTraceRects(rect, rects);

    if(!m_pTraceItem)
    {
        m_pTraceItem = new QGraphicsPathItem;
    }
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidthF(2 / dScale);
    m_pTraceItem->setPen(pen);
    m_pTraceItem->setBrush(Qt::black);

    QPainterPath paintPath;
    for (int i = 0; i < 9; ++i)
    {
        if(i == ERECT_CENTER)
        {
            qreal xCenter = rect.center().x();
            qreal yCenter = rect.center().y();
            paintPath.moveTo(xCenter - dLineLength, yCenter - dLineLength);
            paintPath.lineTo(xCenter + dLineLength, yCenter + dLineLength);
            paintPath.moveTo(xCenter + dLineLength, yCenter - dLineLength);
            paintPath.lineTo(xCenter - dLineLength, yCenter + dLineLength);
            continue;
        }
        paintPath.addRect(rects[i]);
    }

    m_pTraceItem->setPath(paintPath);

    m_pView->scene()->addItem(m_pTraceItem);
}

void BaseDrawingTool::ClearTrace()
{
    if(m_pTraceItem)
    {
        m_pView->scene()->removeItem(m_pTraceItem);
        delete m_pTraceItem;
        m_pTraceItem = nullptr;
    }
}

void BaseDrawingTool::HandleRightButtonPress(QMouseEvent *event)
{
    m_bRightPressed  = true;
    m_bRightDragged  = false;
    m_rightPressPos  = event->pos();
}

void BaseDrawingTool::HandleRightButtonMove(QMouseEvent *event)
{
    if (!m_bRightPressed)
    {
        return;
    }

    QPointF delta = event->pos() - m_rightPressPos;

    if (!m_bRightDragged && (qAbs(delta.x()) > RIGHT_DRAG_THRESHOLD || qAbs(delta.y()) > RIGHT_DRAG_THRESHOLD))
    {
        m_bRightDragged = true;
    }
}

void BaseDrawingTool::HandleRightButtonRelease(QMouseEvent *event)
{
    if (!m_bRightPressed)
    {
        return;
    }

    // 拖动右键 → 拖动画布，不中断
    if (m_bRightDragged)
    {
        m_bRightPressed = false;
        return;
    }

    // 未拖动 → 右键单击逻辑
    if (m_state == State::Drawing)
    {
        // 中断绘制
        m_state = State::Interrupted;
        CancelDrawing();
    }
    else
    {
        // 第二次右键 → 完全结束工具
        emit ToolFinished();
    }

    m_bRightPressed = false;
}
