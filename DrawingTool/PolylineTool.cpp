#include "PolylineTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "ShapeVector.h"
#include <QGraphicsView>
#include <QMouseEvent>

#include "MyMath.h"

xcanvas::PolylineTool::PolylineTool(MyGraphicsView* view, Canvas* canvas) : DrawingTool(view, canvas)
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

    m_mousePos = m_canvasView->mapToScene(event->pos());

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
        QPointF currentPos = m_canvasView->mapToScene(event->pos());
        m_points.last()    = currentPos;

        m_previewPath = QPainterPath();
        m_previewPath.moveTo(m_points[0]);
        for (int i = 1; i < m_points.size(); ++i)
        {
            m_previewPath.lineTo(m_points[i]);
        }

        m_canvasView->requestFullUpdate();
    }

    handleRightButtonMove(event);
}

void xcanvas::PolylineTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
    }
}

DrawingToolType xcanvas::PolylineTool::toolType()
{
    return DrawingToolType::Polyline;
}

void xcanvas::PolylineTool::cancelDrawing()
{
    m_points.removeLast();

    if (m_points.size() < 2)
    {
        m_previewPath = QPainterPath();
    }
    else
    {
        auto* shape = new ShapeVector();
        shape->setSemantic(VectorSemantic::Polyline);
        shape->segments() = geometryMath::buildPolylineSegments(m_points);
        m_canvas->addShape(shape);
        m_canvas->shapeManager()->selectShape(shape, true);
    }

    DrawingTool::cancelDrawing();
}
