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
        m_startPos = m_pView->mapToScene(event->pos());

        m_pDrawingItem = new QGraphicsRectItem;
        QPen pen       = GetDrawedLinePen(m_pView->GetScaleFactory());
        m_pDrawingItem->setPen(pen);
        m_pView->scene()->addItem(m_pDrawingItem);
    }
}

void RectDrawingTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_pDrawingItem)
    {
        m_bDrawing = true;

        QPointF currentPos = m_pView->mapToScene(event->pos());
        QRectF rect(
            qMin(m_startPos.x(), currentPos.x()),
            qMin(m_startPos.y(), currentPos.y()),
            qAbs(currentPos.x() - m_startPos.x()),
            qAbs(currentPos.y() - m_startPos.y())
            );

        m_pDrawingItem->setRect(rect);
    }
}

void RectDrawingTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_bDrawing && m_pDrawingItem)
    {
        m_pView->GetCurrentShapes()->SelectShapes(false);
        RectShape* pShape = new RectShape;
        pShape->SetRect(m_pDrawingItem->boundingRect());
        pShape->Select(true);
        m_pView->GetCurrentShapes()->AddShape(pShape);
        m_pView->scene()->addItem(pShape);

        m_pView->scene()->removeItem(m_pDrawingItem);
        delete m_pDrawingItem;
        m_pDrawingItem = nullptr;
        m_bDrawing     = false;

        m_pView->UpdateCanvas();
    }
    else if(m_pDrawingItem && !m_bDrawing)
    {
        m_pView->scene()->removeItem(m_pDrawingItem);
        delete m_pDrawingItem;
        m_pDrawingItem = nullptr;

        m_pView->GetCurrentShapes()->SelectShapes(false);
        m_pView->UpdateCanvas();
    }
}

int RectDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Rect);
}
