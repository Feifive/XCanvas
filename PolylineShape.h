#ifndef POLYLINESHAPE_H
#define POLYLINESHAPE_H

#include "BaseShape.h"

class PolylineShape : public BaseShape
{
public:
    PolylineShape();
    virtual ~PolylineShape() override;

    QPainterPath shape() const         override;
    QRectF       boundingRect() const  override;
    void SetPoints(QVector<QPointF> points);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void UpdatePath();

private:
    QVector<QPointF> m_points;
};

#endif // POLYLINESHAPE_H
