#ifndef MYMATH_H
#define MYMATH_H

#include <QtMath>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <algorithm>

class MyMath
{
public:
    static double point2Segment(const QPointF& A, const QPointF& B, const QPointF& P);
    static QVector<QPointF> buildRegularPolygon(const QRectF& rect, int sides, double startAngleDeg = -90.0);
};

#endif