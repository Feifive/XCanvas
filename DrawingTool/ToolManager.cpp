#include "ToolManager.h"
#include "../MyGraphicsView.h"

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
    m_shapeEditor.reset();
    m_shapeEditor = std::make_unique<ShapeEditor>(m_view, m_canvas);
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
            emit drawingFinished();
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

bool ToolManager::booleanUnion() {
    return m_shapeEditor->booleanUnion();
}

bool ToolManager::booleanIntersection() {
    return m_shapeEditor->booleanIntersection();
}

bool ToolManager::booleanSubtractAB() {
    return m_shapeEditor->booleanSubtractAB();
}

bool ToolManager::booleanSubtractBA() {
    return m_shapeEditor->booleanSubtractBA();
}

bool ToolManager::mirrorHorizontal() {
    return m_shapeEditor->mirrorHorizontal();
}

bool ToolManager::mirrorVertical() {
    return m_shapeEditor->mirrorVertical();
}

bool ToolManager::alignLeft() {
    return m_shapeEditor->alignLeft();
}

bool ToolManager::alignRight() {
    return m_shapeEditor->alignRight();
}

bool ToolManager::alignTop() {
    return m_shapeEditor->alignTop();
}

bool ToolManager::alignBottom() {
    return m_shapeEditor->alignBottom();
}

bool ToolManager::alignHorizontalCenter() {
    return m_shapeEditor->alignHorizontalCenter();
}

bool ToolManager::alignVerticalCenter() {
    return m_shapeEditor->alignVerticalCenter();
}

bool ToolManager::alignCenter() {
    return m_shapeEditor->alignCenter();
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
