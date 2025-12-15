#include "EllipseTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "Ellipse.h"
#include "ShapeManager.h"
#include <QGraphicsEllipseItem>
#include <QMouseEvent>
#include <QDebug>

xcanvas::EllipseTool::EllipseTool(MyGraphicsView* pView) :
    DrawingTool(pView)
{}

xcanvas::EllipseTool::~EllipseTool()
{
}

void xcanvas::EllipseTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    if(m_state == State::Idle)
    {
        m_mousePos = m_pView->mapToScene(event->pos());
        m_previewPath = QPainterPath();
        m_state = State::Drawing;
    }
}

void xcanvas::EllipseTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_state == State::Drawing)
    {
        QPointF currentPos = m_pView->mapToScene(event->pos());

        QRectF rect(
            qMin(m_mousePos.x(), currentPos.x()),
            qMin(m_mousePos.y(), currentPos.y()),
            qAbs(currentPos.x() - m_mousePos.x()),
            qAbs(currentPos.y() - m_mousePos.y())
        );

        m_previewPath = QPainterPath();
        m_previewPath.addEllipse(rect);

        m_pView->updateCanvas();
    }

    handleRightButtonMove(event);
}

void xcanvas::EllipseTool::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
        return;
    }

    if(m_state == State::Drawing)
    {
        QRectF rect = m_previewPath.boundingRect();

        Ellipse* pShape = new Ellipse();
        pShape->setEllipse(rect);
        pShape->setSelected(true);

        m_pView->GetCurrentShapes()->deselectAll();
        m_pView->addShape(pShape);

        cancelDrawing();
    }
}

DrawingToolType xcanvas::EllipseTool::toolType()
{
    return DrawingToolType::Ellipse;
}
