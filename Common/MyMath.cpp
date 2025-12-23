#include "MyMath.h"
#include <QtMath>

namespace xcanvas::geometryMath
{

TraceHandles traceRects(const QRectF &rect, double zoomValue) {
    TraceHandles handles;
    handles.resizeRects.reserve(9);

    const double dRectSize  = 4 / zoomValue;
    const double dRectWidth = dRectSize * 2;
    const double offset     = 5 / zoomValue;
    const QRectF outRect    = rect.adjusted(-offset, -offset, offset, offset);

    handles.resizeRects.append(QRectF(outRect.left() - dRectWidth, outRect.top() - dRectWidth, dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.center().x() - dRectSize, outRect.top() - dRectWidth, dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.right(), outRect.top() - dRectWidth, dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.left() - dRectWidth, outRect.center().y() - dRectSize, dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.center().x() - dRectSize, outRect.center().y() - dRectSize, dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.right(), outRect.center().y() - dRectSize, dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.left() - dRectWidth, outRect.bottom(), dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.center().x() - dRectSize, outRect.bottom(), dRectWidth, dRectWidth));
    handles.resizeRects.append(QRectF(outRect.right(), outRect.bottom(), dRectWidth, dRectWidth));

    const double rotateSize    = 16.0 / zoomValue;
    const double rotateGap     = 28.0 / zoomValue;
    const double rotateOffsetX = -1.0 / zoomValue;
    const QPointF rotateCenter(rect.center().x() + rotateOffsetX, rect.top() - rotateGap);
    handles.rotateRect = QRectF(rotateCenter.x() - rotateSize * 0.5,rotateCenter.y() - rotateSize * 0.5,rotateSize,rotateSize);

    return handles;
}

double point2Segment(const QPointF& A, const QPointF& B, const QPointF& P)
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

QVector<QPointF> buildRegularPolygon(const QRectF& rect, int sides, double startAngleDeg)
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

QVector<Segment> buildPolylineSegments(const QVector<QPointF>& points)
{
    QVector<Segment> segments;
    if (points.size() < 2) {
        return segments;
    }

    segments.reserve(points.size());
    segments.append(Segment::moveTo(points[0]));
    for (int i = 1; i < points.size(); ++i) {
        segments.append(Segment::lineTo(points[i]));
    }

    return segments;
}

QVector<Segment> buildCurveSegments(const QVector<QPointF> &points) {
    QVector<Segment> segments;
    if (points.size() < 4) {
        return segments;
    }

    segments.reserve(points.size());

    segments.append(Segment::moveTo(points[0]));

    for (int i = 0; i + 3 < points.size(); i += 3)
    {
        const QPointF& c1  = points[i + 1];
        const QPointF& c2  = points[i + 2];
        const QPointF& end = points[i + 3];

        segments.append(Segment::cubicTo(c1, c2, end));
    }

    return segments;
}

QVector<xcanvas::Segment> buildEllipseSegments(const QRectF& rect)
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

}// namespace xcanvas::geometryMath
