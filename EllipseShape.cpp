#include "EllipseShape.h"
#include "Global.h"
#include <QPainter>
#include <QPainterPath>

EllipseShape::EllipseShape()
{
}

EllipseShape::~EllipseShape()
{
    if (m_pPath)
    {
        delete m_pPath;
    }
}

void EllipseShape::Offset(const QPointF& offset)
{
    prepareGeometryChange();
    m_center += offset;
    UpdatePath();
    update();
}

QRectF EllipseShape::boundingRect() const
{
    return m_pPath ? m_pPath->boundingRect() : QRectF();
}

QPainterPath EllipseShape::shape() const
{
    return m_pPath ? *m_pPath : QPainterPath();
}

void EllipseShape::SetEllipse(const QPointF& center, double rx, double ry, double rotationDeg)
{
    prepareGeometryChange();
    m_center          = center;
    m_rx              = rx;
    m_ry              = ry;
    m_dRotationDegree = rotationDeg;
    m_dStartDegree    = 0;
    m_dEndDegree      = 360;

    UpdatePath();
    update();
}

void EllipseShape::SetEllipseArc(const QPointF& center, double rx, double ry, double rotationDeg, double startDeg, double endDeg)
{
    prepareGeometryChange();
    m_center          = center;
    m_rx              = rx;
    m_ry              = ry;
    m_dRotationDegree = rotationDeg;
    m_dStartDegree    = startDeg;
    m_dEndDegree      = endDeg;

    UpdatePath();
    update();
}

bool EllipseShape::IsCircle() const
{
    return IsEqual(m_rx, m_ry);
}

void EllipseShape::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (!m_pPath)
    {
        return;
    }

    qreal scale = painter->transform().m11();

    if (IsSelected())
    {
        painter->setPen(SELECTED_LINE_PEN(scale));
    }
    else
    {
        painter->setPen(DRAWED_LINE_PEN(scale));
    }

    painter->drawPath(*m_pPath);
}

void EllipseShape::UpdatePath()
{
    if (!m_pPath)
    {
        m_pPath = new QPainterPath;
    }

    m_pPath->clear();

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

    *m_pPath = transform.map(path);
}
