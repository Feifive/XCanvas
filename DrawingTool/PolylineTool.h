#ifndef POLYLINEDRAWINGTOOL_H
#define POLYLINEDRAWINGTOOL_H

#include "DrawingTool.h"
#include <QPointF>
#include <QVector>

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
    explicit PolylineTool(MyGraphicsView* view, Canvas* canvas);
    virtual ~PolylineTool() override;

    virtual void            mousePressEvent(QMouseEvent* event) override;
    virtual void            mouseMoveEvent(QMouseEvent* event) override;
    virtual void            mouseReleaseEvent(QMouseEvent* event) override;
    virtual DrawingToolType toolType() override;

  protected:
    void cancelDrawing() override;

  private:
    QVector<QPointF> m_points;
};
}// namespace xcanvas

#endif// POLYLINEDRAWINGTOOL_H
