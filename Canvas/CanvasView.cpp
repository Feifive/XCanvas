#include "CanvasView.h"
#include "RulerInteractionPolicy.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSignalBlocker>

#include <algorithm>
#include <cmath>
#include <limits>

namespace xcanvas
{

namespace
{
int boundedScrollExtent(const qreal extent)
{
    if (!qIsFinite(extent) || extent <= 0.0)
    {
        return 0;
    }
    return static_cast<int>(std::min<qreal>(std::ceil(extent), std::numeric_limits<int>::max()));
}
}// namespace

CanvasView::CanvasView(QWidget* const parent)
    : QAbstractScrollArea(parent),
      m_camera(1.0, m_worldRect.center())
{
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    viewport()->setMouseTracking(true);
    viewport()->setFocusPolicy(Qt::NoFocus);
    viewport()->setAcceptDrops(true);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged,
            this, [this] { updateCameraFromScrollBars(); });
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this] { updateCameraFromScrollBars(); });
    updateScrollBars();
}

QWidget* CanvasView::viewportWidget() const { return viewport(); }
QWidget* CanvasView::hostWidget() const { return const_cast<CanvasView*>(this); }
QRect CanvasView::viewportRect() const { return viewport() ? viewport()->rect() : QRect(); }
QRect CanvasView::hostRect() const { return rect(); }
QRectF CanvasView::worldRect() const { return m_worldRect; }
qreal CanvasView::zoomScale() const { return m_camera.scale(); }

QPointF CanvasView::mapToWorld(const QPointF& point) const
{
    return m_camera.mapToWorld(point, viewportRect().size());
}

QPointF CanvasView::mapFromWorld(const QPointF& point) const
{
    return m_camera.mapFromWorld(point, viewportRect().size());
}

QRectF CanvasView::mapToWorld(const QRect& value) const
{
    return m_camera.mapToWorld(QRectF(value), viewportRect().size());
}

QRectF CanvasView::mapFromWorld(const QRectF& value) const
{
    return m_camera.mapFromWorld(value, viewportRect().size());
}

QRectF CanvasView::visibleWorldRect() const
{
    return m_camera.visibleWorldRect(viewportRect().size());
}

QPoint CanvasView::mapFromGlobalToView(const QPoint& point) const
{
    return viewport() ? viewport()->mapFromGlobal(point) : QPoint();
}

QPoint CanvasView::mapFromViewToGlobal(const QPoint& point) const
{
    return viewport() ? viewport()->mapToGlobal(point) : QPoint();
}

void CanvasView::setInputMethodEnabled(const bool enabled)
{
    setAttribute(Qt::WA_InputMethodEnabled, enabled);
}

void CanvasView::setViewCursor(const QCursor& value) { viewport()->setCursor(value); }
Qt::CursorShape CanvasView::viewCursorShape() const { return viewport()->cursor().shape(); }
void CanvasView::focusViewport() { setFocus(Qt::OtherFocusReason); }
void CanvasView::requestUpdate() const { viewport()->update(); }

void CanvasView::scaleBy(const qreal factor)
{
    if (qIsFinite(factor) && factor > 0.0)
    {
        setZoomScale(m_camera.scale() * factor);
    }
}

void CanvasView::translateWorld(const QPointF& delta)
{
    applyCameraChange(m_camera.setCenter(m_camera.center() - delta));
}

void CanvasView::centerOnWorld(const QPointF& point)
{
    applyCameraChange(m_camera.centerOn(point));
}

void CanvasView::scrollByViewDelta(const QPoint& delta)
{
    applyCameraChange(m_camera.panByViewDelta(delta));
}

void CanvasView::scrollAxisBy(const bool horizontal, const int delta)
{
    scrollByViewDelta(horizontal ? QPoint(delta, 0) : QPoint(0, delta));
}

const CanvasCamera& CanvasView::camera() const { return m_camera; }

void CanvasView::setWorldRect(const QRectF& rect)
{
    if (!rect.isValid() || rect.isEmpty()
        || !qIsFinite(rect.left()) || !qIsFinite(rect.top())
        || !qIsFinite(rect.width()) || !qIsFinite(rect.height())
        || rect == m_worldRect)
    {
        return;
    }
    m_worldRect = rect;
    if (updateScrollBars())
    {
        emit cameraChanged();
    }
    requestUpdate();
}

bool CanvasView::setZoomScale(const qreal scale)
{
    const bool changed = m_camera.setScale(scale);
    applyCameraChange(changed);
    return changed;
}

bool CanvasView::zoomAt(const QPointF& viewAnchor, const qreal targetScale)
{
    const bool changed = m_camera.zoomAt(viewAnchor, targetScale, viewportRect().size());
    applyCameraChange(changed);
    return changed;
}

const RulerRenderer& CanvasView::rulerRenderer() const { return m_rulerRenderer; }

RulerRenderer::HitArea CanvasView::rulerHitArea(const QPoint& point) const
{
    return m_rulerRenderer.hitTest(point, viewportRect());
}

bool CanvasView::isInRuler(const QPoint& point) const
{
    return rulerHitArea(point) != RulerRenderer::HitArea::None;
}

void CanvasView::paintEvent(QPaintEvent* const event)
{
    Q_UNUSED(event)
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    drawViewportBackground(painter, viewportRect());

    painter.save();
    painter.setWorldTransform(m_camera.worldToView(viewportRect().size()));
    drawWorld(painter, visibleWorldRect());
    painter.restore();

    drawViewOverlay(painter, viewportRect());
    m_rulerRenderer.draw(&painter, viewportRect(), viewportRect().size(),
                         m_camera, m_worldRect.center(), rulerBaseColor(), palette());
}

void CanvasView::resizeEvent(QResizeEvent* const event)
{
    QAbstractScrollArea::resizeEvent(event);
    if (updateScrollBars())
    {
        emit cameraChanged();
    }
    emit viewportGeometryChanged(viewportRect());
    requestUpdate();
}

void CanvasView::changeEvent(QEvent* const event)
{
    QAbstractScrollArea::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange)
    {
        requestUpdate();
    }
}

void CanvasView::mousePressEvent(QMouseEvent* const event)
{
    focusForPointerPress();
    if (filterRulerMousePress(event))
    {
        return;
    }
    QAbstractScrollArea::mousePressEvent(event);
}

void CanvasView::focusForPointerPress()
{
    setFocus(Qt::MouseFocusReason);
}

void CanvasView::mouseMoveEvent(QMouseEvent* const event)
{
    if (filterRulerMouseMove(event))
    {
        return;
    }
    QAbstractScrollArea::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent* const event)
{
    if (filterRulerMouseRelease(event))
    {
        return;
    }
    QAbstractScrollArea::mouseReleaseEvent(event);
}

void CanvasView::drawViewportBackground(QPainter& painter, const QRect& rect)
{
    painter.fillRect(rect, palette().color(QPalette::Base));
}

void CanvasView::drawWorld(QPainter&, const QRectF&) {}
void CanvasView::drawViewOverlay(QPainter&, const QRect&) {}
QColor CanvasView::rulerBaseColor() const { return palette().color(QPalette::Base); }

bool CanvasView::filterRulerMousePress(QMouseEvent* const event)
{
    if (!event || !isInRuler(event->position().toPoint()))
    {
        return false;
    }

    m_rulerPointerGrab = true;
    event->accept();
    return true;
}

bool CanvasView::filterRulerMouseMove(QMouseEvent* const event)
{
    if (!event
        || !RulerInteractionPolicy::blocksMove(
            m_rulerPointerGrab,
            isInRuler(event->position().toPoint()),
            event->buttons()))
    {
        return false;
    }

    event->accept();
    return true;
}

bool CanvasView::filterRulerMouseRelease(QMouseEvent* const event)
{
    if (!event || !m_rulerPointerGrab)
    {
        return false;
    }

    m_rulerPointerGrab = false;
    event->accept();
    return true;
}

void CanvasView::applyCameraChange(const bool changed)
{
    if (!changed)
    {
        return;
    }
    updateScrollBars();
    requestUpdate();
    emit cameraChanged();
}

bool CanvasView::updateScrollBars()
{
    if (m_syncingScrollBars || !viewport())
    {
        return false;
    }

    m_syncingScrollBars = true;
    const QSizeF size = viewportRect().size();
    const qreal scale = m_camera.scale();
    QPointF center = m_camera.center();

    auto configureAxis = [scale](QScrollBar* bar, const qreal worldStart,
                                 const qreal worldExtent, const qreal viewExtent,
                                 qreal& centerCoordinate)
    {
        const qreal naturalRange = std::max<qreal>(0.0, worldExtent * scale - viewExtent);
        const int range = boundedScrollExtent(naturalRange);
        const int pageStep = std::max(0, boundedScrollExtent(viewExtent));
        bar->setRange(0, range);
        bar->setPageStep(pageStep);
        bar->setSingleStep(20);

        if (range == 0)
        {
            centerCoordinate = worldStart + worldExtent / 2.0;
            bar->setValue(0);
            return;
        }

        const qreal halfVisibleWorld = viewExtent / (2.0 * scale);
        centerCoordinate = std::clamp(centerCoordinate,
                                      worldStart + halfVisibleWorld,
                                      worldStart + worldExtent - halfVisibleWorld);
        const qreal position = (centerCoordinate - worldStart) * scale - viewExtent / 2.0;
        const qreal projectedPosition = naturalRange > std::numeric_limits<int>::max()
            ? position * range / naturalRange
            : position;
        bar->setValue(std::clamp(qRound(projectedPosition), 0, range));
    };

    {
        const QSignalBlocker horizontalBlocker(horizontalScrollBar());
        const QSignalBlocker verticalBlocker(verticalScrollBar());
        configureAxis(horizontalScrollBar(), m_worldRect.left(), m_worldRect.width(),
                      size.width(), center.rx());
        configureAxis(verticalScrollBar(), m_worldRect.top(), m_worldRect.height(),
                      size.height(), center.ry());
    }

    const bool centerChanged = m_camera.setCenter(center);
    m_syncingScrollBars = false;
    return centerChanged;
}

void CanvasView::updateCameraFromScrollBars()
{
    if (m_syncingScrollBars || !viewport())
    {
        return;
    }

    QPointF center = m_camera.center();
    const QSizeF size = viewportRect().size();
    const qreal scale = m_camera.scale();
    if (horizontalScrollBar()->maximum() > 0)
    {
        const qreal naturalRange = m_worldRect.width() * scale - size.width();
        const qreal position = naturalRange > std::numeric_limits<int>::max()
            ? horizontalScrollBar()->value() * naturalRange / horizontalScrollBar()->maximum()
            : horizontalScrollBar()->value();
        center.setX(m_worldRect.left()
                    + (position + size.width() / 2.0) / scale);
    }
    else
    {
        center.setX(m_worldRect.center().x());
    }
    if (verticalScrollBar()->maximum() > 0)
    {
        const qreal naturalRange = m_worldRect.height() * scale - size.height();
        const qreal position = naturalRange > std::numeric_limits<int>::max()
            ? verticalScrollBar()->value() * naturalRange / verticalScrollBar()->maximum()
            : verticalScrollBar()->value();
        center.setY(m_worldRect.top()
                    + (position + size.height() / 2.0) / scale);
    }
    else
    {
        center.setY(m_worldRect.center().y());
    }

    if (m_camera.setCenter(center))
    {
        requestUpdate();
        emit cameraChanged();
    }
}

}// namespace xcanvas
