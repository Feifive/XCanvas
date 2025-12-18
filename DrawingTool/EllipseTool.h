#ifndef ELLIPSEDRAWINGTOOL_H
#define ELLIPSEDRAWINGTOOL_H

#include "DrawingTool.h"
#include <QObject>
#include <QPointF>

class QGraphicsEllipseItem;

namespace xcanvas
{
class EllipseTool : public DrawingTool
{
    Q_OBJECT
  public:
    explicit EllipseTool(MyGraphicsView* view, Canvas* canvas);
    virtual ~EllipseTool() override;

    virtual void            mousePressEvent(QMouseEvent* event) override;
    virtual void            mouseMoveEvent(QMouseEvent* event) override;
    virtual void            mouseReleaseEvent(QMouseEvent* event) override;
    virtual DrawingToolType toolType() override;
};
}// namespace xcanvas

#endif// ELLIPSEDRAWINGTOOL_H
