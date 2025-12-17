#ifndef CANVASRENDERER_H
#define CANVASRENDERER_H
#include "Canvas.h"

namespace xcanvas {
    class Canvas;
}

class QPainter;

namespace xcanvas {

class CanvasRenderer {

public:
    void drawBackground(QPainter* painter, const Canvas* canvas, const QRectF& rect);
    void drawForeground(QPainter* painter, const Canvas* canvas, const QRectF& rect);

private:
    void drawNormalShapes(QPainter* painter, const Canvas* canvas, const QRectF& visibleRect);
    void drawSelectedShapes(QPainter* painter, const Canvas* canvas, const QRectF& visibleRect);
    void drawGrid(QPainter* painter, const Canvas* canvas);
    void drawTrace(QPainter* painter, const Canvas* canvas);
    void drawCanvas(QPainter* painter, const Canvas* canvas);
};

} // xcanvas

#endif //CANVASRENDERER_H
