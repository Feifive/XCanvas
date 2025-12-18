#ifndef CURVETOOL_H
#define CURVETOOL_H

#include "DrawingTool.h"

class MyGraphicsView;

namespace xcanvas
{
class CurveTool : public DrawingTool
{
    Q_OBJECT
  public:
    explicit CurveTool(MyGraphicsView* view, Canvas* canvas);
    ~CurveTool() override;

    void            mousePressEvent(QMouseEvent* event) override;
    void            mouseMoveEvent(QMouseEvent* event) override;
    void            mouseReleaseEvent(QMouseEvent* event) override;
    DrawingToolType toolType() override;

  protected:
    void cancelDrawing() override;

  private:
    QVector<QPointF> computeBezierPoints(const QVector<QPointF>& anchorPoints) const;
    QPainterPath     buildCurvePath(const QVector<QPointF>& bezierPoints) const;
    void             rebuildPreviewPath();

  private:
    QVector<QPointF> m_points;
};

}// namespace xcanvas

#endif//CURVETOOL_H
