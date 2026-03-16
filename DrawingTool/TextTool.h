#ifndef TEXTTOOL_H
#define TEXTTOOL_H

#include "DrawingTool.h"
#include <QFont>

class QGraphicsTextItem;

namespace xcanvas
{
class TextTool : public DrawingTool
{
    Q_OBJECT

  public:
    explicit TextTool(MyGraphicsView* view, Canvas* canvas);
    virtual ~TextTool() override;

    virtual void            mousePressEvent(QMouseEvent* event) override;
    virtual void            mouseMoveEvent(QMouseEvent* event) override;
    virtual void            mouseReleaseEvent(QMouseEvent* event) override;
    virtual DrawingToolType toolType() override;

  protected:
    void cancelDrawing() override;
    void finishDrawing() override;

  private:
    bool finishEdit(bool commit);
    void startEdit();

  private:
    QGraphicsTextItem* m_pTextItem;
    QFont              m_font;
};
}// namespace xcanvas

#endif// TEXTTOOL_H
