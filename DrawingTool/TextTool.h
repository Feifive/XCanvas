#ifndef TEXTTOOL_H
#define TEXTTOOL_H

#include "DrawingTool.h"
#include <QFont>

namespace xcanvas
{
class TextTool : public DrawingTool
{
    Q_OBJECT

  public:
    explicit TextTool(MyGraphicsView* view, Canvas* canvas);
    virtual ~TextTool() override;

    void                    mousePressEvent(QMouseEvent* event) override;
    void                    mouseMoveEvent(QMouseEvent* event) override;
    void                    mouseReleaseEvent(QMouseEvent* event) override;
    void                    keyPressEvent(QKeyEvent* event) override;
    void                    inputMethodEvent(QInputMethodEvent* event) override;
    QVariant                inputMethodQuery(Qt::InputMethodQuery query) const override;
    void                    drawPreview(QPainter* painter) override;
    DrawingToolType         toolType() override;

  protected:
    void cancelDrawing() override;
    void finishDrawing() override;

  private:
    void startEdit(const QPointF& pos);
    bool finishEdit(bool commit);
    void drawTextCursor(QPainter* painter) const;
    void drawPreeditText(QPainter* painter) const;
    bool hitTest(const QPointF& scenePos) const;
    int  cursorPosAtPoint(const QPointF& scenePos) const;

  private:
    QString m_editText;
    int     m_cursorPos = 0;
    QFont   m_font;
    QPointF m_textPos;
    QString m_preeditText;
};
}// namespace xcanvas

#endif// TEXTTOOL_H
