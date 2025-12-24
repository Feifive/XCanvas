#include "SelectTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "Shape.h"
#include "ShapeManager.h"
#include "TranslateShapeListCommand.h"
#include "RotateShapesCommand.h"
#include <QMouseEvent>

#include "MyMath.h"

xcanvas::SelectTool::SelectTool(MyGraphicsView* view, Canvas* canvas) :
    DrawingTool(view, canvas),
    m_bMovingItem(false),
    m_rotating(false),
    m_totalRotation(0.0),
    m_lastHitPos(-2),
    m_lastHighlightedShape(nullptr)
{
}

xcanvas::SelectTool::~SelectTool()
{
}

void xcanvas::SelectTool::mousePressEvent(QMouseEvent* event)
{
    QPointF scenePos = m_canvasView->mapToScene(event->pos());
    m_mousePos       = scenePos;
    if (event->button() == Qt::LeftButton && m_canvasView->cursor().shape() == Qt::ArrowCursor)
    {
        if (m_state == State::Idle)
        {
            m_state = State::Drawing;
        }
    }
    if (event->button() == Qt::LeftButton && m_canvasView->cursor().shape() == Qt::SizeAllCursor)
    {
        m_bMovingItem  = true;
        m_dragStartPos = scenePos;
    }
    if (event->button() == Qt::LeftButton && hitTraceHandle(scenePos) == Rotate)
    {
        m_rotating       = true;
        m_totalRotation  = 0.0;
        m_dragStartPos   = scenePos;
        m_rotationCenter = m_canvas->shapeManager()->selectedBoundingRect().center();
    }
}

void xcanvas::SelectTool::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF scenePos = m_canvasView->mapToScene(event->pos());

    if (m_state == State::Drawing)
    {
        const QRectF rect(qMin(m_mousePos.x(), scenePos.x()), qMin(m_mousePos.y(), scenePos.y()), qAbs(scenePos.x() - m_mousePos.x()), qAbs(scenePos.y() - m_mousePos.y()));

        updateSelectionRect(rect);

        m_canvasView->requestFullUpdate();
    }
    else
    {
        if (m_bMovingItem)
        {
            const QPointF delta = scenePos - m_mousePos;

            ShapeList selectedShapeList = m_canvas->shapeManager()->selectedShapes();
            for (Shape* shape : selectedShapeList)
            {
                shape->translate(delta);
            }

            m_mousePos = scenePos;
            m_canvasView->requestFullUpdate();
        }
        else if (m_rotating)
        {
            const QRectF  rect   = m_canvas->shapeManager()->selectedBoundingRect();
            const QPointF center = rect.center();

            const double oldAngle  = std::atan2(m_mousePos.y() - center.y(), m_mousePos.x() - center.x());
            const double newAngle  = std::atan2(scenePos.y() - center.y(), scenePos.x() - center.x());
            const double angleDiff = qRadiansToDegrees(newAngle - oldAngle);
            m_totalRotation        += angleDiff;

            ShapeList selectedShapeList = m_canvas->shapeManager()->selectedShapes();
            for (Shape* shape : selectedShapeList)
            {
                shape->rotate(angleDiff, m_rotationCenter);
            }

            m_mousePos = scenePos;
            m_canvasView->requestFullUpdate();
        }
        else
        {
            int hitPos  = -1;
            hitPos = hitTraceHandle(scenePos);
            if (hitPos != m_lastHitPos) {
                setCanvasCursorShape(hitPos);
                m_lastHitPos = hitPos;
            }

            Shape* currentShape = nullptr;
            if (hitPos == -1) {
                currentShape = hitUnselectedShape(scenePos);
            }

            if (currentShape != m_lastHighlightedShape) {
                if (currentShape) {
                    updateHighlight(*currentShape);
                } else {
                    clearHighlight();
                }
                m_lastHighlightedShape = currentShape;
                m_canvasView->requestFullUpdate();
            }
        }
    }
}

void xcanvas::SelectTool::mouseReleaseEvent(QMouseEvent* event)
{
    QPointF scenePos = m_canvasView->mapToScene(event->pos());

    if (m_state == State::Drawing)
    {
        QRectF rect = m_selectionRectPath.boundingRect();

        if (rect.width() > 1 && rect.height() > 1)
        {
            m_canvas->shapeManager()->selectInRect(rect);
        }
        else
        {
            m_canvas->shapeManager()->deselectAll();
            Shape* pShape = hitUnselectedShape(scenePos);
            if (pShape)
            {
                pShape->setSelected(true);
            }
        }

        clearSelectionRect();
        clearHighlight();

        m_state = State::Idle;

        m_canvasView->requestFullUpdate();
    }

    if (m_bMovingItem)
    {
        m_bMovingItem = false;

        if (const QPointF totalOffset = scenePos - m_dragStartPos; !totalOffset.isNull())
        {
            ShapeList selectedShapeList = m_canvas->shapeManager()->selectedShapes();
            for (Shape* shape : selectedShapeList)
            {
                shape->translate(-totalOffset);
            }
            m_canvas->undoStack()->push(new TranslateShapesCommand(selectedShapeList, totalOffset));
        }

        m_canvasView->requestFullUpdate();
    }

    if (m_rotating)
    {
        m_rotating = false;
        if (std::abs(m_totalRotation) > 0.01)
        {
            ShapeList selectedShapeList = m_canvas->shapeManager()->selectedShapes();

            for (Shape* shape : selectedShapeList) {
                shape->rotate(-m_totalRotation, m_rotationCenter);
            }

            m_canvas->undoStack()->push(new RotateShapesCommand(selectedShapeList, m_totalRotation, m_rotationCenter));
        }
        m_canvasView->requestFullUpdate();
    }
}

void xcanvas::SelectTool::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        ShapeList shapeList = m_canvas->shapeManager()->selectedShapes();
        if (shapeList.isEmpty())
        {
            event->accept();
            return;
        }
        m_canvas->removeShapes(shapeList);

        m_canvasView->requestFullUpdate();
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
    const QRectF rect = m_canvas->shapeManager()->selectedBoundingRect();
    if (!rect.isValid()) {
        return -1;
    }

    if (const QRectF probeRect = rect.adjusted(-40, -40, 40, 40); !probeRect.contains(pos)) {
        return -1;
    }

    auto [resizeRects, rotateRect] = geometryMath::traceRects(rect, m_canvasView->zoomValue());

    if (rotateRect.contains(pos)) {
        return Rotate;
    }

    for (int i = 0; i < resizeRects.size(); ++i) {
        if (resizeRects[i].contains(pos)) {
            return i;
        }
    }

    if (rect.contains(pos)) {
        return Center;
    }

    return -1;
}

void xcanvas::SelectTool::setCanvasCursorShape(int nHitPos)
{
    switch (nHitPos)
    {
    case TopLeft:
    case BottomRight:
        m_canvasView->setCursor(Qt::SizeFDiagCursor);
        break;
    case TopMid:
    case BottomMid:
        m_canvasView->setCursor(Qt::SizeVerCursor);
        break;
    case TopRight:
    case BottomLeft:
        m_canvasView->setCursor(Qt::SizeBDiagCursor);
        break;
    case MidLeft:
    case MidRight:
        m_canvasView->setCursor(Qt::SizeHorCursor);
        break;
    case Center:
        m_canvasView->setCursor(Qt::SizeAllCursor);
        break;
    case Rotate: {
        const QIcon rotateIcon(":/Resource/Icons/Rotate.svg");
        const QPixmap pixmap = rotateIcon.pixmap(QSize(16, 16));
        m_canvasView->setCursor(QCursor(pixmap));
    }
        break;
    default:
        m_canvasView->setCursor(Qt::ArrowCursor);
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
    double        dScale  = m_canvasView->zoomValue();
    ShapeManager* pShapes = m_canvas->shapeManager();
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
        if (pShape->isPointNearPath(pos, dScale))
        {
            return pShape;
        }
    }

    return nullptr;
}
