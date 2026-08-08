#ifndef ICANVASVIEWPORT_H
#define ICANVASVIEWPORT_H

#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QCursor>

class QWidget;

// Coordinate and host-widget boundary used by rendering and editing code.
// Implementations provide the application-owned canvas mapping contract.
class ICanvasViewport
{
  public:
    virtual ~ICanvasViewport() = default;

    virtual QWidget* viewportWidget() const = 0;
    virtual QWidget* hostWidget() const = 0;
    virtual QRect viewportRect() const = 0;
    virtual QRect hostRect() const = 0;
    virtual QRectF worldRect() const = 0;
    virtual qreal zoomScale() const = 0;
    virtual QPointF mapToWorld(const QPointF& viewPoint) const = 0;
    virtual QPointF mapFromWorld(const QPointF& worldPoint) const = 0;
    virtual QRectF mapToWorld(const QRect& viewRect) const = 0;
    virtual QRectF mapFromWorld(const QRectF& worldRect) const = 0;
    virtual QPoint mapFromGlobalToView(const QPoint& globalPoint) const = 0;
    virtual QPoint mapFromViewToGlobal(const QPoint& viewPoint) const = 0;
    virtual void setInputMethodEnabled(bool enabled) = 0;
    virtual void setViewCursor(const QCursor& cursor) = 0;
    virtual Qt::CursorShape viewCursorShape() const = 0;
    virtual void focusViewport() = 0;
    virtual void requestUpdate() const = 0;
};

// Navigation is deliberately separate: most consumers only need mapping.
class ICanvasNavigation
{
  public:
    virtual ~ICanvasNavigation() = default;

    virtual void scaleBy(qreal factor) = 0;
    virtual void translateWorld(const QPointF& delta) = 0;
    virtual void centerOnWorld(const QPointF& point) = 0;
    virtual void scrollByViewDelta(const QPoint& delta) = 0;
    virtual void scrollAxisBy(bool horizontal, int delta) = 0;
};

#endif // ICANVASVIEWPORT_H
