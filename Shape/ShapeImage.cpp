#include "ShapeImage.h"
#include "Global.h"

namespace xcanvas
{
ShapeImage::ShapeImage(QImage image) : m_image(std::move(image))
{
}

ShapeImage::ShapeImage(const ShapeImage& other) :
    Shape(other),
    m_image(other.m_image),
    m_imageSize(other.m_imageSize)
{
}

ShapeImage::~ShapeImage()
{
}

bool ShapeImage::isImage() const
{
    return true;
}

bool ShapeImage::hitTest(const QPointF &point, double tolerance) const {
    if (!isPointNearPath(point, tolerance)) {
        return path().contains(point);
    }
    return true;
}

Shape * ShapeImage::clone() {
    return new ShapeImage(*this);
}

ShapeType ShapeImage::type() const
{
    return ShapeType::Image;
}

void ShapeImage::setSize(const QSizeF &size) {
    m_imageSize = size;
    markDirty();
}

const QImage& ShapeImage::image() const
{
    return m_image;
}

QSizeF ShapeImage::imageSize() const
{
    return m_imageSize;
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
