#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include "Global.h"
#include "Shapes.h"

class QGraphicsScene;
class QGraphicsRectItem;
class BaseDrawingTool;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget *parent = nullptr);
    ~MyGraphicsView();

    void    SetTool(DrawingToolType type);
    Shapes* GetCurrentShapes();
    double  GetScaleFactory();
    void    UpdateCanvas();

signals:
    void mouseMovePos(QPointF pos);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    QGraphicsScene*  m_pScene;
    QPointF          m_startPos;
    bool             m_bDragging;
    double           m_dScaleFactor;
    DrawingToolType  m_eToolType;
    BaseDrawingTool* m_pBaseDrawingTool;
    Shapes*          m_pShapes;
    bool             m_bInitPosition;
};

#endif // MYGRAPHICSVIEW_H
