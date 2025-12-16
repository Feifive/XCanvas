#include "Image.h"
#include <QPainter>
xcanvas::Image::Image(QImage image) : m_image(std::move(image)) {
}

xcanvas::Image::~Image() = default;

void xcanvas::Image::draw(QPainter *painter) const {
    if (m_image.isNull()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(m_rect, m_image);
    painter->restore();

    Shape::draw(painter);
}

void xcanvas::Image::translate(const QPointF &offset) {
    m_rect.translate(offset);
    markDirty();
}

void xcanvas::Image::setRect(const QRectF &rect) {
    m_rect = rect;
    markDirty();
}

void xcanvas::Image::updatePainterPath() {
    if (!m_rect.isValid() || m_image.isNull()) {
        return;
    }

    m_path = QPainterPath();
    m_path.addRect(m_rect);
}
