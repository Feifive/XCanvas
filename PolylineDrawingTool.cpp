#include "PolylineDrawingTool.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include "PolylineShape.h"
#include <QMouseEvent>

LineDrawingTool::LineDrawingTool(MyGraphicsView* pView) :
    BaseDrawingTool(pView),
    m_pDrawingItem(nullptr),
    m_startPos(-1, -1),
    m_bDrawing(false)
{}

LineDrawingTool::~LineDrawingTool()
{
}

void LineDrawingTool::mousePressEvent(QMouseEvent *event)
{
    m_startPos = m_pView->mapToScene(event->pos());

    if(event->button() == Qt::LeftButton && !m_bDrawing)
    {
        m_points.clear();
        m_bDrawing = true;

        m_pDrawingItem = new PolylineShape();
        m_pView->GetCurrentShapes()->AddShape(m_pDrawingItem);
        m_pView->scene()->addItem(m_pDrawingItem);
        m_points.append(m_startPos);
    }
    else if(event->button() == Qt::LeftButton && m_bDrawing)
    {
        m_points.append(m_startPos);
    }


    if(event->button() == Qt::RightButton && m_bDrawing)
    {
        m_bDrawing = false;
    }
}

void LineDrawingTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bDrawing && m_pDrawingItem)
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
}

void LineDrawingTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_bDrawing)
    {
        if(m_pDrawingItem)
        {
            m_points.removeLast();
            m_pDrawingItem->SetPoints(m_points);
            m_pDrawingItem = nullptr;
        }
    }
}

int LineDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Line);
}
