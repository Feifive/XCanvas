#ifndef GLOBAL_H
#define GLOBAL_H

#include <QDateTime>
#include <QFontDatabase>
#include <QPen>

#define qDebugTime() qDebug().noquote() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")

#define Z_VALUE_HIGHLIGHT 99999

#define MIN_ZOOM 0.05
#define MAX_ZOOM 100.0

const double BOUNDING_BOX_TOLERANCE = 0.001;

enum class DrawingToolType : uint8_t
{
    None = 0,
    Select,
    Polyline,
    Curve,
    Rect,
    Ellipse,
    Text,
    Polygon
};

enum HandleType
{
    TopLeft = 0,
    TopMid,
    TopRight,
    MidLeft,
    Center,
    MidRight,
    BottomLeft,
    BottomMid,
    BottomRight,
    Rotate
};

inline QPen dotLinePen()
{
    QPen pen(Qt::black);
    pen.setWidth(1);
    pen.setCosmetic(true);
    pen.setStyle(Qt::DotLine);
    return pen;
}

inline QPen normalPen(QColor color = QColor(Qt::black))
{
    QPen pen(color);
    pen.setWidth(1);
    pen.setCosmetic(true);
    pen.setStyle(Qt::SolidLine);
    return pen;
}

inline QPen selectedPen()
{
    QPen pen(QColor(244, 155, 33));
    pen.setWidth(1);
    pen.setCosmetic(true);
    pen.setStyle(Qt::SolidLine);
    return pen;
}

inline QPen highlightPen()
{
    QPen pen(QColor(244, 155, 33));
    pen.setWidth(1);
    pen.setCosmetic(true);
    pen.setStyle(Qt::SolidLine);
    return pen;
}

inline bool IsEqual(double a, double b, double eps = 1e-6)
{
    return std::fabs(a - b) <= eps;
}

inline bool IsEqual(const QPointF& a, const QPointF& b, double eps = 1e-6)
{
    return (std::fabs(a.x() - b.x()) < eps) && (std::fabs(a.y() - b.y()) < eps);
}

#endif// GLOBAL_H
