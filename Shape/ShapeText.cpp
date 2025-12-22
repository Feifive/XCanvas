#include "ShapeText.h"
#include <QFontMetrics>

namespace xcanvas {
    ShapeText::ShapeText()
    {
    }

    ShapeText::~ShapeText()
    {
    }

    void ShapeText::translate(const QPointF& offset)
    {
        m_position += offset;

        markDirty();
    }

    ShapeType ShapeText::type() const
    {
        return ShapeType::Text;
    }

    void ShapeText::setFont(QFont font)
    {
        m_font = font;
        markDirty();
    }

    QFont ShapeText::font() const
    {
        return m_font;
    }

    void ShapeText::setText(const QString& text)
    {
        m_text = text;
        markDirty();
    }

    QString ShapeText::text() const
    {
        return m_text;
    }

    void ShapeText::setPosition(const QPointF& position)
    {
        m_position = position;
        markDirty();
    }

    QPointF ShapeText::position() const
    {
        return m_position;
    }

    void ShapeText::updatePainterPath()
    {
        m_path = QPainterPath();

        if (m_text.isEmpty())
        {
            return;
        }

        QStringList lines = m_text.split('\n');

        QFontMetricsF metrics(m_font);
        qreal         lineSpacing = metrics.lineSpacing();
        qreal         ascent      = metrics.ascent();

        // 文本基点（第一行左上角）
        qreal baseX = m_position.x();
        qreal baseY = m_position.y();

        // 逐行绘制
        for (int i = 0; i < lines.size(); ++i)
        {
            QString line = lines[i];

            // baseline Y
            qreal y = baseY + ascent + i * lineSpacing;

            QPainterPath path;
            path.addText(baseX, y, m_font, line);
            m_path.addPath(path);
        }
    }
}