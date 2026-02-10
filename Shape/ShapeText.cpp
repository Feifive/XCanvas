#include "ShapeText.h"
#include <QFontMetrics>

namespace xcanvas
{
ShapeText::ShapeText()
{
}

ShapeText::ShapeText(const ShapeText& other) :
    Shape(other),
    m_font(other.m_font),
    m_text(other.m_text)
{
}

ShapeText::~ShapeText()
{
}

bool ShapeText::hitTest(const QPointF& point, const double tolerance) const
{
    if (!isPointNearPath(point, tolerance))
    {
        return path().contains(point);
    }
    return true;
}

Shape* ShapeText::clone()
{
    return new ShapeText(*this);
}

ShapeType ShapeText::type() const
{
    return ShapeType::Text;
}

void ShapeText::setFont(const QFont& font)
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

void ShapeText::updatePainterPath()
{
    m_originalPath = QPainterPath();
    if (m_text.isEmpty())
    {
        return;
    }

    QStringList         lines = m_text.split('\n');
    const QFontMetricsF metrics(m_font);
    const qreal         lineSpacing = metrics.lineSpacing();
    const qreal         ascent      = metrics.ascent();

    for (int i = 0; i < lines.size(); ++i)
    {
        const qreal y = ascent + i * lineSpacing;
        m_originalPath.addText(0, y, m_font, lines[i]);
    }
}
}// namespace xcanvas
