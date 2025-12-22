#ifndef ELLIPSEDRAWINGTOOL_H
#define ELLIPSEDRAWINGTOOL_H

#include "DrawingTool.h"
#include "ShapeVector.h"

class QGraphicsEllipseItem;

namespace xcanvas
{
class EllipseTool : public DrawingTool
{
    Q_OBJECT
  public:
    explicit EllipseTool(MyGraphicsView* view, Canvas* canvas);
    virtual ~EllipseTool() override;

    void            mousePressEvent(QMouseEvent* event) override;
    void            mouseMoveEvent(QMouseEvent* event) override;
    void            mouseReleaseEvent(QMouseEvent* event) override;
    DrawingToolType toolType() override;

};
}// namespace xcanvas

#endif// ELLIPSEDRAWINGTOOL_H
