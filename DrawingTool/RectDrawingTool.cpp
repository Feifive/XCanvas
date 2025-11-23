#include "RectDrawingTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "PolylineShape.h"
#include <QDebug>
#include <QMouseEvent>

RectDrawingTool::RectDrawingTool(MyGraphicsView* pView) : BaseDrawingTool(pView), m_pTempPolylineItem(nullptr), m_startPos(-1, -1)
{
}

RectDrawingTool::~RectDrawingTool()
{
}

void RectDrawingTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if(m_state == State::Idle)
        {
            m_startPos          = m_pView->mapToScene(event->pos());
            m_pTempPolylineItem = new QGraphicsPathItem;
            m_pTempPolylineItem->setPen(DRAWED_LINE_PEN(m_pView->GetScaleFactory()));
            m_pView->scene()->addItem(m_pTempPolylineItem);

            m_state = State::Drawing;
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        HandleRightButtonPress(event);
    }
}

void RectDrawingTool::mouseMoveEvent(QMouseEvent* event)
{
    if (m_state == State::Drawing)
    {
        QPointF currentPos = m_pView->mapToScene(event->pos());

        QPointF p1 = m_startPos;
        QPointF p2(currentPos.x(), m_startPos.y());
        QPointF p3 = currentPos;
        QPointF p4(m_startPos.x(), currentPos.y());

        QPainterPath path;
        path.moveTo(p1);
        path.lineTo(p2);
        path.lineTo(p3);
        path.lineTo(p4);
        path.lineTo(p1);

        m_pTempPolylineItem->setPath(path);
    }

    HandleRightButtonMove(event);
}

void RectDrawingTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        HandleRightButtonRelease(event);
        return;
    }

    if (m_state == State::Drawing)
    {
        m_pView->GetCurrentShapes()->SelectShapes(false);

        QPointF endPos = m_pView->mapToScene(event->pos());

        QPointF p1 = m_startPos;
        QPointF p2(endPos.x(), m_startPos.y());
        QPointF p3 = endPos;
        QPointF p4(m_startPos.x(), endPos.y());

        QVector<QPointF> points{p1, p2, p3, p4, p1};// 闭合

        PolylineShape* pShape = new PolylineShape();
        pShape->SetPoints(points);
        pShape->Select(true);

        m_pView->GetCurrentShapes()->AddShape(pShape);
        m_pView->scene()->addItem(pShape);

        m_pView->scene()->removeItem(m_pTempPolylineItem);
        delete m_pTempPolylineItem;
        m_pTempPolylineItem = nullptr;

        m_pView->UpdateCanvas();

        m_state = State::Idle;
    }
}

int RectDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Rect);
}

void RectDrawingTool::CancelDrawing()
{
    if(m_pTempPolylineItem)
    {
        m_pView->scene()->removeItem(m_pTempPolylineItem);
        delete m_pTempPolylineItem;
        m_pTempPolylineItem = nullptr;

        m_pView->GetCurrentShapes()->SelectShapes(false);
        m_pView->UpdateCanvas();
    }

    m_state = State::Idle;
}
