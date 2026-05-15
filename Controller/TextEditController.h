#ifndef TEXTEDITCONTROLLER_H
#define TEXTEDITCONTROLLER_H

#include <functional>
#include <QPointF>
#include <QString>

class QInputMethodEvent;
class QKeyEvent;
class QPainter;
class QGraphicsView;
class ViewRenderController;

namespace xcanvas
{
class Canvas;
class ShapeText;
}

class TextEditController final
{
  public:
    using UpdateUiAction = std::function<void()>;

    TextEditController(
        QGraphicsView*         view,
        xcanvas::Canvas*       canvas,
        ViewRenderController*  viewRenderController,
        UpdateUiAction         updateSelectionHud,
        UpdateUiAction         requestFullUpdate);

    bool      isEditing() const;
    bool      beginInlineEdit(xcanvas::ShapeText* shape, const QPointF& scenePos);
    void      finishInlineEdit(bool commit);
    bool      moveCursorToScenePos(const QPointF& scenePos);

    void      keyPressEvent(QKeyEvent* event);
    void      inputMethodEvent(QInputMethodEvent* event);
    QVariant  inputMethodQuery(Qt::InputMethodQuery query) const;
    void      drawPreview(QPainter* painter);

  private:
    void drawCursor(QPainter* painter) const;
    void drawPreedit(QPainter* painter) const;
    int  cursorPosAtLocalPos(const QPointF& localPos) const;

  private:
    QGraphicsView*        m_view;
    xcanvas::Canvas*      m_canvas;
    ViewRenderController* m_viewRenderController;
    UpdateUiAction        m_updateSelectionHud;
    UpdateUiAction        m_requestFullUpdate;

    xcanvas::ShapeText* m_inlineEditingShape = nullptr;
    QString             m_inlineOriginalText;
    QString             m_editText;
    int                 m_cursorPos = 0;
    QString             m_preeditText;
};

#endif// TEXTEDITCONTROLLER_H
