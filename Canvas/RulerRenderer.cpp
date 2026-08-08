#include "RulerRenderer.h"

#include "CanvasCamera.h"

#include <QPainter>
#include <QtMath>

#include <algorithm>
#include <cmath>

namespace xcanvas
{

namespace
{
constexpr qreal labelEdgePadding = 3.0;
}

RulerRenderer::RulerRenderer(const int thickness)
    : m_thickness(std::max(0, thickness))
{
    m_font.setFamily("MiSans");
    m_font.setPixelSize(8);
}

int RulerRenderer::thickness() const
{
    return m_thickness;
}

void RulerRenderer::setThickness(const int thickness)
{
    m_thickness = std::max(0, thickness);
}

QColor RulerRenderer::backgroundColor(const QColor& baseColor) const
{
    QColor color = baseColor;
    color.setAlpha(204);
    return color;
}

QRect RulerRenderer::horizontalRect(const QRect& viewportRect) const
{
    return QRect(viewportRect.left(), viewportRect.top(), viewportRect.width(),
                 std::min(m_thickness, viewportRect.height()));
}

QRect RulerRenderer::verticalRect(const QRect& viewportRect) const
{
    return QRect(viewportRect.left(), viewportRect.top(),
                 std::min(m_thickness, viewportRect.width()), viewportRect.height());
}

QRect RulerRenderer::cornerRect(const QRect& viewportRect) const
{
    return horizontalRect(viewportRect).intersected(verticalRect(viewportRect));
}

RulerRenderer::HitArea RulerRenderer::hitTest(
    const QPoint& viewPoint,
    const QRect& viewportRect) const
{
    if (!viewportRect.contains(viewPoint))
    {
        return HitArea::None;
    }
    if (cornerRect(viewportRect).contains(viewPoint))
    {
        return HitArea::Corner;
    }
    if (horizontalRect(viewportRect).contains(viewPoint))
    {
        return HitArea::Horizontal;
    }
    if (verticalRect(viewportRect).contains(viewPoint))
    {
        return HitArea::Vertical;
    }
    return HitArea::None;
}

QString RulerRenderer::labelText(const qreal worldCoordinate, const qreal originCoordinate)
{
    return QString::number(worldCoordinate - originCoordinate, 'f', 0);
}

void RulerRenderer::draw(
    QPainter* const painter,
    const QRect& viewportRect,
    const QSizeF& viewportSize,
    const CanvasCamera& camera,
    const QPointF& worldOrigin,
    const QColor& baseColor,
    const QPalette& palette) const
{
    if (!painter || viewportRect.isEmpty() || viewportSize.isEmpty() || m_thickness <= 0)
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->fillRect(horizontalRect(viewportRect), backgroundColor(baseColor));
    const QRect verticalOnly = verticalRect(viewportRect).adjusted(
        0, cornerRect(viewportRect).height(), 0, 0);
    painter->fillRect(verticalOnly, backgroundColor(baseColor));

    QColor foreground = palette.color(QPalette::WindowText);
    foreground.setAlpha(235);
    QPen pen(foreground);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setFont(m_font);

    drawHorizontal(painter, horizontalRect(viewportRect), viewportSize, camera, worldOrigin.x());
    drawVertical(painter, verticalRect(viewportRect), viewportSize, camera, worldOrigin.y());
    painter->restore();
}

qreal RulerRenderer::tickStep(const qreal scale) const
{
    static constexpr qreal steps[] = {0.1, 0.5, 1.0, 5.0, 10.0, 50.0, 100.0, 500.0};
    for (const qreal step : steps)
    {
        if (step * scale >= 10.0)
        {
            return step;
        }
    }
    return steps[7];
}

void RulerRenderer::drawHorizontal(
    QPainter* const painter,
    const QRect& rect,
    const QSizeF& viewportSize,
    const CanvasCamera& camera,
    const qreal originX) const
{
    if (rect.isEmpty())
    {
        return;
    }

    const qreal start = camera.mapToWorld(QPointF(rect.left(), rect.top()), viewportSize).x();
    const qreal end = camera.mapToWorld(QPointF(rect.right() + 1.0, rect.top()), viewportSize).x();
    const qreal step = tickStep(camera.scale());
    qint64 index = qFloor((start + 1e-12) / step);

    for (qreal world = index * step; world <= end + step; world += step, ++index)
    {
        if (index % 10 != 0)
        {
            continue;
        }
        const qreal x = std::round(camera.mapFromWorld(QPointF(world, 0.0), viewportSize).x()) + 0.5;
        painter->drawLine(QPointF(x, rect.bottom() - rect.height() * 0.4), QPointF(x, rect.bottom()));

        const QString text = labelText(world, originX);
        const QFontMetrics metrics(painter->font());
        const int width = metrics.horizontalAdvance(text);
        if (10.0 * step * camera.scale() >= width)
        {
            painter->drawText(QRectF(x - width / 2.0,
                                     rect.top() + labelEdgePadding,
                                     width,
                                     metrics.height()),
                              Qt::AlignCenter, text);
        }
    }
}

void RulerRenderer::drawVertical(
    QPainter* const painter,
    const QRect& rect,
    const QSizeF& viewportSize,
    const CanvasCamera& camera,
    const qreal originY) const
{
    if (rect.isEmpty())
    {
        return;
    }

    const qreal start = camera.mapToWorld(QPointF(rect.left(), rect.top()), viewportSize).y();
    const qreal end = camera.mapToWorld(QPointF(rect.left(), rect.bottom() + 1.0), viewportSize).y();
    const qreal step = tickStep(camera.scale());
    qint64 index = qFloor((start + 1e-12) / step);

    for (qreal world = index * step; world <= end + step; world += step, ++index)
    {
        if (index % 10 != 0)
        {
            continue;
        }
        const qreal y = std::round(camera.mapFromWorld(QPointF(0.0, world), viewportSize).y()) + 0.5;
        painter->drawLine(QPointF(rect.right(), y), QPointF(rect.right() - rect.width() * 0.4, y));

        const QString text = labelText(world, originY);
        const QFontMetrics metrics(painter->font());
        if (10.0 * step * camera.scale() >= metrics.height())
        {
            const qreal labelCenterX = rect.left()
                + labelEdgePadding
                + metrics.height() / 2.0;
            painter->save();
            painter->translate(labelCenterX, y);
            painter->rotate(-90.0);
            painter->drawText(QRectF(-metrics.horizontalAdvance(text) / 2.0,
                                     -metrics.height() / 2.0,
                                     metrics.horizontalAdvance(text), metrics.height()),
                              Qt::AlignCenter, text);
            painter->restore();
        }
    }
}

}// namespace xcanvas
