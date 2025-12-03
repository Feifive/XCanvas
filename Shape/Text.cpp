#include "Text.h"
#include <QFontMetrics>

xcanvas::Text::Text()
{

}

xcanvas::Text::~Text()
{

}

void xcanvas::Text::translate(const QPointF &offset)
{
    m_position += offset;

    markDirty();
}

void xcanvas::Text::setFont(QFont font)
{
    m_font = font;
    markDirty();
}

QFont xcanvas::Text::font() const
{
    return m_font;
}

void xcanvas::Text::setText(const QString &text)
{
    m_text = text;
    markDirty();
}

QString xcanvas::Text::text() const
{
    return m_text;
}

void xcanvas::Text::setPosition(const QPointF &position)
{
    m_position = position;
    markDirty();
}

QPointF xcanvas::Text::position() const
{
    return m_position;
}

void xcanvas::Text::updatePainterPath()
{
    m_path = QPainterPath();

    if (m_text.isEmpty())
    {
        return;
    }

    QStringList lines = m_text.split('\n');

    QFontMetricsF metrics(m_font);
    qreal lineSpacing = metrics.lineSpacing();
    qreal ascent      = metrics.ascent();

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
