#include "SelectTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "Shape.h"
#include "ShapeManager.h"
#include "TransformCommand.h"
#include <QMouseEvent>

#include "MyMath.h"

xcanvas::SelectTool::SelectTool(MyGraphicsView* view, Canvas* canvas) :
    DrawingTool(view, canvas),
    m_bMovingItem(false),
    m_rotating(false),
    m_resizing(false),
    m_resizeHandle(-1),
    m_startClickDistX(0.0),
    m_startClickDistY(0.0),
    m_lastHitPos(-2),
    m_lastHighlightedShape(nullptr)
{
}

xcanvas::SelectTool::~SelectTool()
{
}

void xcanvas::SelectTool::mousePressEvent(QMouseEvent* event)
{
    const QPointF scenePos = m_canvasView->mapToScene(event->pos());
    m_mousePos             = scenePos;
    m_dragStartPos         = scenePos;
    const int hitPos       = hitTraceHandle(scenePos);
    if (event->button() == Qt::LeftButton)
    {
        m_dragStartPos = scenePos;
        if (m_canvasView->cursor().shape() == Qt::ArrowCursor) {
            if (m_state == State::Idle)
            {
                m_state = State::Drawing;
            }
        }
        bool isTransforming = false;
        if (m_canvasView->cursor().shape() == Qt::SizeAllCursor) {
            m_bMovingItem  = true;
            isTransforming = true;
        }
        else if (hitPos == Rotate) {
            m_rotating       = true;
            m_rotationCenter = m_canvas->shapeManager()->selectedBoundingRect().center();
            isTransforming = true;
        }
        else if (hitPos != -1) {
            m_resizing            = true;
            m_resizeHandle        = hitPos;
            m_initialSelectedRect = m_canvas->shapeManager()->selectedBoundingRect();
            m_anchorPoint         = getAnchorPoint(hitPos, m_initialSelectedRect);
            m_startClickDistX     = m_dragStartPos.x() - m_anchorPoint.x();
            m_startClickDistY     = m_dragStartPos.y() - m_anchorPoint.y();
            isTransforming = true;
        }
        if (isTransforming) {
            m_initialTransforms.clear();
            for (Shape* shape : m_canvas->shapeManager()->selectedShapeList()) {
                m_initialTransforms[shape] = shape->transform();
            }
        }
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
        QTransform deltaTransform;
        bool isTransforming = false;

        if (m_bMovingItem)
        {
            const QPointF totalDelta = scenePos - m_dragStartPos;
            deltaTransform.translate(totalDelta.x(), totalDelta.y());
            isTransforming = true;
        }
        else if (m_rotating)
        {
            const double startAngle   = std::atan2(m_dragStartPos.y() - m_rotationCenter.y(), m_dragStartPos.x() - m_rotationCenter.x());
            const double currentAngle = std::atan2(scenePos.y() - m_rotationCenter.y(), scenePos.x() - m_rotationCenter.x());
            const double angleDiff = qRadiansToDegrees(currentAngle - startAngle);
            deltaTransform.translate(m_rotationCenter.x(), m_rotationCenter.y());
            deltaTransform.rotate(angleDiff);
            deltaTransform.translate(-m_rotationCenter.x(), -m_rotationCenter.y());
            isTransforming = true;
        }
        else if (m_resizing) {
            if (const QRectF initialRect = m_initialSelectedRect; !initialRect.isValid() || initialRect.width() <= 0 || initialRect.height() <= 0) {
                return;
            }

            const double currentDistX = scenePos.x() - m_anchorPoint.x();
            const double currentDistY = scenePos.y() - m_anchorPoint.y();

            double sx = 1.0;
            double sy = 1.0;

            if (m_resizeHandle != TopMid && m_resizeHandle != BottomMid) {
                sx = currentDistX / m_startClickDistX;
            }
            if (m_resizeHandle != MidLeft && m_resizeHandle != MidRight) {
                sy = currentDistY / m_startClickDistY;
            }

            if (event->modifiers() != Qt::ShiftModifier && (m_resizeHandle == TopLeft || m_resizeHandle == TopRight || m_resizeHandle == BottomLeft || m_resizeHandle == BottomRight)) {
                if (sx == 1.0) {
                    sx = sy;
                }
                else if (sy == 1.0) {
                    sy = sx;
                }
                else {
                    const double scale = (std::abs(sx) > std::abs(sy)) ? sx : sy;
                    sx = (sx > 0 ? std::abs(scale) : -std::abs(scale));
                    sy = (sy > 0 ? std::abs(scale) : -std::abs(scale));
                }
            }

            deltaTransform.translate(m_anchorPoint.x(), m_anchorPoint.y());
            deltaTransform.scale(sx, sy);
            deltaTransform.translate(-m_anchorPoint.x(), -m_anchorPoint.y());
            isTransforming = true;
        }

        if (isTransforming) {
            for (auto const& [shape, initMatrix] : m_initialTransforms) {
                shape->setTransform(initMatrix * deltaTransform);
            }
            m_canvas->shapeManager()->invalidateSelectedRect();
            m_canvasView->requestFullUpdate();
            m_canvasView->updateSelectionHud();
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
    const QPointF scenePos = m_canvasView->mapToScene(event->pos());

    if (m_state == State::Drawing)
    {
        if (const QRectF rect = m_selectionRectPath.boundingRect(); rect.width() > 1 && rect.height() > 1)
        {
            m_canvas->shapeManager()->selectInRect(rect);
        }
        else
        {
            if (Shape* shape = hitUnselectedShape(scenePos)) {
                m_canvas->shapeManager()->selectShape(shape, true);
            }
            else {
                m_canvas->shapeManager()->deselectAll();
            }
        }

        clearSelectionRect();
        clearHighlight();

        m_state = State::Idle;

        m_canvasView->requestFullUpdate();
    }
    else if ((m_bMovingItem || m_rotating || m_resizing) && !m_initialTransforms.empty()) {
        QString text;
        if (m_bMovingItem) text = "Translate Shapes";
        else if (m_rotating) text = "Rotate Shapes";
        else if (m_resizing) text = "Resize Shapes";

        m_canvas->undoStack()->push(new TransformCommand(m_canvas->shapeManager(), std::move(m_initialTransforms), text));
        m_bMovingItem = m_rotating = m_resizing = false;
        m_initialTransforms.clear();
        m_canvasView->requestFullUpdate();
    }
}

void xcanvas::SelectTool::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        const ShapeList shapeList = m_canvas->shapeManager()->selectedShapeList();
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

QPointF xcanvas::SelectTool::getAnchorPoint(const int handle, QRectF rect) const {
    QPointF anchorPoint;
    switch (handle) {
        case TopLeft:     anchorPoint = rect.bottomRight(); break;
        case TopMid:      anchorPoint = QPointF(rect.center().x(), rect.bottom()); break;
        case TopRight:    anchorPoint = rect.bottomLeft();  break;
        case MidRight:    anchorPoint = QPointF(rect.left(), rect.center().y());   break;
        case BottomRight: anchorPoint = rect.topLeft();     break;
        case BottomMid:   anchorPoint = QPointF(rect.center().x(), rect.top());    break;
        case BottomLeft:  anchorPoint = rect.topRight();    break;
        case MidLeft:     anchorPoint = QPointF(rect.right(), rect.center().y());  break;
        default: break;
    }
    return anchorPoint;
}

xcanvas::Shape* xcanvas::SelectTool::hitUnselectedShape(QPointF pos)
{
    const double        dScale  = m_canvasView->zoomValue();
    const ShapeManager* pShapes = m_canvas->shapeManager();
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
        if (const double tolerance = 6 / dScale; pShape->hitTest(pos, tolerance))
        {
            return pShape;
        }
    }

    return nullptr;
}
