#include "TextTool.h"
#include "../Canvas/ICanvasViewport.h"
#include "Canvas.h"
#include "ShapeText.h"
#include <QGuiApplication>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include <cmath>
#include <limits>

#include "AppSettings.h"

xcanvas::TextTool::TextTool(ICanvasViewport* view, Canvas* canvas) : DrawingTool(view, canvas)
{
    m_font.setFamily("MiSans");
    m_font.setPixelSize(24);
}

xcanvas::TextTool::~TextTool()
{
}

void xcanvas::TextTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    m_mousePos = m_canvasView->mapToWorld(event->pos());

    if (m_state == State::Drawing)
    {
        if (hitTest(m_mousePos))
        {
            m_cursorPos = cursorPosAtPoint(m_mousePos);
            m_canvasView->requestUpdate();
            return;
        }
        finishDrawing();
        return;
    }

    if (m_state == State::Idle)
    {
        startEdit(m_mousePos);
    }
}

void xcanvas::TextTool::mouseMoveEvent(QMouseEvent* event)
{
    handleRightButtonMove(event);
}

void xcanvas::TextTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
    }
}

void xcanvas::TextTool::keyPressEvent(QKeyEvent* event)
{
    if (m_state != State::Drawing)
    {
        DrawingTool::keyPressEvent(event);
        return;
    }

    bool handled = true;

    switch (event->key())
    {
    case Qt::Key_Escape:
        cancelDrawing();
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() & Qt::ShiftModifier)
        {
            m_editText.insert(m_cursorPos, '\n');
            ++m_cursorPos;
            m_canvasView->requestUpdate();
        }
        else
        {
            finishDrawing();
        }
        break;

    case Qt::Key_Backspace:
        if (m_cursorPos > 0)
        {
            m_editText.remove(m_cursorPos - 1, 1);
            --m_cursorPos;
            m_canvasView->requestUpdate();
        }
        break;

    case Qt::Key_Delete:
        if (m_cursorPos < m_editText.length())
        {
            m_editText.remove(m_cursorPos, 1);
            m_canvasView->requestUpdate();
        }
        break;

    case Qt::Key_Left:
        if (m_cursorPos > 0)
        {
            --m_cursorPos;
            m_canvasView->requestUpdate();
        }
        break;

    case Qt::Key_Right:
        if (m_cursorPos < m_editText.length())
        {
            ++m_cursorPos;
            m_canvasView->requestUpdate();
        }
        break;

    case Qt::Key_Home:
        m_cursorPos = 0;
        m_canvasView->requestUpdate();
        break;

    case Qt::Key_End:
        m_cursorPos = m_editText.length();
        m_canvasView->requestUpdate();
        break;

    default:
        handled = false;
        break;
    }

    if (handled)
    {
        event->accept();
        return;
    }

    const QString text = event->text();
    if (!text.isEmpty())
    {
        for (const QChar& ch : text)
        {
            if (ch.isPrint())
            {
                m_editText.insert(m_cursorPos, ch);
                ++m_cursorPos;
            }
        }
        m_canvasView->requestUpdate();
        event->accept();
    }
}

void xcanvas::TextTool::inputMethodEvent(QInputMethodEvent* event)
{
    if (m_state != State::Drawing)
    {
        return;
    }

    const QString commitStr = event->commitString();
    if (!commitStr.isEmpty())
    {
        for (const QChar& ch : commitStr)
        {
            m_editText.insert(m_cursorPos, ch);
            ++m_cursorPos;
        }
    }

    m_preeditText = event->preeditString();
    m_canvasView->requestUpdate();

    if (QInputMethod* im = QGuiApplication::inputMethod())
    {
        im->update(Qt::ImCursorRectangle);
    }
}

QVariant xcanvas::TextTool::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (m_state != State::Drawing)
    {
        return {};
    }

    switch (query)
    {
    case Qt::ImCursorRectangle:
    {
        const QFontMetricsF metrics(m_font);
        const qreal         lineHeight = metrics.lineSpacing();

        const QString     textBefore = m_editText.left(m_cursorPos);
        const QStringList lines      = textBefore.split('\n');
        const int         cursorLine = lines.size() - 1;
        const QString     lastLine   = lines.last();

        qreal cursorX = m_textPos.x() + metrics.horizontalAdvance(lastLine);
        if (!m_preeditText.isEmpty())
        {
            cursorX += metrics.horizontalAdvance(m_preeditText);
        }
        const qreal cursorY = m_textPos.y() + cursorLine * lineHeight;

        const QRectF sceneRect(cursorX, cursorY, 2.0, lineHeight);
        const QPoint viewPos = m_canvasView->mapFromWorld(sceneRect.topLeft()).toPoint();
        return QRect(viewPos.x(), viewPos.y(), 2, qMax(1, static_cast<int>(lineHeight)));
    }
    case Qt::ImCursorPosition:
        return m_cursorPos;
    case Qt::ImFont:
        return m_font;
    case Qt::ImSurroundingText:
        return m_editText;
    case Qt::ImCurrentSelection:
        return QString();
    default:
        return {};
    }
}

void xcanvas::TextTool::drawPreview(QPainter* painter)
{
    if (m_state != State::Drawing)
        return;

    painter->save();

    const QColor color = AppSettings::instance().activeColor();

    const QFontMetricsF metrics(m_font);
    const qreal         lineSpacing = metrics.lineSpacing();
    const qreal         ascent      = metrics.ascent();

    if (!m_editText.isEmpty())
    {
        const ProcessMode mode = activeProcessMode();
        if (mode == ProcessMode::Scan)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
        }
        else
        {
            painter->setPen(QPen(color, 1.0, Qt::SolidLine, Qt::FlatCap));
            painter->setBrush(Qt::NoBrush);
        }
        painter->setRenderHint(QPainter::Antialiasing);

        QPainterPath textPath;
        const QStringList lines = m_editText.split('\n');
        for (int i = 0; i < lines.size(); ++i)
        {
            const qreal y = m_textPos.y() + ascent + i * lineSpacing;
            textPath.addText(m_textPos.x(), y, m_font, lines[i]);
        }
        painter->drawPath(textPath);
    }

    drawPreeditText(painter);
    drawTextCursor(painter);

    painter->restore();
}

DrawingToolType xcanvas::TextTool::toolType()
{
    return DrawingToolType::Text;
}

void xcanvas::TextTool::cancelDrawing()
{
    finishEdit(false);
    DrawingTool::cancelDrawing();
}

void xcanvas::TextTool::finishDrawing()
{
    if (!finishEdit(true))
    {
        DrawingTool::cancelDrawing();
        return;
    }

    DrawingTool::finishDrawing();
}

void xcanvas::TextTool::startEdit(const QPointF& pos)
{
    m_textPos      = pos;
    m_editText.clear();
    m_preeditText.clear();
    m_cursorPos    = 0;
    m_state        = State::Drawing;
    m_canvasView->setInputMethodEnabled(true);
    m_canvasView->setViewCursor(Qt::IBeamCursor);
    m_canvasView->focusViewport();
    m_canvasView->requestUpdate();
}

bool xcanvas::TextTool::finishEdit(bool commit)
{
    m_state = State::Idle;
    m_canvasView->setInputMethodEnabled(false);
    m_canvasView->setViewCursor(Qt::ArrowCursor);

    const QString plainText = m_editText;
    m_editText.clear();
    m_preeditText.clear();
    m_cursorPos = 0;

    if (commit && !plainText.isEmpty())
    {
        auto* shape = new ShapeText();
        shape->setText(plainText);
        shape->translate(m_textPos);
        shape->setFont(m_font);
        shape->setColor(AppSettings::instance().activeColor());
        m_canvas->addShape(shape);
        m_canvas->shapeManager()->selectShape(shape, true);

        m_canvasView->requestUpdate();
        return true;
    }

    m_canvasView->requestUpdate();
    return false;
}

bool xcanvas::TextTool::hitTest(const QPointF& scenePos) const
{
    const QFontMetricsF metrics(m_font);
    const qreal         lineHeight   = metrics.lineSpacing();
    const QStringList   lines        = m_editText.split('\n');
    const qreal         totalHeight  = lineHeight * qMax(1, lines.size());

    qreal maxWidth = 0;
    for (const QString& line : lines)
    {
        maxWidth = qMax(maxWidth, metrics.horizontalAdvance(line));
    }

    const QPointF local = scenePos - m_textPos;
    return local.x() >= -2.0 && local.x() <= maxWidth + 2.0 &&
           local.y() >= -lineHeight && local.y() <= totalHeight + lineHeight;
}

int xcanvas::TextTool::cursorPosAtPoint(const QPointF& scenePos) const
{
    const QPointF      local = scenePos - m_textPos;
    const QFontMetricsF metrics(m_font);
    const qreal         lineHeight = metrics.lineSpacing();
    const QStringList   allLines   = m_editText.split('\n');

    int lineIndex = qMax(0, static_cast<int>(local.y() / lineHeight));
    lineIndex     = qMin(lineIndex, allLines.size() - 1);

    const QString& line     = allLines[lineIndex];
    int            bestPos  = 0;
    qreal          bestDist = std::numeric_limits<qreal>::max();

    for (int i = 0; i <= line.length(); ++i)
    {
        const qreal charX = metrics.horizontalAdvance(line.left(i));
        const qreal dist  = std::abs(local.x() - charX);
        if (dist < bestDist)
        {
            bestDist = dist;
            bestPos  = i;
        }
    }

    int cursorPos = bestPos;
    for (int i = 0; i < lineIndex; ++i)
    {
        cursorPos += allLines[i].length() + 1;
    }

    return qBound(0, cursorPos, m_editText.length());
}

xcanvas::ProcessMode xcanvas::TextTool::activeProcessMode() const
{
    const QColor color = AppSettings::instance().activeColor();
    const xcanvas::LayerManager* lm = m_canvas->layerManager();
    if (lm)
    {
        for (int id : lm->layerIds())
        {
            const xcanvas::LayerParameter* layer = lm->tryGetLayer(id);
            if (layer && layer->color == color && layer->mode != xcanvas::ProcessMode::Image)
            {
                return layer->mode;
            }
        }
    }
    return xcanvas::ProcessMode::Cut;
}

void xcanvas::TextTool::drawTextCursor(QPainter* painter) const
{
    const QFontMetricsF metrics(m_font);
    const qreal         lineHeight = metrics.lineSpacing();

    const QString     textBefore = m_editText.left(m_cursorPos);
    const QStringList lines      = textBefore.split('\n');
    const int         cursorLine = lines.size() - 1;
    const QString     lastLine   = lines.last();

    qreal cursorX = m_textPos.x() + metrics.horizontalAdvance(lastLine);

    if (!m_preeditText.isEmpty())
    {
        cursorX += metrics.horizontalAdvance(m_preeditText);
    }

    const qreal cursorY = m_textPos.y() + cursorLine * lineHeight;

    const QColor color = AppSettings::instance().activeColor();
    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(QPointF(cursorX, cursorY), QPointF(cursorX, cursorY + lineHeight));
}

void xcanvas::TextTool::drawPreeditText(QPainter* painter) const
{
    if (m_preeditText.isEmpty())
        return;

    painter->setPen(AppSettings::instance().activeColor());

    const QFontMetricsF metrics(m_font);
    const qreal         lineSpacing = metrics.lineSpacing();
    const qreal         ascent      = metrics.ascent();

    const QString     textBefore = m_editText.left(m_cursorPos);
    const QStringList lines      = textBefore.split('\n');
    const int         cursorLine = lines.size() - 1;
    const QString     lastLine   = lines.last();

    const qreal preeditX = m_textPos.x() + metrics.horizontalAdvance(lastLine);
    const qreal preeditY = m_textPos.y() + ascent + cursorLine * lineSpacing;

    painter->drawText(QPointF(preeditX, preeditY), m_preeditText);

    const qreal underlineY  = preeditY + metrics.underlinePos();
    const qreal preeditWidth = metrics.horizontalAdvance(m_preeditText);
    painter->drawLine(QPointF(preeditX, underlineY), QPointF(preeditX + preeditWidth, underlineY));
}
