#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include "Global.h"
#include <QGraphicsView>

class QGraphicsRectItem;
class BottomFloatingToolBar;

namespace xcanvas
{
	class Shape;
    class Shapes;
    class DrawingTool;
}

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
  public:
    explicit MyGraphicsView(QWidget* parent = nullptr);
    ~MyGraphicsView();

    void    setTool(DrawingToolType type);
    xcanvas::Shapes* GetCurrentShapes();
    double  scale();
    void    updateCanvas();
    void    updateShape(xcanvas::Shape* shape);
    void    updateSelectedShapes();
    void    traceRects(const QRectF& rect, QRectF rects[9]);

  signals:
    void mouseMovePos(QPointF pos);

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private:
    void drawShapes(QPainter* painter, const QRectF& visibleRect);
    void drawNormalShapes(QPainter* painter, const QRectF& visibleRect);
    void drawSelectedShapes(QPainter* painter, const QRectF& visibleRect);
    void drawGrid(QPainter* painter);
    void drawTrace(QPainter* painter);
    void drawCanvas(QPainter* painter);


  private:
    double gridStep(double scale) const;
    void ImportFile();
    void updateBottomFloatingToolBarPos();
    void zoomIn();
    void zoomOut();
    void zoomTo(qreal zoomValue);
    void fitWidth();
    void fitHeight();
    void fitCanvas();
    void fitShapes();

  private:
    QPointF          m_startPos;
    bool             m_bDragging;
    double           m_dScaleFactor;
    QRectF m_CanvasRect;
    DrawingToolType  m_eToolType;
    xcanvas::DrawingTool*     m_pBaseDrawingTool;
    xcanvas::Shapes* m_pShapes;
    BottomFloatingToolBar* m_pFloatingToolBar;
};

#endif// MYGRAPHICSVIEW_H
