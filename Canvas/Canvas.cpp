#include "Canvas.h"
#include "ShapeManager.h"
#include <QUndoStack>

namespace xcanvas {
    Canvas::Canvas(QObject *parent) : QObject(parent),
    m_canvasRect(QRectF(10000, 10000, 1280, 720)),
    m_undoStack(new QUndoStack(this)),
    m_shapeManager(new ShapeManager) {
    }

    Canvas::~Canvas() {
    }

    QRectF Canvas::canvasRect() const {
        return m_canvasRect;
    }

    void Canvas::setCanvasRect(const QRectF &rect) {
        m_canvasRect = rect;
    }

    ShapeManager * Canvas::shapeManager() const {
        return m_shapeManager;
    }

    QUndoStack * Canvas::undoStack() const {
        return m_undoStack;
    }
} // xcanvas