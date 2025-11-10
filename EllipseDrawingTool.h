#ifndef ELLIPSEDRAWINGTOOL_H
#define ELLIPSEDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include "BaseDrawingTool.h"

class MyGraphicsView;
class QGraphicsEllipseItem;

class EllipseDrawingTool : public QObject, public BaseDrawingTool
{
    Q_OBJECT
public:
    explicit EllipseDrawingTool(MyGraphicsView* pView);
    virtual ~EllipseDrawingTool() override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual int  ToolType() override;

private:
    QGraphicsEllipseItem* m_pDrawingItem;
    bool                  m_bDrawing;
    QPointF               m_startPos;
};

#endif // ELLIPSEDRAWINGTOOL_H
