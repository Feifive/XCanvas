#include "ShapeInteractionController.h"

#include "TextEditController.h"
#include "../Canvas/Canvas.h"
#include "../Canvas/ICanvasViewport.h"
#include "DrawingTool.h"
#include "../Shape/Shape.h"
#include "../Shape/ShapeManager.h"
#include "../Shape/ShapeText.h"

#include <QMouseEvent>

ShapeInteractionController::ShapeInteractionController(
    ICanvasViewport* const    view,
    xcanvas::Canvas* const    canvas,
    IsSelectTool              isSelectTool,
    TextEditController* const textEditController)
    : m_view(view),
      m_canvas(canvas),
      m_isSelectTool(std::move(isSelectTool)),
      m_textEditController(textEditController)
{
}

bool ShapeInteractionController::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!event || event->button() != Qt::LeftButton || !m_isSelectTool || !m_isSelectTool() || !m_view || !m_textEditController)
    {
        return false;
    }

    const QPointF scenePos = m_view->mapToWorld(event->pos());
    xcanvas::Shape* const shape = findTopShapeAtScenePos(scenePos);
    if (!shape || shape->type() != xcanvas::ShapeType::Text)
    {
        return false;
    }

    auto* const textShape = static_cast<xcanvas::ShapeText*>(shape);
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return false;
    }

    m_canvas->shapeManager()->selectShape(textShape, true);
    if (!m_textEditController->beginInlineEdit(textShape, scenePos))
    {
        return false;
    }

    event->accept();
    return true;
}

xcanvas::Shape* ShapeInteractionController::findTopShapeAtScenePos(const QPointF& scenePos) const
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_view)
    {
        return nullptr;
    }

    const xcanvas::ShapeList shapeList = m_canvas->shapeManager()->shapes();
    const double tolerance = 6.0 / m_view->zoomScale();
    for (int i = shapeList.size() - 1; i >= 0; --i)
    {
        xcanvas::Shape* const shape = shapeList.at(i);
        if (!shape || !shape->isVisible())
        {
            continue;
        }
        if (shape->hitTest(scenePos, tolerance))
        {
            return shape;
        }
    }

    return nullptr;
}
