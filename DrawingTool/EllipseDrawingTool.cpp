#include "EllipseDrawingTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "EllipseShape.h"
#include <QGraphicsEllipseItem>
#include <QMouseEvent>
#include <QDebug>

EllipseDrawingTool::EllipseDrawingTool(MyGraphicsView* pView) :
    BaseDrawingTool(pView),
    m_pTempEllipseItem(nullptr),
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
        m_bDrawing         = false;
        m_startPos         = m_pView->mapToScene(event->pos());
        m_pTempEllipseItem = new QGraphicsEllipseItem();
        m_pTempEllipseItem->setPen(DRAWED_LINE_PEN(m_pView->GetScaleFactory()));

        m_pView->scene()->addItem(m_pTempEllipseItem);
    }

    BaseDrawingTool::mousePressEvent(event);
}

void EllipseDrawingTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_pTempEllipseItem)
    {
        m_bDrawing         = true;
        QPointF currentPos = m_pView->mapToScene(event->pos());
        QRectF rect(
            qMin(m_startPos.x(), currentPos.x()),
            qMin(m_startPos.y(), currentPos.y()),
            qAbs(currentPos.x() - m_startPos.x()),
            qAbs(currentPos.y() - m_startPos.y())
        );

        m_pTempEllipseItem->setRect(rect);
    }

    BaseDrawingTool::mouseMoveEvent(event);
}

void EllipseDrawingTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_pTempEllipseItem)
    {
        BaseDrawingTool::mouseReleaseEvent(event);

        return;
    }

    if (!m_bDrawing)
    {
        m_pView->scene()->removeItem(m_pTempEllipseItem);
        delete m_pTempEllipseItem;
        m_pTempEllipseItem = nullptr;

        BaseDrawingTool::mouseReleaseEvent(event);

        return;
    }

    QRectF rect = m_pTempEllipseItem->rect();

    m_pView->scene()->removeItem(m_pTempEllipseItem);
    delete m_pTempEllipseItem;
    m_pTempEllipseItem = nullptr;

    EllipseShape* pShape = new EllipseShape();
    pShape->SetEllipse(rect);

    m_pView->GetCurrentShapes()->SelectShapes(false);
    pShape->Select(true);

    m_pView->GetCurrentShapes()->AddShape(pShape);
    m_pView->scene()->addItem(pShape);

    m_pView->UpdateCanvas();
    m_bDrawing = false;

    BaseDrawingTool::mouseReleaseEvent(event);
}

int EllipseDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Ellipse);
}

void EllipseDrawingTool::CancelDrawing()
{

}
