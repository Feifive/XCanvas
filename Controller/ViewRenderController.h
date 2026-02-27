#ifndef VIEWRENDERCONTROLLER_H
#define VIEWRENDERCONTROLLER_H

#include <QRectF>

class QGraphicsView;
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
    ViewRenderController(QGraphicsView* view, xcanvas::Canvas* canvas, QSvgRenderer* rotateHandle);

    void setRotateHandle(QSvgRenderer* rotateHandle);
    void setSuppressedShape(const xcanvas::Shape* shape);
    void setSelectionHandlesVisible(bool visible);

    void drawBackground(QPainter* painter, const QRectF& rect);
    void drawForeground(QPainter* painter, const QRectF& rect);
    void drawShapes(QPainter* painter, const QRectF& visibleRect);
    void drawNormalShapes(QPainter* painter, const QRectF& visibleRect);
    void drawSelectedShapes(QPainter* painter, const QRectF& visibleRect);
    void drawGrid(QPainter* painter);
    void drawTrace(QPainter* painter);
    void drawCanvas(QPainter* painter);
    double gridStep(double scale) const;

  private:
    QGraphicsView*       m_view;
    xcanvas::Canvas*     m_canvas;
    QSvgRenderer*        m_rotateHandle;
    const xcanvas::Shape* m_suppressedShape = nullptr;
    bool m_selectionHandlesVisible = true;
};

#endif// VIEWRENDERCONTROLLER_H
