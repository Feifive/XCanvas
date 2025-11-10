#ifndef RECTSHAPE_H
#define RECTSHAPE_H

#include "BaseShape.h"

class RectShape : public BaseShape
{
public:
    RectShape();
    virtual ~RectShape() override;

    QRectF       boundingRect() const  override;
    QPainterPath shape() const         override;

    void SetRect(const QRectF& rect);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void UpdatePath();

private:
    QRectF m_rect;
};

#endif // RECTSHAPE_H
