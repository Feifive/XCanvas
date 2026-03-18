#include "TextEditController.h"

#include "ViewRenderController.h"
#include "../Canvas/Canvas.h"
#include "Global.h"
#include "../Shape/EditTextCommand.h"
#include "../Shape/ShapeManager.h"
#include "../Shape/ShapeText.h"

#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTimer>
#include <QUndoStack>

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
    return m_inlineTextEditor != nullptr;
}

bool TextEditController::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_inlineTextEditor || !event)
    {
        return false;
    }

    if (event->type() == QEvent::FocusOut)
    {
        QTimer::singleShot(0, m_view, [this]() { finishInlineEdit(true); });
    }
    else if (event->type() == QEvent::KeyPress)
    {
        const auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent && keyEvent->key() == Qt::Key_Escape)
        {
            finishInlineEdit(false);
            return true;
        }
    }

    return false;
}

bool TextEditController::beginInlineEdit(xcanvas::ShapeText* shape)
{
    QGraphicsScene* const scene = m_view ? m_view->scene() : nullptr;
    if (!shape || !scene || m_inlineTextEditor)
    {
        return false;
    }

    m_inlineEditingShape = shape;
    m_inlineOriginalText = shape->text();

    m_inlineTextEditor = new QGraphicsTextItem(shape->text());
    m_inlineTextEditor->document()->setDocumentMargin(0);
    m_inlineTextEditor->setTextInteractionFlags(Qt::TextEditorInteraction);
    m_inlineTextEditor->setFont(shape->font());
    m_inlineTextEditor->setDefaultTextColor(shape->color());
    m_inlineTextEditor->setTransform(shape->transform());
    m_inlineTextEditor->setZValue(Z_VALUE_HIGHLIGHT + 1);
    m_inlineTextEditor->installEventFilter(m_view);

    scene->addItem(m_inlineTextEditor);
    m_inlineTextEditor->setFocus(Qt::MouseFocusReason);

    QTextCursor cursor = m_inlineTextEditor->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_inlineTextEditor->setTextCursor(cursor);

    if (m_viewRenderController)
    {
        m_viewRenderController->setSuppressedShape(shape);
        m_viewRenderController->setSelectionHandlesVisible(false);
    }

    if (m_updateSelectionHud)
    {
        m_updateSelectionHud();
    }
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }

    return true;
}

void TextEditController::finishInlineEdit(const bool commit)
{
    if (!m_inlineTextEditor)
    {
        return;
    }

    const QString editedText = m_inlineTextEditor->toPlainText();
    xcanvas::ShapeText* const editingShape = m_inlineEditingShape;

    m_inlineTextEditor->removeEventFilter(m_view);
    if (QGraphicsScene* const scene = m_view ? m_view->scene() : nullptr)
    {
        scene->removeItem(m_inlineTextEditor);
    }
    delete m_inlineTextEditor;
    m_inlineTextEditor = nullptr;

    if (editingShape)
    {
        if (commit && editedText != m_inlineOriginalText && m_canvas && m_canvas->undoStack() && m_canvas->shapeManager())
        {
            m_canvas->undoStack()->push(
                new xcanvas::EditTextCommand(m_canvas->shapeManager(), editingShape, m_inlineOriginalText, editedText));
        }
    }

    m_inlineEditingShape = nullptr;
    m_inlineOriginalText.clear();

    if (m_viewRenderController)
    {
        m_viewRenderController->setSuppressedShape(nullptr);
        m_viewRenderController->setSelectionHandlesVisible(true);
    }
    if (m_updateSelectionHud)
    {
        m_updateSelectionHud();
    }
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
}
