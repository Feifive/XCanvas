#include "EllipseTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "MyMath.h"
#include <QGraphicsEllipseItem>
#include <QMouseEvent>

#include "AppSettings.h"

xcanvas::EllipseTool::EllipseTool(MyGraphicsView* view, Canvas* canvas) : DrawingTool(view, canvas)
{
}

xcanvas::EllipseTool::~EllipseTool()
{
}

void xcanvas::EllipseTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    if (m_state == State::Idle)
    {
        m_mousePos    = m_canvasView->mapToScene(event->pos());
        m_previewPath = QPainterPath();
        m_state       = State::Drawing;
    }
}

void xcanvas::EllipseTool::mouseMoveEvent(QMouseEvent* event)
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

        QRectF rect(qMin(m_mousePos.x(), currentPos.x()), qMin(m_mousePos.y(), currentPos.y()), qAbs(currentPos.x() - m_mousePos.x()), qAbs(currentPos.y() - m_mousePos.y()));

        m_previewPath = QPainterPath();
        m_previewPath.addEllipse(rect);

        m_canvasView->requestFullUpdate();
    }

    handleRightButtonMove(event);
}

void xcanvas::EllipseTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
        return;
    }

    if (m_state == State::Drawing)
    {
        const QRectF rect  = m_previewPath.boundingRect();
        auto*        shape = new ShapeVector();
        if (qFuzzyCompare(rect.width(), rect.height()))
        {
            shape->setSemantic(VectorSemantic::Circle);
        }
        else
        {
            shape->setSemantic(VectorSemantic::Ellipse);
        }

        shape->segments() = geometryMath::buildEllipseSegments(rect);
        shape->setColor(AppSettings::instance().activeColor());
        m_canvas->addShape(shape);
        m_canvas->shapeManager()->selectShape(shape, true);

        cancelDrawing();
    }
}

DrawingToolType xcanvas::EllipseTool::toolType()
{
    return DrawingToolType::Ellipse;
}
