#include "CanvasCamera.h"

#include "Global.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace xcanvas
{

namespace
{
constexpr qreal kScaleComparisonEpsilon = 1e-12;

bool nearlyEqual(const qreal lhs, const qreal rhs)
{
    return std::abs(lhs - rhs) <= kScaleComparisonEpsilon * std::max<qreal>({1.0, std::abs(lhs), std::abs(rhs)});
}
}// namespace

CanvasCamera::CanvasCamera(const qreal scale, const QPointF& center)
{
    setScale(scale);
    setCenter(center);
}

qreal CanvasCamera::scale() const
{
    return m_scale;
}

QPointF CanvasCamera::center() const
{
    return m_center;
}

bool CanvasCamera::setScale(const qreal scale)
{
    if (!qIsFinite(scale) || scale <= 0.0)
    {
        return false;
    }

    const qreal nextScale = boundedScale(scale);
    if (nearlyEqual(nextScale, m_scale))
    {
        return false;
    }

    m_scale = nextScale;
    return true;
}

bool CanvasCamera::setCenter(const QPointF& center)
{
    if (!isFinitePoint(center) || center == m_center)
    {
        return false;
    }

    m_center = center;
    return true;
}

QTransform CanvasCamera::worldToView(const QSizeF& viewportSize) const
{
    if (!isValidViewport(viewportSize))
    {
        return QTransform();
    }

    QTransform transform;
    transform.translate(viewportSize.width() / 2.0, viewportSize.height() / 2.0);
    transform.scale(m_scale, m_scale);
    transform.translate(-m_center.x(), -m_center.y());
    return transform;
}

QTransform CanvasCamera::viewToWorld(const QSizeF& viewportSize) const
{
    bool invertible = false;
    const QTransform inverse = worldToView(viewportSize).inverted(&invertible);
    return invertible ? inverse : QTransform();
}

QPointF CanvasCamera::mapFromWorld(const QPointF& worldPoint, const QSizeF& viewportSize) const
{
    if (!isFinitePoint(worldPoint) || !isValidViewport(viewportSize))
    {
        return QPointF();
    }
    return worldToView(viewportSize).map(worldPoint);
}

QPointF CanvasCamera::mapToWorld(const QPointF& viewPoint, const QSizeF& viewportSize) const
{
    if (!isFinitePoint(viewPoint) || !isValidViewport(viewportSize))
    {
        return QPointF();
    }
    return viewToWorld(viewportSize).map(viewPoint);
}

QRectF CanvasCamera::mapFromWorld(const QRectF& worldRect, const QSizeF& viewportSize) const
{
    if (!worldRect.isValid() || worldRect.isEmpty() || !isValidViewport(viewportSize))
    {
        return QRectF();
    }
    return worldToView(viewportSize).mapRect(worldRect);
}

QRectF CanvasCamera::mapToWorld(const QRectF& viewRect, const QSizeF& viewportSize) const
{
    if (!viewRect.isValid() || viewRect.isEmpty() || !isValidViewport(viewportSize))
    {
        return QRectF();
    }
    return viewToWorld(viewportSize).mapRect(viewRect);
}

QRectF CanvasCamera::visibleWorldRect(const QSizeF& viewportSize) const
{
    if (!isValidViewport(viewportSize))
    {
        return QRectF();
    }
    return mapToWorld(QRectF(QPointF(), viewportSize), viewportSize);
}

bool CanvasCamera::zoomAt(
    const QPointF& viewAnchor,
    const qreal targetScale,
    const QSizeF& viewportSize)
{
    if (!isFinitePoint(viewAnchor) || !qIsFinite(targetScale) || targetScale <= 0.0
        || !isValidViewport(viewportSize))
    {
        return false;
    }

    const qreal nextScale = boundedScale(targetScale);
    if (nearlyEqual(nextScale, m_scale))
    {
        return false;
    }

    const QPointF worldAnchor = mapToWorld(viewAnchor, viewportSize);
    const QPointF viewportCenter(viewportSize.width() / 2.0, viewportSize.height() / 2.0);
    const QPointF nextCenter = worldAnchor - (viewAnchor - viewportCenter) / nextScale;
    if (!isFinitePoint(nextCenter))
    {
        return false;
    }

    m_scale = nextScale;
    m_center = nextCenter;
    return true;
}

bool CanvasCamera::panByViewDelta(const QPointF& delta)
{
    if (!isFinitePoint(delta) || delta.isNull())
    {
        return false;
    }

    const QPointF nextCenter = m_center - delta / m_scale;
    if (!isFinitePoint(nextCenter))
    {
        return false;
    }

    m_center = nextCenter;
    return true;
}

bool CanvasCamera::centerOn(const QPointF& worldPoint)
{
    return setCenter(worldPoint);
}

std::optional<CanvasCamera::FitResult> CanvasCamera::calculateFit(
    const QRectF& worldRect,
    const QSizeF& viewportSize,
    const FitMode mode)
{
    if (!worldRect.isValid() || worldRect.isEmpty() || !isValidViewport(viewportSize))
    {
        return std::nullopt;
    }

    const qreal widthScale = viewportSize.width() / worldRect.width();
    const qreal heightScale = viewportSize.height() / worldRect.height();
    qreal targetScale = 1.0;
    switch (mode)
    {
    case FitMode::Width:
        targetScale = widthScale;
        break;
    case FitMode::Height:
        targetScale = heightScale;
        break;
    case FitMode::Contain:
        targetScale = std::min(widthScale, heightScale);
        break;
    }

    if (!qIsFinite(targetScale) || targetScale <= 0.0 || !isFinitePoint(worldRect.center()))
    {
        return std::nullopt;
    }

    return FitResult{boundedScale(targetScale), worldRect.center()};
}

bool CanvasCamera::fitToRect(const QRectF& worldRect, const QSizeF& viewportSize, const FitMode mode)
{
    const std::optional<FitResult> result = calculateFit(worldRect, viewportSize, mode);
    if (!result)
    {
        return false;
    }

    const bool changed = !nearlyEqual(m_scale, result->scale) || m_center != result->center;
    m_scale = result->scale;
    m_center = result->center;
    return changed;
}

qreal CanvasCamera::boundedScale(const qreal scale)
{
    return std::clamp(scale, static_cast<qreal>(MIN_ZOOM), static_cast<qreal>(MAX_ZOOM));
}

bool CanvasCamera::isValidViewport(const QSizeF& viewportSize)
{
    return viewportSize.isValid() && !viewportSize.isEmpty()
        && qIsFinite(viewportSize.width()) && qIsFinite(viewportSize.height());
}

bool CanvasCamera::isFinitePoint(const QPointF& point)
{
    return qIsFinite(point.x()) && qIsFinite(point.y());
}

}// namespace xcanvas
