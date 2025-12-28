#include "ShapeText.h"
#include <QFontMetrics>

namespace xcanvas
{
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

void ShapeText::rotate(double angle, const QPointF& customCenter)
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

    QTransform t;
    t.translate(customCenter.x(), customCenter.y());
    t.rotate(angle);
    t.translate(-customCenter.x(), -customCenter.y());
    m_position = t.map(m_position);

    markDirty();
}

bool ShapeText::hitTest(const QPointF &point, const double tolerance) const {
    if (!isPointNearPath(point, tolerance)) {
        return path().contains(point);
    }
    return true;
}

void ShapeText::scale(double sx, double sy, std::optional<QPointF> center) {
}

void ShapeText::resize(const QSizeF &targetSize, bool keepAspectRatio) {
}

std::unique_ptr<ShapeState> ShapeText::createSnapshot() const {
}

void ShapeText::restoreSnapshot(const ShapeState *state) {
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

    QStringList   lines = m_text.split('\n');
    QFontMetricsF metrics(m_font);
    qreal         lineSpacing = metrics.lineSpacing();
    qreal         ascent      = metrics.ascent();

    qreal baseX = m_position.x();
    qreal baseY = m_position.y();

    // 生成原始（未旋转）的文本路径
    QPainterPath textPath;
    for (int i = 0; i < lines.size(); ++i)
    {
        QString line = lines[i];
        qreal   y    = baseY + ascent + i * lineSpacing;
        textPath.addText(baseX, y, m_font, line);
    }

    QTransform t;
    t.translate(baseX, baseY);
    t.rotate(m_rotation);
    t.translate(-baseX, -baseY);

    m_path = t.map(textPath);
}
}// namespace xcanvas
