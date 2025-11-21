#include "SelectDrawingTool.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include "Shapes.h"
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QMouseEvent>

SelectDrawingTool::SelectDrawingTool(MyGraphicsView* pView) : BaseDrawingTool(pView), m_pDrawingItem(nullptr), m_pHighlightItem(nullptr), m_bDrawing(false), m_bMovingItem(false), m_startPos(-1, -1)
{
}

SelectDrawingTool::~SelectDrawingTool()
{
}

void SelectDrawingTool::mousePressEvent(QMouseEvent* event)
{
    if (!m_pView)
    {
        return;
    }

    QPointF scenePos = m_pView->mapToScene(event->pos());
    m_startPos       = scenePos;
    if (event->button() == Qt::LeftButton && m_pView->cursor().shape() == Qt::ArrowCursor)
    {
        m_bDrawing     = true;
        m_pDrawingItem = new QGraphicsRectItem();
        m_pDrawingItem->setPen(DRAWING_LINE_PEN);
        m_pView->scene()->addItem(m_pDrawingItem);
    }
    if (event->button() == Qt::LeftButton && m_pView->cursor().shape() == Qt::SizeAllCursor)
    {
        m_bMovingItem = true;
        ClearTrace();
    }
}

void SelectDrawingTool::mouseMoveEvent(QMouseEvent* event)
{
    QPointF scenePos = m_pView->mapToScene(event->pos());
    int     nHitPos  = -1;

    if (m_bDrawing && m_pDrawingItem)
    {
        QRectF rect(qMin(m_startPos.x(), scenePos.x()), qMin(m_startPos.y(), scenePos.y()), qAbs(scenePos.x() - m_startPos.x()), qAbs(scenePos.y() - m_startPos.y()));

        m_pDrawingItem->setRect(rect);
    }
    else
    {
        if (m_bMovingItem)
        {
            Shapes* pShapes = m_pView->GetCurrentShapes()->GetSelectedShapes();
            pShapes->Offset(scenePos - m_startPos);
            delete pShapes;
            m_startPos = scenePos;
        }
        else
        {
            nHitPos = HitSelectedShapeTrace(scenePos);
            SetCanvasCursorShape(nHitPos);
            if (nHitPos == -1)
            {
                Shape* pShape = HitUnselectedShape(scenePos);
                if (pShape)
                {
                    AddHighlightItemToCanvas(pShape);
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
}

void SelectDrawingTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_bDrawing && m_pDrawingItem)
    {
        QRectF rect = m_pDrawingItem->rect();
        if (rect.isValid())
        {
            m_pView->GetCurrentShapes()->SelectShapes(rect);
        }
        else
        {
            m_pView->GetCurrentShapes()->SelectShapes(false);
            Shape* pShape = HitUnselectedShape(m_pView->mapToScene(event->pos()));
            if (pShape)
            {
                pShape->Select(true);
            }
        }

        delete m_pDrawingItem;
        m_pDrawingItem = nullptr;
        m_bDrawing     = false;

        RemoveHighlightItemFromCanvas();
        m_pView->UpdateCanvas();
    }

    if (m_bMovingItem)
    {
        m_bMovingItem = false;
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
    if (rectf.isValid())
    {
        QRectF traces[9];
        GetTraceRects(rectf, traces);

        for (int i = 0; i < ERECT_POS_COUNT; ++i)
        {
            if (traces[i].contains(pos))
            {
                return i;
            }
        }
    }
    return -1;
}

void SelectDrawingTool::SetCanvasCursorShape(int nHitPos)
{
    switch (nHitPos)
    {
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

Shape* SelectDrawingTool::HitUnselectedShape(QPointF pos)
{
    double  dScale  = m_pView->GetScaleFactory();
    Shapes* pShapes = m_pView->GetCurrentShapes();
    for (int i = 0; i < pShapes->Count(); ++i)
    {
        Shape* pShape = pShapes->GetShape(i);
        if (!pShape)
        {
            continue;
        }
        if (pShape->IsSelected())
        {
            continue;
        }
        if (pShape->IsPointNearPath(pos, dScale))
        {
            return pShape;
        }
    }

    return nullptr;
}

void SelectDrawingTool::AddHighlightItemToCanvas(const Shape* pShape)
{
    if (!m_pHighlightItem)
    {
        m_pHighlightItem = new QGraphicsPathItem;
        QPen pen         = HIGHLIGHT_LINE_PEN(m_pView->GetScaleFactory());
        m_pHighlightItem->setPen(pen);
        m_pHighlightItem->setPath(pShape->shape());
        m_pHighlightItem->setZValue(Z_VALUE_HIGHLIGHT);
        m_pView->scene()->addItem(m_pHighlightItem);
    }
    else
    {
        m_pHighlightItem->setPath(pShape->shape());
    }
}

void SelectDrawingTool::RemoveHighlightItemFromCanvas()
{
    if (m_pHighlightItem)
    {
        m_pView->scene()->removeItem(m_pHighlightItem);
        delete m_pHighlightItem;
        m_pHighlightItem = nullptr;
    }
}
