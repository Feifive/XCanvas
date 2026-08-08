#ifndef VIEWPORTTRANSFORMCONTROLLER_H
#define VIEWPORTTRANSFORMCONTROLLER_H

#include <QPointF>

class BottomFloatingToolBar;
class ICanvasViewport;
class ICanvasNavigation;

namespace xcanvas
{
class Canvas;
}

class ViewportTransformController final
{
  public:
    ViewportTransformController(
        ICanvasViewport*       view,
        ICanvasNavigation*     navigation,
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
    ICanvasViewport*       m_view;
    ICanvasNavigation*     m_navigation;
    xcanvas::Canvas*       m_canvas;
    qreal                  m_scaleFactor;
};

#endif// VIEWPORTTRANSFORMCONTROLLER_H
