#include "Shape.h"
#include "Global.h"
#include "MyMath.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <cmath>

namespace xcanvas {
    Shape::Shape(const Shape& other) :
        m_selected(other.m_selected),
        m_dirty(other.m_dirty),
        m_boundingRectDirty(other.m_boundingRectDirty),
        m_cachedBoundingRect(other.m_cachedBoundingRect),
        m_path(other.m_path),
        m_originalPath(other.m_originalPath),
        m_color(other.m_color),
        m_rotation(other.m_rotation),
        m_transform(other.m_transform),
        m_groupId(other.m_groupId)
    {
    }

    void Shape::setLayerId(int layerId) {
        m_layerId = layerId;
    }

    QString Shape::groupId() const
    {
        return m_groupId;
    }

    void Shape::setGroupId(const QString& groupId)
    {
        m_groupId = groupId;
    }

    Shape::Shape() :
        m_selected(false),
        m_dirty(true),
        m_boundingRectDirty(true),
        m_color(Qt::black),
        m_rotation(0.0)
    {
    }

    Shape::~Shape()
    {
    }

    void Shape::draw(QPainter* painter) const
    {
        painter->drawPath(path());
    }

    bool Shape::isImage() const
    {
        return false;
    }

    void Shape::setSelected(bool selected)
    {
        m_selected = selected;
    }

    bool Shape::isPointNearPath(const QPointF &point, const double tolerance) const {
        if (!boundingRect().adjusted(-tolerance, -tolerance, tolerance, tolerance).contains(point)) {
            return false;
        }

        const QList<QPolygonF>& polygons = path().toSubpathPolygons();

        for (const QPolygonF& polygon : polygons)
        {
            QRectF rect = polygon.boundingRect();
            rect.adjust(-tolerance, -tolerance, tolerance, tolerance);
            if (!rect.contains(point))
            {
                continue;
            }
            for (int j = 0; j < polygon.size() - 1; ++j)
            {
                if (geometryMath::point2Segment(polygon[j], polygon[j + 1], point) <= tolerance)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool Shape::isSelected() const {
        return m_selected;
    }

    void Shape::setColor(const QColor& color)
    {
        m_color = color;
        markDirty();
    }

    QColor Shape::color() const
    {
        return m_color;
    }

    void Shape::setTransform(const QTransform &transform) {
        m_transform = transform;
        markDirty();
    }

    void Shape::translate(const QPointF &offset) {
        if (qFuzzyIsNull(offset)) {
            return;
        }
        QTransform delta;
        delta.translate(offset.x(), offset.y());
        setTransform(m_transform * delta);
    }

    void Shape::rotate(const double angle, const QPointF &center) {
        if (qFuzzyIsNull(angle)) {
            return;
        }
        QTransform delta;
        delta.translate(center.x(), center.y());
        delta.rotate(angle);
        delta.translate(-center.x(), -center.y());
        setTransform(m_transform * delta);
    }

    void Shape::scale(const double sx, const double sy, const QPointF &anchor) {
        if (qFuzzyCompare(sx, 1.0) && qFuzzyCompare(sy, 1.0)) {
            return;
        }

        constexpr double minScale = 1e-6;
        double safeSx = sx;
        double safeSy = sy;

        if (qAbs(sx) < minScale) {
            safeSx = (sx >= 0 ? minScale : -minScale);
        }
        if (qAbs(sy) < minScale) {
            safeSy = (sy >= 0 ? minScale : -minScale);
        }

        QTransform delta;
        delta.translate(anchor.x(), anchor.y());
        delta.scale(safeSx, safeSy);
        delta.translate(-anchor.x(), -anchor.y());
        setTransform(m_transform * delta);
    }

    double Shape::rotationDeg() const
    {
        // Qt 画布坐标系下使用 m12/m11 可得到顺时针角度
        double deg = qRadiansToDegrees(qAtan2(m_transform.m12(), m_transform.m11()));
        deg = std::fmod(deg, 360.0);
        if (deg < 0.0)
        {
            deg += 360.0;
        }
        if (qAbs(deg - 360.0) < 1e-6)
        {
            deg = 0.0;
        }
        return deg;
    }

    QPainterPath& Shape::path() const
    {
        if (m_dirty)
        {
            m_dirty = false;
            const_cast<Shape*>(this)->updatePainterPath();
            m_path = m_transform.map(m_originalPath);
        }

        return m_path;
    }

    QRectF Shape::boundingRect() const
    {
        if (!m_boundingRectDirty)
        {
            return m_cachedBoundingRect;
        }

        const QRectF rawRect = path().boundingRect();

        qreal left   = rawRect.left();
        qreal top    = rawRect.top();
        qreal right  = rawRect.right();
        qreal bottom = rawRect.bottom();

        const double delta = BOUNDING_BOX_TOLERANCE;

        if (qFuzzyIsNull(right - left))
        {
            left -= delta / 2.0;
            right += delta / 2.0;
        }

        if (qFuzzyIsNull(bottom - top))
        {
            top -= delta / 2.0;
            bottom += delta / 2.0;
        }

        m_boundingRectDirty  = false;
        m_cachedBoundingRect = QRectF(QPointF(left, top), QPointF(right, bottom));

        return m_cachedBoundingRect;
    }

    bool Shape::isDirty() const
    {
        return m_dirty;
    }

    void Shape::setDirty(const bool dirty) const
    {
        m_dirty = dirty;
    }

    int Shape::layerId() const {
        return m_layerId;
    }

    void Shape::markDirty() const
{
    m_dirty             = true;
    m_boundingRectDirty = true;
}
}// namespace xcanvas
