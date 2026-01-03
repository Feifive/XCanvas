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

private:
    MyGraphicsView* m_canvasView;
    Canvas*         m_canvas;
};
}




#endif //SHAPEEDITOR_H
