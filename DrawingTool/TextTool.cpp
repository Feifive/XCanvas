#include "TextTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "ShapeText.h"
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QTextCursor>

xcanvas::TextTool::TextTool(MyGraphicsView* view, Canvas* canvas) : DrawingTool(view, canvas), m_pTextItem(nullptr)
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

    m_mousePos = m_canvasView->mapToScene(event->pos());

    if (m_state == State::Idle)
    {
        startEdit();
    }
    else if (m_state == State::Drawing && m_pTextItem)
    {
        QPointF                      localPos = m_pTextItem->mapFromScene(m_mousePos);
        QAbstractTextDocumentLayout* pLayout  = m_pTextItem->document()->documentLayout();
        int                          nPos     = pLayout->hitTest(localPos, Qt::ExactHit);

        if (nPos < 0)
        {
            cancelDrawing();
            return;
        }

        QTextCursor cursor = m_pTextItem->textCursor();
        cursor.setPosition(nPos);
        m_pTextItem->setTextCursor(cursor);
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

DrawingToolType xcanvas::TextTool::toolType()
{
    return DrawingToolType::Text;
}

void xcanvas::TextTool::cancelDrawing()
{
    finishEdit();
    DrawingTool::cancelDrawing();
}

void xcanvas::TextTool::startEdit()
{
    m_pTextItem = new QGraphicsTextItem("HELLO");
    m_pTextItem->document()->setDocumentMargin(0);
    m_pTextItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    m_pTextItem->setPos(m_mousePos);
    m_pTextItem->setFont(m_font);
    m_pTextItem->setDefaultTextColor(Qt::black);
    m_pTextItem->setFocus();

    QTextCursor cursor = m_pTextItem->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_pTextItem->setTextCursor(cursor);

    m_canvasView->scene()->addItem(m_pTextItem);

    m_state = State::Drawing;
}

void xcanvas::TextTool::finishEdit()
{
    if (!m_pTextItem)
    {
        return;
    }

    m_canvasView->scene()->removeItem(m_pTextItem);

    const QString plainText = m_pTextItem->toPlainText();
    if (!plainText.isEmpty())
    {
        auto* shape = new ShapeText();
        shape->setText(plainText);
        shape->translate(m_pTextItem->pos());
        shape->setFont(m_font);
        m_canvas->addShape(shape);
        m_canvas->shapeManager()->selectShape(shape, true);

        m_canvasView->requestFullUpdate();
    }

    delete m_pTextItem;
    m_pTextItem = nullptr;
}
