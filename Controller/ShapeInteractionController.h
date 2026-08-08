#ifndef SHAPEINTERACTIONCONTROLLER_H
#define SHAPEINTERACTIONCONTROLLER_H

#include <functional>

#include <QPointF>

class ICanvasViewport;
class QMouseEvent;
class TextEditController;

namespace xcanvas
{
class Canvas;
class Shape;
}

class ShapeInteractionController final
{
  public:
    using IsSelectTool = std::function<bool()>;

    ShapeInteractionController(
        ICanvasViewport*     view,
        xcanvas::Canvas*     canvas,
        IsSelectTool         isSelectTool,
        TextEditController*  textEditController);

    bool mouseDoubleClickEvent(QMouseEvent* event);

  private:
    xcanvas::Shape* findTopShapeAtScenePos(const QPointF& scenePos) const;

  private:
    ICanvasViewport*    m_view;
    xcanvas::Canvas*    m_canvas;
    IsSelectTool        m_isSelectTool;
    TextEditController* m_textEditController;
};

#endif// SHAPEINTERACTIONCONTROLLER_H
