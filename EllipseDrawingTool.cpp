#include "EllipseDrawingTool.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include <QGraphicsEllipseItem>
#include <QMouseEvent>
#include <QDebug>

EllipseDrawingTool::EllipseDrawingTool(MyGraphicsView* pView) :
    BaseDrawingTool(pView),
    m_pDrawingItem(nullptr),
    m_bDrawing(false),
    m_startPos(-1, -1)
{}

EllipseDrawingTool::~EllipseDrawingTool()
{
    qDebug()<<__FUNCTION__;
}

void EllipseDrawingTool::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_bDrawing = true;
        m_startPos = m_pView->mapToScene(event->pos());

        m_pDrawingItem = new QGraphicsEllipseItem();
        m_pDrawingItem->setPen(DRAWING_LINE_PEN);

        m_pView->scene()->addItem(m_pDrawingItem);
    }
}

void EllipseDrawingTool::mouseMoveEvent(QMouseEvent *event)
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

        m_pDrawingItem->setRect(rect);
    }
}

void EllipseDrawingTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_bDrawing && m_pDrawingItem)
    {
        // m_pDrawingItem->setPen(DRAWED_LINE_PEN);
        m_pDrawingItem->setFlags(/*QGraphicsItem::ItemIsMovable | */QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
        m_pDrawingItem = nullptr;
        m_bDrawing = false;
    }
}

int EllipseDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Ellipse);
}
