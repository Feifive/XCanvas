#include "TextEditController.h"

#include "ViewRenderController.h"
#include "../Canvas/Canvas.h"
#include "../Shape/EditTextCommand.h"
#include "../Shape/ShapeManager.h"
#include "../Shape/ShapeText.h"

#include <QGuiApplication>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsView>

#include <cmath>
#include <limits>

TextEditController::TextEditController(
    QGraphicsView* const        view,
    xcanvas::Canvas* const      canvas,
    ViewRenderController* const viewRenderController,
    UpdateUiAction              updateSelectionHud,
    UpdateUiAction              requestFullUpdate)
    : m_view(view),
      m_canvas(canvas),
      m_viewRenderController(viewRenderController),
      m_updateSelectionHud(std::move(updateSelectionHud)),
      m_requestFullUpdate(std::move(requestFullUpdate))
{
}

bool TextEditController::isEditing() const
{
    return m_inlineEditingShape != nullptr;
}

bool TextEditController::beginInlineEdit(xcanvas::ShapeText* shape, const QPointF& scenePos)
{
    if (!shape || !m_view || m_inlineEditingShape)
    {
        return false;
    }

    m_inlineEditingShape = shape;
    m_inlineOriginalText = shape->text();
    m_editText           = shape->text();
    m_preeditText.clear();

    const QPointF localPos = shape->transform().inverted().map(scenePos);
    m_cursorPos = cursorPosAtLocalPos(localPos);

    m_view->setAttribute(Qt::WA_InputMethodEnabled, true);

    if (m_viewRenderController)
    {
        m_viewRenderController->setSuppressedShape(shape);
        m_viewRenderController->setSelectionHandlesVisible(false);
    }

    m_updateSelectionHud();
    m_requestFullUpdate();

    return true;
}

void TextEditController::finishInlineEdit(const bool commit)
{
    if (!m_inlineEditingShape)
    {
        return;
    }

    xcanvas::ShapeText* const editingShape = m_inlineEditingShape;
    const QString             editedText   = m_editText;

    m_inlineEditingShape = nullptr;
    m_inlineOriginalText.clear();
    m_editText.clear();
    m_preeditText.clear();
    m_cursorPos = 0;

    m_view->setAttribute(Qt::WA_InputMethodEnabled, false);

    if (editingShape && commit && editedText != m_inlineOriginalText && m_canvas && m_canvas->undoStack() && m_canvas->shapeManager())
    {
        m_canvas->undoStack()->push(
            new xcanvas::EditTextCommand(m_canvas->shapeManager(), editingShape, m_inlineOriginalText, editedText));
    }

    if (m_viewRenderController)
    {
        m_viewRenderController->setSuppressedShape(nullptr);
        m_viewRenderController->setSelectionHandlesVisible(true);
    }

    m_updateSelectionHud();
    m_requestFullUpdate();
}

bool TextEditController::moveCursorToScenePos(const QPointF& scenePos)
{
    if (!m_inlineEditingShape)
    {
        return false;
    }

    const QPointF          localPos   = m_inlineEditingShape->transform().inverted().map(scenePos);
    const QFontMetricsF    metrics(m_inlineEditingShape->font());
    const qreal            lineHeight = metrics.lineSpacing();
    const QStringList      lines      = m_editText.split('\n');
    const qreal            totalHeight = lineHeight * qMax(1, lines.size());

    if (localPos.y() >= -lineHeight && localPos.y() <= totalHeight + lineHeight)
    {
        m_cursorPos = cursorPosAtLocalPos(localPos);
        m_requestFullUpdate();
        return true;
    }

    return false;
}

int TextEditController::cursorPosAtLocalPos(const QPointF& localPos) const
{
    const QFontMetricsF metrics(m_inlineEditingShape->font());
    const qreal         lineHeight = metrics.lineSpacing();
    const QStringList   allLines   = m_editText.split('\n');

    int lineIndex = qMax(0, static_cast<int>(localPos.y() / lineHeight));
    lineIndex     = qMin(lineIndex, allLines.size() - 1);

    const QString& line    = allLines[lineIndex];
    int            bestPos = 0;
    qreal          bestDist = std::numeric_limits<qreal>::max();

    for (int i = 0; i <= line.length(); ++i)
    {
        const qreal charX = metrics.horizontalAdvance(line.left(i));
        const qreal dist  = std::abs(localPos.x() - charX);
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

void TextEditController::keyPressEvent(QKeyEvent* event)
{
    if (!m_inlineEditingShape)
    {
        return;
    }

    bool handled = true;

    switch (event->key())
    {
    case Qt::Key_Escape:
        finishInlineEdit(false);
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() & Qt::ShiftModifier)
        {
            m_editText.insert(m_cursorPos, '\n');
            ++m_cursorPos;
            m_requestFullUpdate();
        }
        else
        {
            finishInlineEdit(true);
        }
        break;

    case Qt::Key_Backspace:
        if (m_cursorPos > 0)
        {
            m_editText.remove(m_cursorPos - 1, 1);
            --m_cursorPos;
            m_requestFullUpdate();
        }
        break;

    case Qt::Key_Delete:
        if (m_cursorPos < m_editText.length())
        {
            m_editText.remove(m_cursorPos, 1);
            m_requestFullUpdate();
        }
        break;

    case Qt::Key_Left:
        if (m_cursorPos > 0)
        {
            --m_cursorPos;
            m_requestFullUpdate();
        }
        break;

    case Qt::Key_Right:
        if (m_cursorPos < m_editText.length())
        {
            ++m_cursorPos;
            m_requestFullUpdate();
        }
        break;

    case Qt::Key_Home:
        m_cursorPos = 0;
        m_requestFullUpdate();
        break;

    case Qt::Key_End:
        m_cursorPos = m_editText.length();
        m_requestFullUpdate();
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
        m_requestFullUpdate();
        event->accept();
    }
}

void TextEditController::inputMethodEvent(QInputMethodEvent* event)
{
    if (!m_inlineEditingShape)
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
    m_requestFullUpdate();

    if (QInputMethod* im = QGuiApplication::inputMethod())
    {
        im->update(Qt::ImCursorRectangle);
    }
}

QVariant TextEditController::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (!m_inlineEditingShape)
    {
        return {};
    }

    switch (query)
    {
    case Qt::ImCursorRectangle:
    {
        const QFontMetricsF metrics(m_inlineEditingShape->font());
        const qreal         lineHeight = metrics.lineSpacing();

        const QString     textBefore = m_editText.left(m_cursorPos);
        const QStringList lines      = textBefore.split('\n');
        const int         cursorLine = lines.size() - 1;
        const QString     lastLine   = lines.last();

        qreal cursorX = metrics.horizontalAdvance(lastLine);
        if (!m_preeditText.isEmpty())
        {
            cursorX += metrics.horizontalAdvance(m_preeditText);
        }
        const qreal cursorY = cursorLine * lineHeight;

        const QPointF localPos(cursorX, cursorY);
        const QPointF scenePos = m_inlineEditingShape->transform().map(localPos);
        const QPoint  viewPos  = m_view->mapFromScene(scenePos);

        return QRect(viewPos.x(), viewPos.y(), 2, qMax(1, static_cast<int>(lineHeight)));
    }
    case Qt::ImCursorPosition:
        return m_cursorPos;
    case Qt::ImFont:
        return m_inlineEditingShape->font();
    case Qt::ImSurroundingText:
        return m_editText;
    case Qt::ImCurrentSelection:
        return QString();
    default:
        return {};
    }
}

void TextEditController::drawPreview(QPainter* painter)
{
    if (!m_inlineEditingShape)
    {
        return;
    }

    painter->save();

    painter->setTransform(m_inlineEditingShape->transform(), true);
    painter->setFont(m_inlineEditingShape->font());
    painter->setPen(m_inlineEditingShape->color());

    const QFontMetricsF metrics(m_inlineEditingShape->font());
    const qreal         lineSpacing = metrics.lineSpacing();
    const qreal         ascent      = metrics.ascent();

    if (!m_editText.isEmpty())
    {
        const QStringList lines = m_editText.split('\n');
        for (int i = 0; i < lines.size(); ++i)
        {
            const qreal y = ascent + i * lineSpacing;
            painter->drawText(QPointF(0, y), lines[i]);
        }
    }

    drawPreedit(painter);
    drawCursor(painter);

    painter->restore();
}

void TextEditController::drawCursor(QPainter* painter) const
{
    const QFontMetricsF metrics(m_inlineEditingShape->font());
    const qreal         lineHeight = metrics.lineSpacing();

    const QString     textBefore = m_editText.left(m_cursorPos);
    const QStringList lines      = textBefore.split('\n');
    const int         cursorLine = lines.size() - 1;
    const QString     lastLine   = lines.last();

    qreal cursorX = metrics.horizontalAdvance(lastLine);
    if (!m_preeditText.isEmpty())
    {
        cursorX += metrics.horizontalAdvance(m_preeditText);
    }
    const qreal cursorY = cursorLine * lineHeight;

    painter->setPen(QPen(m_inlineEditingShape->color(), 1.5, Qt::SolidLine, Qt::FlatCap));
    painter->drawLine(QPointF(cursorX, cursorY), QPointF(cursorX, cursorY + lineHeight));
}

void TextEditController::drawPreedit(QPainter* painter) const
{
    if (m_preeditText.isEmpty())
    {
        return;
    }

    const QFontMetricsF metrics(m_inlineEditingShape->font());
    const qreal         lineSpacing = metrics.lineSpacing();
    const qreal         ascent      = metrics.ascent();

    const QString     textBefore = m_editText.left(m_cursorPos);
    const QStringList lines      = textBefore.split('\n');
    const int         cursorLine = lines.size() - 1;
    const QString     lastLine   = lines.last();

    const qreal preeditX = metrics.horizontalAdvance(lastLine);
    const qreal preeditY = ascent + cursorLine * lineSpacing;

    painter->drawText(QPointF(preeditX, preeditY), m_preeditText);

    const qreal underlineY  = preeditY + metrics.underlinePos();
    const qreal preeditWidth = metrics.horizontalAdvance(m_preeditText);
    painter->drawLine(QPointF(preeditX, underlineY), QPointF(preeditX + preeditWidth, underlineY));
}
