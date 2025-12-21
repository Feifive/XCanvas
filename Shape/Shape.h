#ifndef BASESHAPE_H
#define BASESHAPE_H

#include <QColor>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>

class QPainter;

namespace xcanvas
{

enum class ShapeType
{
    Vector,
    Polyline,
    Ellipse,
    Text,
    Curve,
    Image
};

class Shape
{
  public:
    virtual ~Shape();

    virtual void draw(QPainter* painter) const;

    virtual void setSelected(bool selected);
    virtual bool isSelected() const
    {
        return m_selected;
    }

    virtual void   setColor(const QColor& color);
    virtual QColor color() const
    {
        return m_color;
    }

    virtual QPainterPath& path() const;
    virtual QRectF        boundingRect() const;

    virtual bool      isPointNearPath(const QPointF& point, double dScale);
    virtual void      translate(const QPointF& offset) = 0;
    virtual ShapeType type() const                     = 0;

    bool isDirty() const
    {
        return m_dirty;
    }
    void setDirty(bool dirty)
    {
        m_dirty = dirty;
    }

  protected:
    virtual void updatePainterPath() = 0;
    void         markDirty();

  protected:
    bool                 m_selected          = false;
    mutable bool         m_dirty             = true;
    mutable bool         m_boundingRectDirty = true;
    mutable QRectF       m_cachedBoundingRect;
    mutable QPainterPath m_path;
    QColor               m_color;
};

}// namespace xcanvas

#endif// BASESHAPE_H
