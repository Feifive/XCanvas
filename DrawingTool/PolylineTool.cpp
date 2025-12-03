#include "PolylineTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "Polyline.h"
#include "Shapes.h"
#include <QMouseEvent>

xcanvas::PolylineTool::PolylineTool(MyGraphicsView* pView) :
    DrawingTool(pView)
{}

xcanvas::PolylineTool::~PolylineTool()
{
}

void xcanvas::PolylineTool::mousePressEvent(QMouseEvent *event)
{
    m_startPos = m_pView->mapToScene(event->pos());

    if(event->button() == Qt::LeftButton)
    {
        if(m_state == State::Idle)
        {
            m_points.clear();
            m_points.append(m_startPos);
            m_points.append(m_startPos);

            m_previewPath = QPainterPath();
            m_previewPath.moveTo(m_startPos);
            m_previewPath.lineTo(m_startPos);

            m_state = State::Drawing;
        }
        else if (m_state == State::Drawing)
        {
            m_points.last() = m_startPos;
            m_points.append(m_startPos);
        }

        m_pView->updateCanvas();
    }
    else if(event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
    }
}

void xcanvas::PolylineTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_state == State::Drawing)
    {
        QPointF currentPos = m_pView->mapToScene(event->pos());
        m_points.last() = currentPos;

        m_previewPath = QPainterPath();
        m_previewPath.moveTo(m_points[0]);
        for (int i = 1; i < m_points.size(); ++i)
        {
            m_previewPath.lineTo(m_points[i]);
        }

        m_pView->updateCanvas();
    }

    handleRightButtonMove(event);
}

void xcanvas::PolylineTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
        return;
    }
}

DrawingToolType xcanvas::PolylineTool::toolType()
{
    return DrawingToolType::Polyline;
}

void xcanvas::PolylineTool::cancelDrawing()
{
    if (m_state != State::Interrupted)
    {
        return;
    }

    m_points.removeLast();

    if (m_points.size() < 2)
    {
        m_previewPath = QPainterPath();
    }
    else
    {
        Polyline* pShape = new Polyline();
        pShape->SetPoints(m_points);
        pShape->setSelected(true);

        m_pView->GetCurrentShapes()->deselectAll();
        m_pView->GetCurrentShapes()->addShape(pShape);
    }

    m_previewPath = QPainterPath();
    m_points.clear();
    m_state = State::Idle;

    m_pView->updateCanvas();
}
