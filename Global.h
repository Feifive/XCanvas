#ifndef GLOBAL_H
#define GLOBAL_H

#include <QPen>
#include <QDateTime>

#define qDebugTime() qDebug().noquote() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")

#define DRAWING_LINE_PEN          GetDrawingLinePen()
#define DRAWED_LINE_PEN(scale)    GetDrawedLinePen(scale)
#define SELECTED_LINE_PEN(scale)  GetSelectedLinePen(scale)
#define HIGHLIGHT_LINE_PEN(scale) GetHighlightPen(scale)

#define Z_VALUE_HIGHLIGHT 10000

enum class DrawingToolType : uint8_t
{
    None    = 0,
    Select  = 1,
    Rect    = 2,
    Line    = 3,
    Ellipse = 4
};

enum ERECT_POS
{
    ERECT_TOP_LEFT = 0,
    ERECT_TOP_MID,
    ERECT_TOP_RIGHT,
    ERECT_MID_LEFT,
    ERECT_CENTER,
    ERECT_MID_RIGHT,
    ERECT_BOTTOM_LEFT,
    ERECT_BOTTOM_MID,
    ERECT_BOTTOM_RIGHT,
    ERECT_POS_COUNT
};

inline QPen GetDrawingLinePen()
{
    QPen pen(Qt::black);
    pen.setCosmetic(true);
    pen.setStyle(Qt::DotLine);
    return pen;
}

inline QPen GetDrawedLinePen(qreal scale)
{
    QPen pen(Qt::black);
    pen.setWidthF(1.0 / scale);
    pen.setStyle(Qt::SolidLine);
    return pen;
}

inline QPen GetSelectedLinePen(qreal scale)
{
    QPen pen(QColor(244,155,33));
    pen.setWidthF(1.0 / scale);
    pen.setStyle(Qt::DashLine);
    return pen;
}

inline QPen GetHighlightPen(qreal scale)
{
    QPen pen(QColor(244,155,33));
    pen.setWidthF(1.0 / scale);
    pen.setStyle(Qt::SolidLine);
    return pen;
}


#endif // GLOBAL_H
