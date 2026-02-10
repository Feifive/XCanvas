#ifndef SHAPEEDITOR_H
#define SHAPEEDITOR_H
#include "Canvas.h"

class MyGraphicsView;
namespace xcanvas {
    class Canvas;
}

namespace xcanvas {
class ShapeEditor {
public:
    explicit ShapeEditor(MyGraphicsView* view, Canvas* canvas);
    ~ShapeEditor() = default;

    bool booleanUnion() const;
    bool booleanIntersection() const;
    bool booleanSubtractAB() const;
    bool booleanSubtractBA() const;
    bool mirrorHorizontal() const;
    bool mirrorVertical() const;
    bool alignLeft() const;
    bool alignRight() const;
    bool alignTop() const;
    bool alignBottom() const;
    bool alignHorizontalCenter() const;
    bool alignVerticalCenter() const;
    bool alignCenter() const;

private:
    MyGraphicsView* m_canvasView;
    Canvas*         m_canvas;
};
}




#endif //SHAPEEDITOR_H
