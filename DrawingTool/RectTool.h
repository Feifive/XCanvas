#ifndef RECTDRAWINGTOOL_H
#define RECTDRAWINGTOOL_H

#include "DrawingTool.h"
#include <QObject>
#include <QPointF>
#include <QPainterPath>

class QGraphicsRectItem;

namespace xcanvas
{
    class RectTool : public DrawingTool
    {
        Q_OBJECT
    public:
        explicit RectTool(MyGraphicsView* pView);
        ~RectTool() override;

        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;
        virtual DrawingToolType toolType() override;
    };
}


#endif// RECTDRAWINGTOOL_H
