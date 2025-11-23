#ifndef POLYLINEDRAWINGTOOL_H
#define POLYLINEDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include <QVector>
#include <QtWidgets/qgraphicsitem.h>
#include "BaseDrawingTool.h"

class MyGraphicsView;
class PolylineShape;

class PolylineDrawingTool : public BaseDrawingTool
{
    Q_OBJECT
public:
    explicit PolylineDrawingTool(MyGraphicsView* pView);
    virtual ~PolylineDrawingTool() override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual int  ToolType() override;

protected:
    void CancelDrawing() override;

private:
    PolylineShape* m_pDrawingItem;
    QPointF m_startPos;
    QVector<QPointF> m_points;
};

#endif // POLYLINEDRAWINGTOOL_H
