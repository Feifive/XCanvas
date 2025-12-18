#ifndef POLYGONTOOL_H
#define POLYGONTOOL_H

#include "DrawingTool.h"

namespace xcanvas
{

class PolygonTool : public DrawingTool
{
    Q_OBJECT
  public:
    explicit PolygonTool(MyGraphicsView* view, Canvas* canvas);
    ~PolygonTool() override;

    virtual void            mousePressEvent(QMouseEvent* event) override;
    virtual void            mouseMoveEvent(QMouseEvent* event) override;
    virtual void            mouseReleaseEvent(QMouseEvent* event) override;
    virtual DrawingToolType toolType() override;
};

}// namespace xcanvas

#endif// !POLYGONTOOL_H
