#ifndef CANVASCAMERA_H
#define CANVASCAMERA_H

#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QTransform>

#include <optional>

namespace xcanvas
{

class CanvasCamera final
{
  public:
    enum class FitMode
    {
        Width,
        Height,
        Contain
    };

    struct FitResult
    {
        qreal   scale;
        QPointF center;
    };

    CanvasCamera() = default;
    CanvasCamera(qreal scale, const QPointF& center = QPointF());

    qreal scale() const;
    QPointF center() const;

    bool setScale(qreal scale);
    bool setCenter(const QPointF& center);

    QTransform worldToView(const QSizeF& viewportSize) const;
    QTransform viewToWorld(const QSizeF& viewportSize) const;

    QPointF mapFromWorld(const QPointF& worldPoint, const QSizeF& viewportSize) const;
    QPointF mapToWorld(const QPointF& viewPoint, const QSizeF& viewportSize) const;
    QRectF mapFromWorld(const QRectF& worldRect, const QSizeF& viewportSize) const;
    QRectF mapToWorld(const QRectF& viewRect, const QSizeF& viewportSize) const;
    QRectF visibleWorldRect(const QSizeF& viewportSize) const;

    bool zoomAt(const QPointF& viewAnchor, qreal targetScale, const QSizeF& viewportSize);
    bool panByViewDelta(const QPointF& delta);
    bool centerOn(const QPointF& worldPoint);

    static std::optional<FitResult> calculateFit(
        const QRectF& worldRect,
        const QSizeF& viewportSize,
        FitMode mode);
    bool fitToRect(const QRectF& worldRect, const QSizeF& viewportSize, FitMode mode);

  private:
    static qreal boundedScale(qreal scale);
    static bool isValidViewport(const QSizeF& viewportSize);
    static bool isFinitePoint(const QPointF& point);

    qreal   m_scale{1.0};
    QPointF m_center;
};

}// namespace xcanvas

#endif// CANVASCAMERA_H
