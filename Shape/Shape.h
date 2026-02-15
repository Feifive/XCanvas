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
    virtual bool isImage() const;
    virtual bool isSelected() const;
    virtual bool isVisible() const {return m_visible;};
    virtual void setVisible(const bool visible) {m_visible = visible;};
    virtual void   setColor(const QColor& color);
    virtual QColor color() const;

    void setTransform(const QTransform& transform);
    QTransform transform() const { return m_transform; }
    void resetTransform() { setTransform(QTransform()); }
    virtual void translate(const QPointF& offset);
    virtual void rotate(double angle, const QPointF& center);
    virtual void scale(double sx, double sy, const QPointF& anchor);
    double rotationDeg() const;

    virtual QPainterPath& path() const;
    virtual QRectF        boundingRect() const;

    virtual ShapeType type() const                     = 0;
    virtual bool hitTest(const QPointF& point, double  tolerance) const = 0;

    virtual Shape* clone() = 0;

    bool isDirty() const;
    void setDirty(const bool dirty) const;

    int layerId() const;
    void setLayerId(int layerId);

  protected:
    Shape();
    Shape(const Shape& other);
    virtual void updatePainterPath() = 0;
    void         markDirty() const;
    virtual void setSelected(bool selected);
    bool isPointNearPath(const QPointF& point, double tolerance) const;

  protected:
    bool                 m_selected = false;
    bool                 m_visible  = true;
    mutable bool         m_dirty;
    mutable bool         m_boundingRectDirty;
    mutable QRectF       m_cachedBoundingRect;
    mutable QPainterPath m_path;
    mutable QPainterPath m_originalPath;
    QColor               m_color;
    double               m_rotation;
    QTransform           m_transform;
    int m_layerId = -1;

  friend class ShapeManager;
};

}// namespace xcanvas

#endif// BASESHAPE_H
