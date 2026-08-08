#include "SafeDropShadowEffect.h"

SafeDropShadowEffect::SafeDropShadowEffect(QObject* parent) : QGraphicsDropShadowEffect(parent)
{
}

QRectF SafeDropShadowEffect::boundingRectFor(const QRectF& sourceRect) const
{
    QRectF      rect = QGraphicsDropShadowEffect::boundingRectFor(sourceRect);
    const qreal blur = blurRadius();
    rect.adjust(+blur, +blur, -blur, -blur);
    return rect;
}
