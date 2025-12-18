#include "ToolManager.h"
#include "../MyGraphicsView.h"

#include "EventBus.h"

#include "SelectTool.h"
#include "RectTool.h"
#include "EllipseTool.h"
#include "PolylineTool.h"
#include "CurveTool.h"
#include "PolygonTool.h"
#include "TextTool.h"

namespace xcanvas {

ToolManager::ToolManager(MyGraphicsView* view, xcanvas::Canvas* canvas)
    : QObject(view)
    , m_view(view)
    , m_canvas(canvas)
{
    setTool(DrawingToolType::Select);
}

void ToolManager::setTool(const DrawingToolType type)
{
    if (type == m_currentType) {
        return;
    }

    m_currentTool.reset();
    m_currentTool = createTool(type);
    m_currentType = type;

    if (m_currentTool) {
        connect(m_currentTool.get(), &DrawingTool::finished, this, [this]() {
            setTool(DrawingToolType::Select);
            emit EventBus::instance().finishDrawing();
        });
    }

        emit toolChanged(type);
}

std::unique_ptr<DrawingTool> ToolManager::createTool(const DrawingToolType type)
{
    switch (type)
    {
        case DrawingToolType::Select:
            return std::make_unique<SelectTool>(m_view, m_canvas);
        case DrawingToolType::Rect:
            return std::make_unique<RectTool>(m_view, m_canvas);
        case DrawingToolType::Ellipse:
            return std::make_unique<EllipseTool>(m_view, m_canvas);
        case DrawingToolType::Polyline:
            return std::make_unique<PolylineTool>(m_view, m_canvas);
        case DrawingToolType::Curve:
            return std::make_unique<CurveTool>(m_view, m_canvas);
        case DrawingToolType::Polygon:
            return std::make_unique<PolygonTool>(m_view, m_canvas);
        case DrawingToolType::Text:
            return std::make_unique<TextTool>(m_view, m_canvas);
        default:
            break;
    }
    return nullptr;
}

void ToolManager::mousePressEvent(QMouseEvent* e)
{
    if (m_currentTool)
        m_currentTool->mousePressEvent(e);
}

void ToolManager::mouseMoveEvent(QMouseEvent* e)
{
    if (m_currentTool)
        m_currentTool->mouseMoveEvent(e);
}

void ToolManager::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_currentTool)
        m_currentTool->mouseReleaseEvent(e);
}

void ToolManager::keyPressEvent(QKeyEvent* e)
{
    if (m_currentTool)
        m_currentTool->keyPressEvent(e);
}

void ToolManager::drawPreview(QPainter* painter)
{
    if (m_currentTool)
        m_currentTool->drawPreview(painter);
}

}
