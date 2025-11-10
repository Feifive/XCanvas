#ifndef BASESHAPE_H
#define BASESHAPE_H

#include <QRectF>
#include <QGraphicsItem>

class QPainterPath;

class BaseShape : public QGraphicsItem
{
public:
    virtual ~BaseShape() = default;
    virtual void Select(bool selected);
    virtual bool IsSelected() const;
    virtual bool IsPointNearPath(const QPointF& point, double dScale);

protected:
    virtual void UpdatePath() = 0;

protected:
    bool m_bSelected      = false;
    QPainterPath* m_pPath = nullptr;
};
#endif // BASESHAPE_H
