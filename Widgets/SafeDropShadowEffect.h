#ifndef SAFEDROPSHADOWEFFECT_H
#define SAFEDROPSHADOWEFFECT_H

#include <QGraphicsDropShadowEffect>

class SafeDropShadowEffect : public QGraphicsDropShadowEffect
{
  public:
    explicit SafeDropShadowEffect(QObject* parent = nullptr);

  protected:
    QRectF boundingRectFor(const QRectF& sourceRect) const override;
};

#endif// SAFEDROPSHADOWEFFECT_H
