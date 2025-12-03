#include "EllipseTool.h"
#include "Global.h"
#include "../MyGraphicsView.h"
#include "Ellipse.h"
#include "Shapes.h"
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
    m_startPos = m_pView->mapToScene(event->pos());

    if(event->button() == Qt::LeftButton)
    {
        if(m_state == State::Idle)
        {
            m_previewPath = QPainterPath();

            m_state = State::Drawing;
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
    }
}

void xcanvas::EllipseTool::mouseMoveEvent(QMouseEvent *event)
{
    if(m_state == State::Drawing)
    {
        QPointF currentPos = m_pView->mapToScene(event->pos());

        QRectF rect(
            qMin(m_startPos.x(), currentPos.x()),
            qMin(m_startPos.y(), currentPos.y()),
            qAbs(currentPos.x() - m_startPos.x()),
            qAbs(currentPos.y() - m_startPos.y())
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
        pShape->setSelected(true);
        pShape->setEllipse(rect);

        m_pView->GetCurrentShapes()->deselectAll();
        m_pView->GetCurrentShapes()->addShape(pShape);

        m_state = State::Idle;

        m_pView->updateCanvas();
    }
}

DrawingToolType xcanvas::EllipseTool::toolType()
{
    return DrawingToolType::Ellipse;
}
