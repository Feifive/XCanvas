#ifndef MYMATH_H
#define MYMATH_H

#include "GeometryDefs.h"
#include <QVector>
#include <QRectF>

constexpr double KAPPA = 0.5522847498307936;
constexpr double TOLERANCE_DUPLICATE = 0.005;// 去重阈值
constexpr double TOLERANCE_RDP       = 0.01;//  RDP简化阈值

struct TraceHandles
{
    QVector<QRectF> resizeRects;
    QRectF          rotateRect;
};

namespace xcanvas::geometryMath {
    TraceHandles     traceRects(const QRectF& rect, double zoomValue);
    double           point2Segment(const QPointF& A, const QPointF& B, const QPointF& P);
    QVector<QPointF> buildRegularPolygon(const QRectF& rect, int sides, double startAngleDeg = -90.0);
    QVector<Segment> buildEllipseSegments(const QRectF& rect);
    QVector<Segment> buildPolylineSegments(const QVector<QPointF>& points);
    QVector<Segment> buildCurveSegments(const QVector<QPointF>& points);
    QVector<QPointF> Optimize(const QVector<QPointF>& inputPoints, bool closed); // 去除冗余点
    QVector<Segment> Optimize(const QVector<Segment>& segments); // 去除冗余线段，支持直线段
}

#endif
