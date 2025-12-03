#include "Ellipse.h"
#include "Global.h"
#include <QPainter>
#include <QPainterPath>

xcanvas::Ellipse::Ellipse()
{
}

xcanvas::Ellipse::~Ellipse()
{
}

void xcanvas::Ellipse::translate(const QPointF& offset)
{
    m_center += offset;

    markDirty();
}

void xcanvas::Ellipse::setEllipse(const QPointF& center, double rx, double ry, double rotationDeg)
{
    m_center          = center;
    m_rx              = rx;
    m_ry              = ry;
    m_dRotationDegree = rotationDeg;
    m_dStartDegree    = 0;
    m_dEndDegree      = 360;

    markDirty();
}

void xcanvas::Ellipse::setEllipse(const QRectF &rect)
{
    m_center          = rect.center();
    m_rx              = rect.width() / 2.0;
    m_ry              = rect.height() / 2.0;
    m_dRotationDegree = 0;
    m_dStartDegree    = 0;
    m_dEndDegree      = 360;

    markDirty();
}

void xcanvas::Ellipse::setEllipseArc(const QPointF& center, double rx, double ry, double rotationDeg, double startDeg, double endDeg)
{
    m_center          = center;
    m_rx              = rx;
    m_ry              = ry;
    m_dRotationDegree = rotationDeg;
    m_dStartDegree    = startDeg;
    m_dEndDegree      = endDeg;

    markDirty();
}

bool xcanvas::Ellipse::isCircle() const
{
    return IsEqual(m_rx, m_ry);
}

void xcanvas::Ellipse::updatePainterPath()
{
    QRectF rc(m_center.x() - m_rx, m_center.y() - m_ry, m_rx * 2, m_ry * 2);

    QPainterPath path;

    if (IsEqual(m_dStartDegree, 0.0) && IsEqual(m_dEndDegree, 360.0))
    {
        // 完整椭圆
        path.addEllipse(rc);
    }
    else
    {
        // 椭圆弧
        path.arcMoveTo(rc, m_dStartDegree);
        path.arcTo(rc, m_dStartDegree, m_dEndDegree - m_dStartDegree);
    }

    // 对椭圆应用旋转（绕中心）
    QTransform transform;
    transform.translate(m_center.x(), m_center.y());
    transform.rotate(m_dRotationDegree);
    transform.translate(-m_center.x(), -m_center.y());

    m_path = transform.map(path);
}
