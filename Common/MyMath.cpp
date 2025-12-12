#include "MyMath.h"

double MyMath::point2Segment(const QPointF& A, const QPointF& B, const QPointF& P)
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

QVector<QPointF> MyMath::buildRegularPolygon(const QRectF& rect, int sides, double startAngleDeg)
{
    QVector<QPointF> points;
    if (sides < 3)
    {
        return points;
    }

    QPointF center = rect.center();
    double rx      = rect.width() / 2.0;
    double ry      = rect.height() / 2.0;

    const double step = 360.0 / sides;

    for (int i = 0; i < sides; ++i)
    {
        double angle = (startAngleDeg + i * step) * M_PI / 180.0;
        double x = center.x() + rx * std::cos(angle);
        double y = center.y() + ry * std::sin(angle);
        points.append(QPointF(x, y));
    }

    return points;
}
