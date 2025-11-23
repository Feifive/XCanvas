#ifndef MYMATH_H
#define MYMATH_H

#include <QtMath>
#include <QPointF>
#include <algorithm>

namespace MyMath
{

// 返回点到线段的距离，参数：线段 AB、点 P
double Point2Segment(const QPointF& A, const QPointF& B, const QPointF& P)
{
    const double vx = B.x() - A.x();
    const double vy = B.y() - A.y();
    const double wx = P.x() - A.x();
    const double wy = P.y() - A.y();

    const double vv = vx * vx + vy * vy;
    if (vv < 1e-12)
        return std::hypot(P.x() - A.x(), P.y() - A.y()); // 退化为点

    double t = (wx * vx + wy * vy) / vv;
    t = std::clamp(t, 0.0, 1.0); // 投影钳制到线段上

    const double projx = A.x() + t * vx;
    const double projy = A.y() + t * vy;

    return std::hypot(P.x() - projx, P.y() - projy);
}

} // namespace CGMMath

#endif // MYMATH_H
