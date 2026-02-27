#ifndef VIEWPORTTRANSFORMCONTROLLER_H
#define VIEWPORTTRANSFORMCONTROLLER_H

#include <QPointF>

class BottomFloatingToolBar;
class QGraphicsView;

namespace xcanvas
{
class Canvas;
}

class ViewportTransformController final
{
  public:
    ViewportTransformController(
        QGraphicsView*         view,
        xcanvas::Canvas*       canvas);

    qreal scaleFactor() const;

    bool zoomIn(const QPointF& zoomCenterPoint);
    bool zoomOut(const QPointF& zoomCenterPoint);
    bool zoomTo(qreal zoomValue);
    bool fitWidth();
    bool fitHeight();
    bool fitCanvas();
    bool fitShapes();

  private:
    QGraphicsView*         m_view;
    xcanvas::Canvas*       m_canvas;
    qreal                  m_scaleFactor;
};

#endif// VIEWPORTTRANSFORMCONTROLLER_H
