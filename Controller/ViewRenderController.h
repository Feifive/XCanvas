#ifndef VIEWRENDERCONTROLLER_H
#define VIEWRENDERCONTROLLER_H

#include <QRectF>

class ICanvasViewport;
class QColor;
class QPainter;
class QSvgRenderer;

namespace xcanvas
{
class Canvas;
class Shape;
}

class ViewRenderController final
{
  public:
    ViewRenderController(ICanvasViewport* view, xcanvas::Canvas* canvas, QSvgRenderer* rotateHandle);

    void setRotateHandle(QSvgRenderer* rotateHandle);
    void setSuppressedShape(const xcanvas::Shape* shape);
    void setSelectionHandlesVisible(bool visible);
    void setSelectionDashPhase(qreal phase);

    void drawBackground(QPainter* painter, const QRectF& rect);
    void drawForeground(QPainter* painter, const QRectF& rect);
    void drawShapes(QPainter* painter, const QRectF& visibleRect);
    void drawNormalShapes(QPainter* painter, const QRectF& visibleRect);
    void drawSelectedShapes(QPainter* painter, const QRectF& visibleRect);
    void drawGrid(QPainter* painter);
    void drawTrace(QPainter* painter);
    void drawCanvas(QPainter* painter);
    double gridStep(double scale) const;
    static QColor workspaceBackgroundColor();

  private:
    ICanvasViewport*     m_view;
    xcanvas::Canvas*     m_canvas;
    QSvgRenderer*        m_rotateHandle;
    const xcanvas::Shape* m_suppressedShape = nullptr;
    bool m_selectionHandlesVisible = true;
    qreal m_selectionDashPhase = 0.0;
};

#endif// VIEWRENDERCONTROLLER_H
