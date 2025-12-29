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
    Text,
    Image
};

class Shape
{
  public:
    virtual ~Shape();

    virtual void draw(QPainter* painter) const;
    virtual bool isSelected() const;
    virtual void   setColor(const QColor& color);
    virtual QColor color() const;

    void setTransform(const QTransform& transform);
    QTransform transform() const { return m_transform; }
    void resetTransform() { setTransform(QTransform()); }
    virtual void translate(const QPointF& offset);
    virtual void rotate(double angle, const QPointF& center);
    virtual void scale(double sx, double sy, const QPointF& anchor);

    virtual QPainterPath& path() const;
    virtual QRectF        boundingRect() const;

    virtual ShapeType type() const                     = 0;
    virtual bool hitTest(const QPointF& point, double  tolerance) const = 0;

    bool isDirty() const;
    void setDirty(const bool dirty) const;

  protected:
    virtual void updatePainterPath() = 0;
    void         markDirty() const;
    virtual void setSelected(bool selected);
    bool isPointNearPath(const QPointF& point, double tolerance) const;

  protected:
    bool                 m_selected          = false;
    mutable bool         m_dirty             = true;
    mutable bool         m_boundingRectDirty = true;
    mutable QRectF       m_cachedBoundingRect;
    mutable QPainterPath m_path;
    mutable QPainterPath m_originalPath;
    QColor               m_color;
    double               m_rotation = 0.0;
    QTransform           m_transform;

  friend class ShapeManager;
};

}// namespace xcanvas

#endif// BASESHAPE_H
