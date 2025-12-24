#include "ShapeImage.h"
#include "Global.h"
#include <QPainter>

namespace xcanvas
{
ShapeImage::ShapeImage(QImage image) : m_image(std::move(image))
{
}

ShapeImage::~ShapeImage()
{
}

void ShapeImage::draw(QPainter* painter) const
{
    if (m_image.isNull())
        return;

    painter->save();

    // 1. 设置渲染抗锯齿（使旋转后的边缘平滑）
    // painter->setRenderHint(QPainter::SmoothPixmapTransform);

    // 2. 坐标变换：绕矩形中心旋转
    QPointF center = m_rect.center();
    painter->translate(center);// 将原点移至图片中心
    painter->rotate(m_rotation);// 执行旋转

    // 3. 绘制图片
    // 因为原点已经移到中心，所以绘制矩形需要偏移回左上角
    QRectF drawRect(-m_rect.width() / 2.0, -m_rect.height() / 2.0, m_rect.width(), m_rect.height());
    painter->drawImage(drawRect, m_image);

    // 4. 绘制选中框
    if (m_selected)
    {
        painter->setPen(selectedPen());
        // 此时坐标系已旋转，直接绘制 drawRect 即可对应旋转后的路径
        painter->drawRect(drawRect);
    }

    painter->restore();
}

void ShapeImage::translate(const QPointF& offset)
{
    m_rect.translate(offset);
    markDirty();
}

void ShapeImage::rotate(double angle, const QPointF& customCenter)
{
    if (qFuzzyIsNull(angle))
    {
        return;
    }

    m_rotation = fmod(m_rotation + angle, 360.0);
    if (m_rotation < 0)
    {
        m_rotation += 360.0;
    }

    markDirty();
}

ShapeType ShapeImage::type() const
{
    return ShapeType::Image;
}

void ShapeImage::setRect(const QRectF& rect)
{
    m_rect = rect;
    markDirty();
}

void ShapeImage::updatePainterPath()
{
    if (!m_rect.isValid() || m_image.isNull())
    {
        return;
    }

    m_path = QPainterPath();

    QPainterPath rectPath;
    rectPath.addRect(m_rect);

    QTransform transform;
    QPointF    center = m_rect.center();
    transform.translate(center.x(), center.y());
    transform.rotate(m_rotation);
    transform.translate(-center.x(), -center.y());

    m_path = transform.map(rectPath);
}
}// namespace xcanvas
