#ifndef BASESHAPE_H
#define BASESHAPE_H

#include <QGraphicsItem>
#include <QRectF>

class QPainterPath;

class Shape : public QGraphicsItem
{
  public:
    virtual ~Shape() = default;
    virtual void Select(bool selected);
    virtual bool IsSelected() const;
    virtual bool IsPointNearPath(const QPointF& point, double dScale);
    virtual void Offset(const QPointF& offset) = 0;

  protected:
    virtual void UpdatePath() = 0;

  protected:
    bool          m_bSelected = false;
    QPainterPath* m_pPath     = nullptr;
};
#endif// BASESHAPE_H
