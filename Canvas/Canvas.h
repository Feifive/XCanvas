#ifndef CANVAS_H
#define CANVAS_H

namespace xcanvas {
    class ShapeManager;
}
class QUndoStack;

#include <QObject>
#include <QRectF>

namespace xcanvas {

class Canvas final : public QObject {
    Q_OBJECT
public:
    explicit Canvas(QObject* parent = nullptr);
    ~Canvas() override;

    QRectF canvasRect() const;
    void   setCanvasRect(const QRectF& rect);

    ShapeManager* shapeManager() const;
    QUndoStack* undoStack() const;

private:
    QRectF m_canvasRect;
    ShapeManager* m_shapeManager;
    QUndoStack* m_undoStack;
};

} // xcanvas

#endif //CANVAS_H
