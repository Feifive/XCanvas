#ifndef TEXTEDITCONTROLLER_H
#define TEXTEDITCONTROLLER_H

#include <functional>

#include <QString>

class QObject;
class QEvent;
class QGraphicsScene;
class QGraphicsTextItem;
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

    bool isEditing() const;
    bool eventFilter(QObject* watched, QEvent* event);
    bool beginInlineEdit(xcanvas::ShapeText* shape);
    void finishInlineEdit(bool commit);

  private:
    QGraphicsView*        m_view;
    xcanvas::Canvas*      m_canvas;
    ViewRenderController* m_viewRenderController;
    UpdateUiAction        m_updateSelectionHud;
    UpdateUiAction        m_requestFullUpdate;
    QGraphicsTextItem*    m_inlineTextEditor = nullptr;
    xcanvas::ShapeText*   m_inlineEditingShape = nullptr;
    QString               m_inlineOriginalText;
};

#endif// TEXTEDITCONTROLLER_H
