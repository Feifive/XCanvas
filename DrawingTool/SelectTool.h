#ifndef SELECTDRAWINGTOOL_H
#define SELECTDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include "DrawingTool.h"

class QGraphicsRectItem;
class QGraphicsPathItem;

namespace xcanvas
{
    class Shape;
}

namespace xcanvas
{
    class SelectTool : public DrawingTool
    {
        Q_OBJECT
    public:
        explicit SelectTool(MyGraphicsView* pView);
        virtual ~SelectTool() override;

        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;
        virtual void keyPressEvent(QKeyEvent* event)override;
        virtual void drawPreview(QPainter* painter) override;
        virtual DrawingToolType toolType() override;

    private:
        int hitTraceHandle(const QPointF& pos) const;
        void setCanvasCursorShape(int nHitPos);
        Shape* hitUnselectedShape(QPointF pos);
        void updateHighlight(const Shape& shape);
        void clearHighlight();
        void updateSelectionRect(const QRectF& rect);
        void clearSelectionRect();

    private:
        bool m_bMovingItem;
        QPainterPath m_highlightPath;
        QPainterPath m_selectionRectPath;
    };
}


#endif // SELECTDRAWINGTOOL_H
