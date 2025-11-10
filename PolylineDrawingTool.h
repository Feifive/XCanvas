#ifndef POLYLINEDRAWINGTOOL_H
#define POLYLINEDRAWINGTOOL_H

#include <QObject>
#include <QPointF>
#include <QVector>
#include <QtWidgets/qgraphicsitem.h>
#include "BaseDrawingTool.h"

class MyGraphicsView;
class PolylineShape;

class LineDrawingTool : public QObject, public BaseDrawingTool
{
    Q_OBJECT
public:
    explicit LineDrawingTool(MyGraphicsView* pView);
    virtual ~LineDrawingTool() override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual int  ToolType() override;

private:
    PolylineShape* m_pDrawingItem;
    QPointF m_startPos;
    bool m_bDrawing;
    QVector<QPointF> m_points;
};

#endif // POLYLINEDRAWINGTOOL_H
