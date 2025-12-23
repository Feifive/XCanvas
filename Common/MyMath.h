#ifndef MYMATH_H
#define MYMATH_H

#include "GeometryDefs.h"
#include <QVector>
#include <QRectF>

constexpr double KAPPA = 0.5522847498307936;
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
}

#endif
