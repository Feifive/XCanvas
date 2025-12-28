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

class ShapeState {
  public:
    virtual ~ShapeState() = default;
};

class Shape
{
  public:
    virtual ~Shape();

    virtual void draw(QPainter* painter) const;
    virtual bool isSelected() const;
    virtual void   setColor(const QColor& color);
    virtual QColor color() const;

    virtual void   rotate(double angle, const QPointF& customCenter) = 0;
    virtual double rotation() const;
    virtual void scale(double sx, double sy, std::optional<QPointF> center) = 0;
    virtual void resize(const QSizeF& targetSize, bool keepAspectRatio) = 0;

    virtual std::unique_ptr<ShapeState> createSnapshot() const = 0;
    virtual void restoreSnapshot(const ShapeState* state) = 0;

    virtual QPainterPath& path() const;
    virtual QRectF        boundingRect() const;

    virtual void      translate(const QPointF& offset) = 0;
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
    QColor               m_color;
    double               m_rotation = 0.0;

  friend class ShapeManager;
};

}// namespace xcanvas

#endif// BASESHAPE_H
