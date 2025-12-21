#include "Image.h"
#include "Global.h"
#include <QPainter>
xcanvas::Image::Image(QImage image) : m_image(std::move(image))
{
}

xcanvas::Image::~Image() {
}

void xcanvas::Image::draw(QPainter* painter) const
{
    if (m_image.isNull())
    {
        return;
    }

    painter->save();
    painter->drawImage(m_rect, m_image);
    if (m_selected)
    {
        painter->setPen(selectedPen());
        painter->drawPath(path());
    }
    painter->restore();
}

void xcanvas::Image::translate(const QPointF& offset)
{
    m_rect.translate(offset);
    markDirty();
}

xcanvas::ShapeType xcanvas::Image::type() const
{
    return ShapeType::Image;
}

void xcanvas::Image::setRect(const QRectF& rect)
{
    m_rect = rect;
    markDirty();
}

void xcanvas::Image::updatePainterPath()
{
    if (!m_rect.isValid() || m_image.isNull())
    {
        return;
    }

    m_path = QPainterPath();
    m_path.addRect(m_rect);
}
