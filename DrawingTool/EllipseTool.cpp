#include "EllipseTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Ellipse.h"
#include "Global.h"
#include <QDebug>
#include <QGraphicsEllipseItem>
#include <QMouseEvent>

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
        QRectF rect = m_previewPath.boundingRect();

        Ellipse* pShape = new Ellipse();
        pShape->setEllipse(rect);
        pShape->setSelected(true);

        m_canvas->shapeManager()->deselectAll();
        m_canvas->addShape(pShape);

        cancelDrawing();
    }
}

DrawingToolType xcanvas::EllipseTool::toolType()
{
    return DrawingToolType::Ellipse;
}
