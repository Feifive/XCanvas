#include "PolygonTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "MyMath.h"
#include "Vector.h"

#include <QPointF>
#include <QRectF>

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
        QRectF  rect(m_mousePos, currentPos);
        rect = rect.normalized();

        QVector<QPointF> pts = MyMath::buildRegularPolygon(rect, 5);

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
        QPointF currentPos = m_canvasView->mapToScene(event->pos());
        QRectF  rect(m_mousePos, currentPos);
        rect = rect.normalized();

        if (!rect.isValid())
        {
            cancelDrawing();
            return;
        }

        if (QVector<QPointF> points = MyMath::buildRegularPolygon(rect, 5); !points.isEmpty())
        {
            if (points.first() != points.last())
            {
                points.append(points.first());
            }

            auto* shape = new Vector();
            shape->setSemantic(VectorSemantic::Polygon);
            shape->setSelected(true);
            shape->moveTo(points[0]);
            for (int i = 1; i < points.size(); ++i) {
                shape->lineTo(points[i]);
            }

            m_canvas->shapeManager()->deselectAll();
            m_canvas->addShape(shape);
        }

        cancelDrawing();
    }
}

DrawingToolType PolygonTool::toolType()
{
    return DrawingToolType::Polygon;
}

}// namespace xcanvas
