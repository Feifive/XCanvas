#include "RectTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "MyMath.h"
#include "ShapeVector.h"
#include "AppSettings.h"
#include <QGraphicsView>
#include <QMouseEvent>

xcanvas::RectTool::RectTool(MyGraphicsView* view, Canvas* canvas) : DrawingTool(view, canvas)
{
}

xcanvas::RectTool::~RectTool()
{
}

void xcanvas::RectTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    if (m_state == State::Idle)
    {
        m_state    = State::Drawing;
        m_mousePos = m_canvasView->mapToScene(event->pos());
    }
}

void xcanvas::RectTool::mouseMoveEvent(QMouseEvent* event)
{
    if (m_state == State::Drawing)
    {
        QPointF currentPos = m_canvasView->mapToScene(event->pos());

        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            const qreal dx   = currentPos.x() - m_mousePos.x();
            const qreal dy   = currentPos.y() - m_mousePos.y();
            const qreal side = std::min(std::abs(dx), std::abs(dy));
            const qreal sx   = (dx == 0.0) ? 1.0 : sign1(dx);
            const qreal sy   = (dy == 0.0) ? 1.0 : sign1(dy);

            currentPos.setX(m_mousePos.x() + sx * side);
            currentPos.setY(m_mousePos.y() + sy * side);
        }

        QPointF p1 = m_mousePos;
        QPointF p2(currentPos.x(), m_mousePos.y());
        QPointF p3 = currentPos;
        QPointF p4(m_mousePos.x(), currentPos.y());

        m_previewPath = QPainterPath(p1);
        m_previewPath.lineTo(p2);
        m_previewPath.lineTo(p3);
        m_previewPath.lineTo(p4);
        m_previewPath.closeSubpath();

        m_canvasView->requestFullUpdate();
    }

    handleRightButtonMove(event);
}

void xcanvas::RectTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
        return;
    }

    if (m_state == State::Drawing)
    {
        QRectF           rect = m_previewPath.boundingRect();
        QVector<QPointF> points{rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft(), rect.topLeft()};

        if (qFuzzyCompare(points[0], points[2]))
        {
            cancelDrawing();
            return;
        }

        auto* shape = new ShapeVector();
        shape->setSemantic(VectorSemantic::Rectangle);
        shape->segments() = geometryMath::buildPolylineSegments(points);
        shape->setColor(AppSettings::instance().activeColor());
        m_canvas->addShape(shape);
        m_canvas->shapeManager()->selectShape(shape, true);

        cancelDrawing();
    }
}

DrawingToolType xcanvas::RectTool::toolType()
{
    return DrawingToolType::Rect;
}
