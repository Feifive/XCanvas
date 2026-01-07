#include "PolygonTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "MyMath.h"
#include "ShapeVector.h"

#include <QPointF>
#include <QRectF>

#include "AppSettings.h"

namespace xcanvas
{

PolygonTool::PolygonTool(MyGraphicsView* view, Canvas* canvas) : DrawingTool(view, canvas)
{
}

PolygonTool::~PolygonTool()
{
}

void PolygonTool::mousePressEvent(QMouseEvent* event)
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

void PolygonTool::mouseMoveEvent(QMouseEvent* event)
{
    if (m_state != State::Drawing)
    {
        handleRightButtonMove(event);
        return;
    }

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

        QRectF rect(m_mousePos, currentPos);
        rect          = rect.normalized();
        m_previewRect = rect;

        QVector<QPointF> pts = geometryMath::buildRegularPolygon(rect, 5);

        m_previewPath = QPainterPath();
        m_previewPath.moveTo(pts[0]);
        for (int i = 1; i < pts.size(); ++i)
            m_previewPath.lineTo(pts[i]);

        m_previewPath.closeSubpath();

        m_canvasView->requestFullUpdate();
    }

    handleRightButtonMove(event);
}

void PolygonTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
        return;
    }

    if (m_state == State::Drawing)
    {
        if (!m_previewRect.isValid())
        {
            cancelDrawing();
            return;
        }

        if (QVector<QPointF> points = geometryMath::buildRegularPolygon(m_previewRect, 5); !points.isEmpty())
        {
            if (points.first() != points.last())
            {
                points.append(points.first());
            }

            auto* shape = new ShapeVector();
            shape->setSemantic(VectorSemantic::Polygon);
            shape->segments() = geometryMath::buildPolylineSegments(points);
            shape->setColor(AppSettings::instance().activeColor());
            m_canvas->addShape(shape);
            m_canvas->shapeManager()->selectShape(shape, true);
        }

        cancelDrawing();
    }
}

DrawingToolType PolygonTool::toolType()
{
    return DrawingToolType::Polygon;
}

}// namespace xcanvas
