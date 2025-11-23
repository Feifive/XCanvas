#include "PolylineDrawingTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "PolylineShape.h"
#include <QMouseEvent>

PolylineDrawingTool::PolylineDrawingTool(MyGraphicsView* pView) :
    BaseDrawingTool(pView),
    m_pDrawingItem(nullptr),
    m_startPos(-1, -1)
{}

PolylineDrawingTool::~PolylineDrawingTool()
{
}

void PolylineDrawingTool::mousePressEvent(QMouseEvent *event)
{
    m_startPos = m_pView->mapToScene(event->pos());

    if(event->button() == Qt::LeftButton)
    {
        if(m_state == State::Idle)
        {
            m_points.clear();

            m_pDrawingItem = new PolylineShape();
            m_pView->scene()->addItem(m_pDrawingItem);
            m_points.append(m_startPos);

            m_state = State::Drawing;
        }
        else if (m_state == State::Drawing)
        {
            m_points.append(m_startPos);
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        HandleRightButtonPress(event);
    }
}

void PolylineDrawingTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_state == State::Drawing)
    {
        QPointF currentPos = m_pView->mapToScene(event->pos());
        if(m_points.size() < 2)
        {
            m_points.append(currentPos);
        }
        else
        {
            m_points.last() = currentPos;
        }
        m_pDrawingItem->SetPoints(m_points);
    }

    HandleRightButtonMove(event);
}

void PolylineDrawingTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        HandleRightButtonRelease(event);
        return;
    }
}

int PolylineDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Polyline);
}

void PolylineDrawingTool::CancelDrawing()
{
    if(m_state == State::Interrupted)
    {
        m_points.removeLast();
        if(m_points.count() < 2)
        {
            m_pView->scene()->removeItem(m_pDrawingItem);
            delete m_pDrawingItem;
            m_pDrawingItem = nullptr;
        }
        else
        {
            m_pView->GetCurrentShapes()->SelectShapes(false);

            m_pDrawingItem->SetPoints(m_points);
            m_pDrawingItem->Select(true);
            m_pView->GetCurrentShapes()->AddShape(m_pDrawingItem);

            m_pView->UpdateCanvas();
        }

        m_state = State::Idle;
    }
}
