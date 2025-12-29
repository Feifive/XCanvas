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
    if (m_image.isNull()) {
        return;
    }

    painter->save();
    painter->setTransform(m_transform, true);
    const auto drawRect = QRectF(0, 0, m_imageSize.width(), m_imageSize.height());
    painter->drawImage(drawRect, m_image);
    if (m_selected)
    {
        painter->setPen(selectedPen());
        painter->drawRect(drawRect);
    }

    painter->restore();
}

bool ShapeImage::hitTest(const QPointF &point, double tolerance) const {
    if (!isPointNearPath(point, tolerance)) {
        return path().contains(point);
    }
    return true;
}

ShapeType ShapeImage::type() const
{
    return ShapeType::Image;
}

void ShapeImage::setSize(const QSizeF &size) {
    m_imageSize = size;
}

void ShapeImage::updatePainterPath()
{
    if (!m_imageSize.isValid() || m_image.isNull())
    {
        return;
    }

    m_originalPath = QPainterPath();
    m_originalPath.addRect(QRectF(0, 0, m_imageSize.width(), m_imageSize.height()));
}
}// namespace xcanvas
