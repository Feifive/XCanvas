#include "MyMath.h"
#include <QtMath>

namespace xcanvas
{

double MyMath::point2Segment(const QPointF& A, const QPointF& B, const QPointF& P)
{
    const double vx = B.x() - A.x();
    const double vy = B.y() - A.y();
    const double wx = P.x() - A.x();
    const double wy = P.y() - A.y();

    const double vv = vx * vx + vy * vy;
    if (vv < 1e-12)
        return std::hypot(P.x() - A.x(), P.y() - A.y());// 退化为点

    double t = (wx * vx + wy * vy) / vv;
    t        = std::clamp(t, 0.0, 1.0);// 投影钳制到线段上

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
    double  rx     = rect.width() / 2.0;
    double  ry     = rect.height() / 2.0;

    const double step = 360.0 / sides;

    for (int i = 0; i < sides; ++i)
    {
        double angle = (startAngleDeg + i * step) * M_PI / 180.0;
        double x     = center.x() + rx * std::cos(angle);
        double y     = center.y() + ry * std::sin(angle);
        points.append(QPointF(x, y));
    }

    return points;
}

QVector<Segment> MyMath::buildPolylineSegments(const QVector<QPointF>& points)
{
    QVector<Segment> dd;
    return dd;
}

QVector<xcanvas::Segment> MyMath::buildEllipseSegments(const QRectF& rect)
{
    QVector<Segment> segments;

    const QPointF c  = rect.center();
    const double  rx = rect.width() * 0.5;
    const double  ry = rect.height() * 0.5;

    const double ox = rx * KAPPA;
    const double oy = ry * KAPPA;

    // 四个端点（顺时针）
    QPointF p0(c.x() + rx, c.y());// 右
    QPointF p1(c.x(), c.y() + ry);// 下
    QPointF p2(c.x() - rx, c.y());// 左
    QPointF p3(c.x(), c.y() - ry);// 上

    segments.reserve(5);

    // 起点
    segments.append(Segment::moveTo(p0));

    // 右 → 下
    segments.append(Segment::cubicTo(QPointF(p0.x(), p0.y() + oy), QPointF(p1.x() + ox, p1.y()), p1));

    // 下 → 左
    segments.append(Segment::cubicTo(QPointF(p1.x() - ox, p1.y()), QPointF(p2.x(), p2.y() + oy), p2));

    // 左 → 上
    segments.append(Segment::cubicTo(QPointF(p2.x(), p2.y() - oy), QPointF(p3.x() - ox, p3.y()), p3));

    // 上 → 右
    segments.append(Segment::cubicTo(QPointF(p3.x() + ox, p3.y()), QPointF(p0.x(), p0.y() - oy), p0));

    return segments;
}

}// namespace xcanvas
