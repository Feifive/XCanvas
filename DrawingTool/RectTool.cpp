#include "RectTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "Polyline.h"
#include "ShapeManager.h"
#include <QMouseEvent>

xcanvas::RectTool::RectTool(MyGraphicsView* pView) : DrawingTool(pView)
{
}

xcanvas::RectTool::~RectTool()
{
}

void xcanvas::RectTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        handleRightButtonPress(event);
        return;
    }

    if(m_state == State::Idle)
    {
        m_state = State::Drawing;
        m_mousePos = m_pView->mapToScene(event->pos());
    }
}

void xcanvas::RectTool::mouseMoveEvent(QMouseEvent* event)
{
    if (m_state == State::Drawing)
    {
        QPointF currentPos = m_pView->mapToScene(event->pos());

        QPointF p1 = m_mousePos;
        QPointF p2(currentPos.x(), m_mousePos.y());
        QPointF p3 = currentPos;
        QPointF p4(m_mousePos.x(), currentPos.y());

        m_previewPath = QPainterPath(p1);
        m_previewPath.lineTo(p2);
        m_previewPath.lineTo(p3);
        m_previewPath.lineTo(p4);
        m_previewPath.closeSubpath();

        m_pView->updateCanvas();
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
        QPointF endPos = m_pView->mapToScene(event->pos());

        QVector<QPointF> points {
            m_mousePos,
            QPointF(endPos.x(), m_mousePos.y()),
            endPos,
            QPointF(m_mousePos.x(), endPos.y()),
            m_mousePos
        };

        if (points[0] == points[2]) {
            cancelDrawing();
            return;
        }

        Polyline* pShape = new Polyline;
        pShape->SetPoints(points);
        pShape->setSelected(true);

        m_pView->GetCurrentShapes()->deselectAll();
        m_pView->addShape(pShape);

        cancelDrawing();
    }
}

DrawingToolType xcanvas::RectTool::toolType()
{
    return DrawingToolType::Rect;
}
