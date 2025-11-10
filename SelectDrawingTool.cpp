#include "SelectDrawingTool.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include "Shapes.h"
#include <QMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsPathItem>

SelectDrawingTool::SelectDrawingTool(MyGraphicsView* pView) :
    BaseDrawingTool(pView),
    m_pDrawingItem(nullptr),
    m_pHighlightItem(nullptr),
    m_bDrawing(false),
    m_startPos(-1, -1)
{}

SelectDrawingTool::~SelectDrawingTool()
{

}

void SelectDrawingTool::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = m_pView->mapToScene(event->pos());
    if(m_pView && event->button() == Qt::LeftButton)
    {
        m_bDrawing     = true;
        m_startPos     = scenePos;
        m_pDrawingItem = new QGraphicsRectItem();
        m_pDrawingItem->setPen(DRAWING_LINE_PEN);
        m_pView->scene()->addItem(m_pDrawingItem);

        m_pView->GetCurrentShapes()->SelectShapes(false);
        BaseShape* pShape = HitUnselectedShape(scenePos);
        if(pShape)
        {
            pShape->Select(true);
        }
    }
}

void SelectDrawingTool::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = m_pView->mapToScene(event->pos());
    int nHitPos = -1;

    if(m_bDrawing && m_pDrawingItem)
    {
        QRectF rect(
            qMin(m_startPos.x(), scenePos.x()),
            qMin(m_startPos.y(), scenePos.y()),
            qAbs(scenePos.x() - m_startPos.x()),
            qAbs(scenePos.y() - m_startPos.y())
            );

        m_pDrawingItem->setRect(rect);
    }
    else
    {
        nHitPos = HitSelectedShapeTrace(scenePos);
        SetCanvasCursorShape(nHitPos);
        if(nHitPos == -1)
        {
            BaseShape* pShape = HitUnselectedShape(scenePos);
            if(pShape)
            {
                AddHighlightItemToCanvas(pShape->boundingRect());
            }
            else
            {
                RemoveHighlightItemFromCanvas();
            }
        }
        else
        {
            RemoveHighlightItemFromCanvas();
        }
    }
}

void SelectDrawingTool::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_bDrawing && m_pDrawingItem)
    {
        QRectF rect = m_pDrawingItem->rect();
        if(rect.isValid())
        {
            m_pView->GetCurrentShapes()->SelectShapes(rect);
        }
        delete m_pDrawingItem;
        m_pDrawingItem = nullptr;
        m_bDrawing     = false;

        m_pView->UpdateCanvas();
    }
}

int SelectDrawingTool::ToolType()
{
    return static_cast<int>(DrawingToolType::Select);
}

int SelectDrawingTool::HitSelectedShapeTrace(QPointF pos)
{
    QRectF rectf = m_pView->GetCurrentShapes()->GetSelectedShapeRect();
    if(rectf.isValid())
    {
        QRectF traces[9];
        GetTraceRects(rectf, traces);

        for (int i = 0; i < ERECT_POS_COUNT; ++i)
        {
            if(traces[i].contains(pos))
            {
                return i;
            }
        }
    }
    return -1;
}

void SelectDrawingTool::SetCanvasCursorShape(int nHitPos)
{
    switch (nHitPos) {
    case ERECT_TOP_LEFT:
    case ERECT_BOTTOM_RIGHT:
        m_pView->setCursor(Qt::SizeFDiagCursor);
        break;
    case ERECT_TOP_MID:
    case ERECT_BOTTOM_MID:
        m_pView->setCursor(Qt::SizeVerCursor);
        break;
    case ERECT_TOP_RIGHT:
    case ERECT_BOTTOM_LEFT:
        m_pView->setCursor(Qt::SizeBDiagCursor);
        break;
    case ERECT_MID_LEFT:
    case ERECT_MID_RIGHT:
        m_pView->setCursor(Qt::SizeHorCursor);
        break;
    case ERECT_CENTER:
        m_pView->setCursor(Qt::SizeAllCursor);
        break;
    default:
        m_pView->setCursor(Qt::ArrowCursor);
        break;
    }
}

BaseShape *SelectDrawingTool::HitUnselectedShape(QPointF pos)
{
    double  dScale  = m_pView->GetScaleFactory();
    Shapes* pShapes = m_pView->GetCurrentShapes();
    for (int i = 0; i < pShapes->Count(); ++i)
    {
        BaseShape* pShape = pShapes->GetShape(i);
        if(!pShape)
        {
            continue;
        }
        if(pShape->IsSelected())
        {
            continue;
        }
        if(pShape->IsPointNearPath(pos, dScale))
        {
            return pShape;
        }
    }

    return nullptr;
}

void SelectDrawingTool::AddHighlightItemToCanvas(const QRectF& rect)
{
    if(!m_pHighlightItem)
    {
        m_pHighlightItem = new QGraphicsRectItem;
        QPen pen = HIGHLIGHT_LINE_PEN(m_pView->GetScaleFactory());
        m_pHighlightItem->setPen(pen);
        m_pHighlightItem->setRect(rect);
        m_pHighlightItem->setZValue(Z_VALUE_HIGHLIGHT);
        m_pView->scene()->addItem(m_pHighlightItem);
    }
    else
    {
        m_pHighlightItem->setRect(rect);
    }
}

void SelectDrawingTool::RemoveHighlightItemFromCanvas()
{
    if(m_pHighlightItem)
    {
        m_pView->scene()->removeItem(m_pHighlightItem);
        delete m_pHighlightItem;
        m_pHighlightItem = nullptr;
    }
}
