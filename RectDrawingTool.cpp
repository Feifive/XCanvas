#include "RectDrawingTool.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include "RectShape.h"
#include <QMouseEvent>
#include <QDebug>

RectDrawingTool::RectDrawingTool(MyGraphicsView *pView) :
    BaseDrawingTool(pView),
    m_pDrawingItem(nullptr),
    m_bDrawing(false),
    m_startPos(-1, -1)
{

}

RectDrawingTool::~RectDrawingTool()
{
    qDebug()<<__FUNCTION__;
}

void RectDrawingTool::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_bDrawing = true;
        m_startPos = m_pView->mapToScene(event->pos());

        m_pDrawingItem = new RectShape();

        m_pView->GetCurrentShapes()->AddShape(m_pDrawingItem);
        m_pView->scene()->addItem(m_pDrawingItem);
    }
}

void RectDrawingTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bDrawing && m_pDrawingItem)
    {
        QPointF currentPos = m_pView->mapToScene(event->pos());
        QRectF rect(
            qMin(m_startPos.x(), currentPos.x()),
            qMin(m_startPos.y(), currentPos.y()),
            qAbs(currentPos.x() - m_startPos.x()),
            qAbs(currentPos.y() - m_startPos.y())
            );

        m_pDrawingItem->SetRect(rect);
    }
}

void RectDrawingTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_bDrawing && m_pDrawingItem)
    {
        m_pView->GetCurrentShapes()->SelectShapes(false);
        m_pDrawingItem->Select(true);
        m_pDrawingItem = nullptr;
        m_bDrawing = false;

        m_pView->UpdateCanvas();
    }
}

int RectDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Rect);
}
