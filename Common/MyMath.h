#ifndef MYMATH_H
#define MYMATH_H

#include "Vector.h"
#include <QVector>

constexpr double KAPPA = 0.5522847498307936;

namespace xcanvas
{

class MyMath
{
  public:
    static double           point2Segment(const QPointF& A, const QPointF& B, const QPointF& P);
    static QVector<QPointF> buildRegularPolygon(const QRectF& rect, int sides, double startAngleDeg = -90.0);
    static QVector<Segment> buildEllipseSegments(const QRectF& rect);
    static QVector<Segment> buildPolylineSegments(const QVector<QPointF>& points);
};

}// namespace xcanvas

#endif
