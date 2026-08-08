#include "SelectionHudController.h"

#include "../Canvas/Canvas.h"
#include "Global.h"
#include "MyMath.h"
#include "SelectionHudBar.h"
#include "../Shape/Shape.h"
#include "../Shape/TransformCommand.h"
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include <map>

SelectionHudController::SelectionHudController(
    xcanvas::Canvas* const canvas,
    SelectionHudBar* const selectionHudBar,
    RequestFullUpdate      requestFullUpdate)
    : m_canvas(canvas),
      m_selectionHudBar(selectionHudBar),
      m_requestFullUpdate(std::move(requestFullUpdate)),
      m_keepAspectRatio(true)
{
}

void SelectionHudController::setKeepAspectRatio(const bool enabled)
{
    m_keepAspectRatio = enabled;
}

void SelectionHudController::applySelectionHudX(const double newX)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  currentX      = sceneToCanvas(selectionRect.topLeft()).x();
    const qreal  dx            = newX - currentX;
    if (qAbs(dx) < 1e-6)
    {
        return;
    }

    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        shape->translate(QPointF(dx, 0));
    }

    if (beforeTransform.empty())
    {
        return;
    }

    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), QStringLiteral("Move Selection X")));
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    updateSelectionHud(false);
}

void SelectionHudController::applySelectionHudY(const double newY)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  currentY      = sceneToCanvas(selectionRect.topLeft()).y();
    const qreal  dy            = newY - currentY;
    if (qAbs(dy) < 1e-6)
    {
        return;
    }

    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        shape->translate(QPointF(0, dy));
    }

    if (beforeTransform.empty())
    {
        return;
    }

    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), QStringLiteral("Move Selection Y")));
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    updateSelectionHud(false);
}

void SelectionHudController::applySelectionHudW(const double newW)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack() || newW <= 0.0)
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  oldW          = selectionRect.width();
    if (oldW < 1e-6)
    {
        return;
    }

    const qreal sx = newW / oldW;
    if (qAbs(sx - 1.0) < 1e-6)
    {
        return;
    }

    const QPointF anchor = selectionRect.topLeft();
    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        if (m_keepAspectRatio)
        {
            shape->scale(sx, sx, anchor);
        }
        else
        {
            shape->scale(sx, 1.0, anchor);
        }
    }

    if (beforeTransform.empty())
    {
        return;
    }

    const QString cmdText = m_keepAspectRatio ? QStringLiteral("Resize Selection (Keep Ratio)") : QStringLiteral("Resize Selection Width");
    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), cmdText));
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    updateSelectionHud(false);
}

void SelectionHudController::applySelectionHudH(const double newH)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack() || newH <= 0.0)
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  oldH          = selectionRect.height();
    if (oldH < 1e-6)
    {
        return;
    }

    const qreal sy = newH / oldH;
    if (qAbs(sy - 1.0) < 1e-6)
    {
        return;
    }

    const QPointF anchor = selectionRect.topLeft();
    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        if (m_keepAspectRatio)
        {
            shape->scale(sy, sy, anchor);
        }
        else
        {
            shape->scale(1.0, sy, anchor);
        }
    }

    if (beforeTransform.empty())
    {
        return;
    }

    const QString cmdText = m_keepAspectRatio ? QStringLiteral("Resize Selection (Keep Ratio)") : QStringLiteral("Resize Selection Height");
    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), cmdText));
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    updateSelectionHud(false);
}

void SelectionHudController::applySelectionHudAngle(const double newAngle)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    if (selectedShapes.size() == 1)
    {
        xcanvas::Shape* shape = selectedShapes.first();
        if (!shape)
        {
            return;
        }

        qreal delta = newAngle - shape->rotationDeg();
        if (delta > 180.0)
        {
            delta -= 360.0;
        }
        else if (delta < -180.0)
        {
            delta += 360.0;
        }
        if (qAbs(delta) < 1e-6)
        {
            return;
        }

        beforeTransform[shape] = shape->transform();
        shape->rotate(delta, shape->boundingRect().center());
    }
    else
    {
        const qreal delta = newAngle;
        if (qAbs(delta) < 1e-6)
        {
            return;
        }

        const QPointF center = m_canvas->shapeManager()->selectedBoundingRect().center();
        for (xcanvas::Shape* shape : selectedShapes)
        {
            if (!shape)
            {
                continue;
            }
            beforeTransform[shape] = shape->transform();
            shape->rotate(delta, center);
        }
    }

    if (beforeTransform.empty())
    {
        return;
    }

    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), QStringLiteral("Rotate Selection")));
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    updateSelectionHud(false);
}

void SelectionHudController::updateSelectionHud(const bool isDestroying)
{
    if (isDestroying || !m_selectionHudBar || !m_canvas || !m_canvas->shapeManager() || !m_selectionHudBar->isVisible())
    {
        return;
    }

    const QRectF  selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const QPointF canvasPos     = sceneToCanvas(selectionRect.topLeft());
    auto* spinX     = m_selectionHudBar->spinX();
    auto* spinY     = m_selectionHudBar->spinY();
    auto* spinW     = m_selectionHudBar->spinW();
    auto* spinH     = m_selectionHudBar->spinH();
    auto* spinAngle = m_selectionHudBar->spinAngle();

    const QSignalBlocker blockerX(spinX);
    const QSignalBlocker blockerY(spinY);
    const QSignalBlocker blockerW(spinW);
    const QSignalBlocker blockerH(spinH);
    const QSignalBlocker blockerAngle(spinAngle);

    spinX->setValue(canvasPos.x());
    spinY->setValue(canvasPos.y());
    spinW->setValue(selectionRect.width());
    spinH->setValue(selectionRect.height());
    if (const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList(); selectedShapes.size() == 1 && selectedShapes.first())
    {
        spinAngle->setValue(selectedShapes.first()->rotationDeg());
    }
    else
    {
        spinAngle->setValue(0.0);
    }
}
