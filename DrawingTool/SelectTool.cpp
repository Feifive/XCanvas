#include "SelectTool.h"
#include "../MyGraphicsView.h"
#include "Global.h"
#include "Shape.h"
#include "ShapeManager.h"
#include "TranslateShapeListCommand.h"
#include <QGraphicsPathItem>
#include <QMouseEvent>

xcanvas::SelectTool::SelectTool(MyGraphicsView* pView) : DrawingTool(pView), m_bMovingItem(false)
{
}

xcanvas::SelectTool::~SelectTool()
{
}

void xcanvas::SelectTool::mousePressEvent(QMouseEvent* event)
{
    QPointF scenePos = m_pView->mapToScene(event->pos());
    m_mousePos       = scenePos;
    if (event->button() == Qt::LeftButton && m_pView->cursor().shape() == Qt::ArrowCursor)
    {
        if (m_state == State::Idle)
        {
            m_state = State::Drawing;
        }
    }
    if (event->button() == Qt::LeftButton && m_pView->cursor().shape() == Qt::SizeAllCursor)
    {
        m_bMovingItem  = true;
        m_dragStartPos = scenePos;
    }
}

void xcanvas::SelectTool::mouseMoveEvent(QMouseEvent* event)
{
    QPointF scenePos = m_pView->mapToScene(event->pos());
    int     nHitPos  = -1;

    if (m_state == State::Drawing)
    {
        QRectF rect(qMin(m_mousePos.x(), scenePos.x()), qMin(m_mousePos.y(), scenePos.y()), qAbs(scenePos.x() - m_mousePos.x()), qAbs(scenePos.y() - m_mousePos.y()));

        updateSelectionRect(rect);

        m_pView->updateCanvas();
    }
    else
    {
        if (m_bMovingItem)
        {
            QPointF delta = scenePos - m_mousePos;

            ShapeList selectedShapeList = m_pView->GetCurrentShapes()->selectedShapes();
            for (Shape* shape : selectedShapeList)
            {
                shape->translate(delta);
            }

            m_mousePos = scenePos;
            m_pView->updateCanvas();
        }
        else
        {
            nHitPos = hitTraceHandle(scenePos);
            setCanvasCursorShape(nHitPos);
            if (nHitPos == -1)
            {
                Shape* pShape = hitUnselectedShape(scenePos);
                if (pShape)
                {
                    updateHighlight(*pShape);
                }
                else
                {
                    clearHighlight();
                }
            }
            else
            {
                clearHighlight();
            }

            m_pView->updateCanvas();
        }
    }
}

void xcanvas::SelectTool::mouseReleaseEvent(QMouseEvent* event)
{
    QPointF scenePos = m_pView->mapToScene(event->pos());

    if (m_state == State::Drawing)
    {
        QRectF rect = m_selectionRectPath.boundingRect();

        if (rect.width() > 1 && rect.height() > 1)
        {
            m_pView->GetCurrentShapes()->selectInRect(rect);
        }
        else
        {
            m_pView->GetCurrentShapes()->deselectAll();
            Shape* pShape = hitUnselectedShape(scenePos);
            if (pShape)
            {
                pShape->setSelected(true);
            }
        }

        clearSelectionRect();
        clearHighlight();

        m_state = State::Idle;

        m_pView->updateCanvas();
    }

    if (m_bMovingItem)
    {
        m_bMovingItem = false;

        if (const QPointF totalOffset = scenePos - m_dragStartPos; !totalOffset.isNull())
        {
            ShapeList selectedShapeList = m_pView->GetCurrentShapes()->selectedShapes();
            for (Shape* shape : selectedShapeList)
            {
                shape->translate(-totalOffset);
            }
            m_pView->getUndoStack()->push(new TranslateShapesCommand(selectedShapeList, totalOffset));
        }

        m_pView->updateCanvas();
    }
}

void xcanvas::SelectTool::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        if (m_pView)
        {
            ShapeList shapeList = m_pView->GetCurrentShapes()->selectedShapes();
            if (shapeList.isEmpty())
            {
                event->accept();
                return;
            }
            m_pView->removeShapes(shapeList);
        }

        m_pView->updateCanvas();
        event->accept();
    }
}

void xcanvas::SelectTool::drawPreview(QPainter* painter)
{
    if (!m_highlightPath.isEmpty())
    {
        painter->save();

        painter->setPen(highlightPen());
        painter->drawPath(m_highlightPath);

        painter->restore();
    }
    else if (!m_selectionRectPath.isEmpty())
    {
        painter->save();

        painter->setPen(dotLinePen());
        painter->drawPath(m_selectionRectPath);

        painter->restore();
    }
}

DrawingToolType xcanvas::SelectTool::toolType()
{
    return DrawingToolType::Select;
}

int xcanvas::SelectTool::hitTraceHandle(const QPointF& pos) const
{
    QRectF rectf = m_pView->GetCurrentShapes()->selectedBoundingRect();

    if (rectf.isValid())
    {
        QRectF traces[9];
        m_pView->traceRects(rectf, traces);

        for (int i = 0; i < ERECT_POS_COUNT; ++i)
        {
            if (traces[i].contains(pos))
            {
                return i;
            }
            else if (rectf.contains(pos))
            {
                return ERECT_CENTER;
            }
        }
    }
    return -1;
}

void xcanvas::SelectTool::setCanvasCursorShape(int nHitPos)
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

void xcanvas::SelectTool::updateHighlight(const Shape& shape)
{
    m_highlightPath = QPainterPath();
    m_highlightPath.addPath(shape.path());
}

void xcanvas::SelectTool::clearHighlight()
{
    m_highlightPath = QPainterPath();
}

void xcanvas::SelectTool::updateSelectionRect(const QRectF& rect)
{
    m_selectionRectPath = QPainterPath();
    m_selectionRectPath.addRect(rect);
}

void xcanvas::SelectTool::clearSelectionRect()
{
    m_selectionRectPath = QPainterPath();
}

xcanvas::Shape* xcanvas::SelectTool::hitUnselectedShape(QPointF pos)
{
    double        dScale  = m_pView->zoomValue();
    ShapeManager* pShapes = m_pView->GetCurrentShapes();
    for (int i = 0; i < pShapes->count(); ++i)
    {
        Shape* pShape = (*pShapes)[i];
        if (!pShape)
        {
            continue;
        }
        if (pShape->isSelected())
        {
            continue;
        }
        if (pShape->type() == ShapeType::Image)
        {
            if (pShape->boundingRect().contains(pos))
            {
                return pShape;
            }
        }
        else if (pShape->isPointNearPath(pos, dScale))
        {
            return pShape;
        }
    }

    return nullptr;
}
