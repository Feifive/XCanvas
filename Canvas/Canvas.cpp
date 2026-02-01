#include "Canvas.h"
#include "AddShapesCommand.h"
#include "RemoveShapesCommand.h"
#include <QUndoStack>

namespace xcanvas
{
Canvas::Canvas(QObject* parent)
    : QObject(parent), m_canvasRect(QRectF(10000, 10000, 900, 600)), m_undoStack(new QUndoStack(this)), m_shapeManager(new ShapeManager(this)), m_layerManager(new LayerManager(this))
{
}

Canvas::~Canvas()
{
}

QRectF Canvas::canvasRect() const
{
    return m_canvasRect;
}

void Canvas::setCanvasRect(const QRectF& rect)
{
    m_canvasRect = rect;
}

ShapeManager* Canvas::shapeManager() const
{
    return m_shapeManager;
}

LayerManager* Canvas::layerManager()
{
    return m_layerManager;
}

QUndoStack* Canvas::undoStack() const
{
    return m_undoStack;
}

void Canvas::addShape(Shape* shape)
{
    if (shape)
    {
        addShapes({shape});
    }
}

void Canvas::addShapes(const ShapeList& shapeList)
{
    if (shapeList.isEmpty())
    {
        return;
    }
    m_undoStack->push(new xcanvas::AddShapesCommand(m_shapeManager, m_layerManager, shapeList));
}

void Canvas::removeShape(Shape* shape)
{
    if (shape)
    {
        removeShapes({shape});
    }
}

void Canvas::removeShapes(const ShapeList& shapeList)
{
    if (shapeList.isEmpty())
    {
        return;
    }

    m_undoStack->push(new xcanvas::RemoveShapesCommand(m_shapeManager, m_layerManager, shapeList));
}

}// namespace xcanvas
