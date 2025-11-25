#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include "Global.h"
#include "Shape/Shapes.h"
#include <QGraphicsView>

class QGraphicsRectItem;
class BaseDrawingTool;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
  public:
    explicit MyGraphicsView(QWidget* parent = nullptr);
    ~MyGraphicsView();

    void    SetTool(DrawingToolType type);
    Shapes* GetCurrentShapes();
    double  GetScaleFactory();
    void    UpdateCanvas();

  signals:
    void mouseMovePos(QPointF pos);
    void ToolFinished();

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private:
    void OnToolFinished();

  private:
    QPointF          m_startPos;
    bool             m_bDragging;
    double           m_dScaleFactor;
    DrawingToolType  m_eToolType;
    BaseDrawingTool* m_pBaseDrawingTool;
    Shapes*          m_pShapes;
};

#endif// MYGRAPHICSVIEW_H
