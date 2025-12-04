#ifndef CURVE_H
#define CURVE_H

#include "Shape.h"
#include <QPointF>
#include <QVector>

namespace xcanvas
{


class Curve : public Shape
{
public:
    Curve();
    ~Curve();

    void SetPoints(QVector<QPointF> points);
    void translate(const QPointF& offset) override;

protected:
    void updatePainterPath() override;

private:
    QVector<QPointF> m_points;
};



} // xcanvas

#endif // CURVE_H
