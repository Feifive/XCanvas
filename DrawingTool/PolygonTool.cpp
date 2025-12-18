#include "PolygonTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "MyMath.h"
#include "Polyline.h"

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

        QVector<QPointF> points = MyMath::buildRegularPolygon(rect, 5);
        if (!points.isEmpty())
        {
            if (points.first() != points.last())
            {
                points.append(points.first());
            }

            Polyline* pShape = new Polyline;
            pShape->SetPoints(points);
            pShape->setSelected(true);

            m_canvas->shapeManager()->deselectAll();
            m_canvas->addShape(pShape);
        }

        cancelDrawing();
    }
}

DrawingToolType PolygonTool::toolType()
{
    return DrawingToolType::Polygon;
}

}// namespace xcanvas
