#include <QMouseEvent>
#include <QGraphicsTextItem>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include "TextTool.h"
#include "../MyGraphicsView.h"
#include "Text.h"
#include "ShapeManager.h"

xcanvas::TextTool::TextTool(MyGraphicsView *pView) :
    DrawingTool(pView),
    m_pTextItem(nullptr)
{
    m_font.setFamily("PingFang SC");
    m_font.setPixelSize(24);
}

xcanvas::TextTool::~TextTool()
{

}

void xcanvas::TextTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    m_mousePos = m_pView->mapToScene(event->pos());

    if(m_state == State::Idle)
    {
        startEdit();
    }
    else if (m_state == State::Drawing && m_pTextItem)
    {
        QPointF localPos = m_pTextItem->mapFromScene(m_mousePos);
        QAbstractTextDocumentLayout* pLayout = m_pTextItem->document()->documentLayout();
        int nPos = pLayout->hitTest(localPos, Qt::ExactHit);

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

void xcanvas::TextTool::mouseMoveEvent(QMouseEvent *event)
{
    handleRightButtonMove(event);
}

void xcanvas::TextTool::mouseReleaseEvent(QMouseEvent *event)
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
    m_pTextItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    m_pTextItem->setPos(m_mousePos);
    m_pTextItem->setFont(m_font);
    m_pTextItem->setDefaultTextColor(Qt::black);
    m_pTextItem->setFocus();

    QTextCursor cursor = m_pTextItem->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_pTextItem->setTextCursor(cursor);

    m_pView->scene()->addItem(m_pTextItem);

    m_state = State::Drawing;
}

void xcanvas::TextTool::finishEdit()
{
    if(!m_pTextItem)
    {
        return;
    }

    m_pView->scene()->removeItem(m_pTextItem);

    const QString plainText = m_pTextItem->toPlainText();
    if(!plainText.isEmpty())
    {
        Text* pShape = new Text();
        pShape->setText(plainText);
        pShape->setPosition(m_pTextItem->pos());
        pShape->setFont(m_font);
        pShape->setSelected(true);
        m_pView->GetCurrentShapes()->deselectAll();
        m_pView->addShape(pShape);


        m_pView->updateCanvas();
    }

    delete m_pTextItem;
    m_pTextItem = nullptr;
}
