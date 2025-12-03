#ifndef ELLIPSEDRAWINGTOOL_H
#define ELLIPSEDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include "DrawingTool.h"

class QGraphicsEllipseItem;

namespace xcanvas
{
    class EllipseTool : public DrawingTool
    {
        Q_OBJECT
    public:
        explicit EllipseTool(MyGraphicsView* pView);
        virtual ~EllipseTool() override;

        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;
        virtual DrawingToolType  toolType() override;
    };
}


#endif // ELLIPSEDRAWINGTOOL_H
