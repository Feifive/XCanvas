#include "ShapeImage.h"
#include "Global.h"
#include <QPainter>

namespace xcanvas {
    ShapeImage::ShapeImage(QImage image) : m_image(std::move(image))
    {
    }

    ShapeImage::~ShapeImage() {
    }

    void ShapeImage::draw(QPainter* painter) const
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

    void ShapeImage::translate(const QPointF& offset)
    {
        m_rect.translate(offset);
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
        m_path.addRect(m_rect);
    }
}