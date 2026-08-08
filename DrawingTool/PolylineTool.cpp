#include "PolylineTool.h"
#include "../Canvas/ICanvasViewport.h"
#include "Canvas.h"
#include "Global.h"
#include "ShapeVector.h"
#include <QMouseEvent>

#include "AppSettings.h"
#include "MyMath.h"

xcanvas::PolylineTool::PolylineTool(ICanvasViewport* view, Canvas* canvas) : DrawingTool(view, canvas)
{
}

xcanvas::PolylineTool::~PolylineTool()
{
}

void xcanvas::PolylineTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    m_mousePos = m_canvasView->mapToWorld(event->pos());

    if (m_state == State::Idle)
    {
        m_points.clear();
        m_points.append(m_mousePos);
        m_points.append(m_mousePos);

        m_previewPath = QPainterPath();
        m_previewPath.moveTo(m_mousePos);
        m_previewPath.lineTo(m_mousePos);

        m_state = State::Drawing;
    }
    else if (m_state == State::Drawing)
    {
        m_points.last() = m_mousePos;
        m_points.append(m_mousePos);
    }
}

void xcanvas::PolylineTool::mouseMoveEvent(QMouseEvent* event)
{
    if (m_state == State::Drawing)
    {
        QPointF currentPos = m_canvasView->mapToWorld(event->pos());
        m_points.last()    = currentPos;

        m_previewPath = QPainterPath();
        m_previewPath.moveTo(m_points[0]);
        for (int i = 1; i < m_points.size(); ++i)
        {
            m_previewPath.lineTo(m_points[i]);
        }

        m_canvasView->requestUpdate();
    }

    handleRightButtonMove(event);
}

void xcanvas::PolylineTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        const bool shouldFinish = m_state == State::Drawing && m_isRightPressed && !m_isRightDragged;
        handleRightButtonRelease(event);
        if (shouldFinish)
        {
            finishDrawing();
        }
    }
}

DrawingToolType xcanvas::PolylineTool::toolType()
{
    return DrawingToolType::Polyline;
}

void xcanvas::PolylineTool::cancelDrawing()
{
    finalizeDrawing(true);
}

void xcanvas::PolylineTool::finishDrawing()
{
    finalizeDrawing(false);
}

void xcanvas::PolylineTool::finalizeDrawing(const bool canceled)
{
    if (!m_points.isEmpty())
    {
        m_points.removeLast();
    }

    if (!canceled && m_points.size() >= 2)
    {
        auto* shape = new ShapeVector();
        shape->setSemantic(VectorSemantic::Polyline);
        shape->segments() = geometryMath::buildPolylineSegments(m_points);
        shape->setColor(AppSettings::instance().activeColor());
        m_canvas->addShape(shape);
        m_canvas->shapeManager()->selectShape(shape, true);
    }

    if (canceled)
    {
        DrawingTool::cancelDrawing();
        return;
    }

    DrawingTool::finishDrawing();
}
