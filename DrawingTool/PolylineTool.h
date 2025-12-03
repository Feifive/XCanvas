#ifndef POLYLINEDRAWINGTOOL_H
#define POLYLINEDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include <QVector>
#include <QtWidgets/qgraphicsitem.h>
#include "DrawingTool.h"

namespace xcanvas
{
    class Polyline;
}

namespace xcanvas
{
    class PolylineTool : public DrawingTool
    {
        Q_OBJECT
    public:
        explicit PolylineTool(MyGraphicsView* pView);
        virtual ~PolylineTool() override;

        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;
        virtual DrawingToolType toolType() override;

    protected:
        void cancelDrawing() override;

    private:
        QVector<QPointF> m_points;
    };
}


#endif // POLYLINEDRAWINGTOOL_H
