#ifndef RECTDRAWINGTOOL_H
#define RECTDRAWINGTOOL_H

#include "DrawingTool.h"

namespace xcanvas
{
class RectTool : public DrawingTool
{
    Q_OBJECT
  public:
    explicit RectTool(ICanvasViewport* view, Canvas* canvas);
    ~RectTool() override;

    virtual void            mousePressEvent(QMouseEvent* event) override;
    virtual void            mouseMoveEvent(QMouseEvent* event) override;
    virtual void            mouseReleaseEvent(QMouseEvent* event) override;
    virtual DrawingToolType toolType() override;
};
}// namespace xcanvas

#endif// RECTDRAWINGTOOL_H
