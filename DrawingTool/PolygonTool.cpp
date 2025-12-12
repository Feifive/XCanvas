#include "PolygonTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "MyMath.h"
#include "Polyline.h"
#include "Shapes.h"

#include <QPointF>
#include <QRectF>

namespace xcanvas {

PolygonTool::PolygonTool(MyGraphicsView* pView) : DrawingTool(pView)
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

    if (event->button() == Qt::LeftButton)
    {
        if (m_state == State::Idle)
        {
            m_mousePos = m_pView->mapToScene(event->pos());

            m_state = State::Drawing;

            m_previewPath = QPainterPath();
        }
    }
}

void PolygonTool::mouseMoveEvent(QMouseEvent* event)
{
    if(m_state != State::Drawing)
    {
        handleRightButtonMove(event);
        return;
	}

    if (m_state == State::Drawing)
    {
		QPointF currentPos = m_pView->mapToScene(event->pos());
		QRectF rect(m_mousePos, currentPos);
		rect = rect.normalized();

        QVector<QPointF> pts = MyMath::buildRegularPolygon(rect, 5);

        m_previewPath = QPainterPath();
        m_previewPath.moveTo(pts[0]);
        for (int i = 1; i < pts.size(); ++i)
            m_previewPath.lineTo(pts[i]);

        m_previewPath.closeSubpath();

        m_pView->updateCanvas();
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
        QPointF currentPos = m_pView->mapToScene(event->pos());
        QRectF rect(m_mousePos, currentPos);
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

            m_pView->GetCurrentShapes()->deselectAll();
			m_pView->addShape(pShape);
        }

        m_state = State::Idle;

        m_pView->updateCanvas();
    }
}

DrawingToolType PolygonTool::toolType()
{
	return DrawingToolType::Polygon;
}

}


