#ifndef RECTSHAPE_H
#define RECTSHAPE_H

#include "BaseShape.h"

class RectShape : public Shape
{
public:
    RectShape();
    virtual ~RectShape() override;

    QRectF       boundingRect() const  override;
    QPainterPath shape() const         override;

    void SetRect(const QRectF& rect);

    void Offset(const QPointF& offset) override;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void UpdatePath() override;

private:
    QRectF m_rect;
};

#endif // RECTSHAPE_H
