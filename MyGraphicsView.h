#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include "Global.h"
#include <QGraphicsView>

class QGraphicsRectItem;

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
    void transformChanged();

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

protected:
    // 重写绘制方法
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private:
    void drawShapes(QPainter* painter, const QRectF& visibleRect);
    void drawNormalShapes(QPainter* painter, const QRectF& visibleRect);
    void drawSelectedShapes(QPainter* painter, const QRectF& visibleRect);
    void drawGrid(QPainter* painter);
    double gridStep(double scale) const;
    void drawTrace(QPainter* painter);

  private:
    void ImportFile();


  private:
    QPointF          m_startPos;
    bool             m_bDragging;
    double           m_dScaleFactor;
    DrawingToolType  m_eToolType;
    xcanvas::DrawingTool*     m_pBaseDrawingTool;
    xcanvas::Shapes* m_pShapes;
};

#endif// MYGRAPHICSVIEW_H
