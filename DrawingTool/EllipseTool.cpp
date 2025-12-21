#include "EllipseTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include "MyMath.h"
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
        const QRectF rect = m_previewPath.boundingRect();
        auto* shape = new Vector();
        if (qFuzzyCompare(rect.width(), rect.height())) {
            shape->setSemantic(VectorSemantic::Circle);
        }
        else {
            shape->setSemantic(VectorSemantic::Ellipse);
        }

        shape->segments() = MyMath::buildEllipseSegments(rect);
        shape->setSelected(true);

        m_canvas->shapeManager()->deselectAll();
        m_canvas->addShape(shape);

        cancelDrawing();
    }
}

DrawingToolType xcanvas::EllipseTool::toolType()
{
    return DrawingToolType::Ellipse;
}
