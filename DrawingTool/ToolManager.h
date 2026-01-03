#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include "DrawingTool.h"
#include "ShapeEditor.h"
#include  <QObject>

class MyGraphicsView;
namespace xcanvas {
    class Canvas;
}

namespace xcanvas {
class ToolManager final : public QObject
{
    Q_OBJECT
public:
    explicit ToolManager(MyGraphicsView* view, Canvas* canvas);
    ~ToolManager() override = default;
    void setTool(DrawingToolType type);
    DrawingToolType currentTool() const { return m_currentType; }
    DrawingTool* currentToolInstance() const { return m_currentTool.get(); }

    bool booleanUnion();

    // 事件转发
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);

    // 绘制预览
    void drawPreview(QPainter* painter);

signals:
    void toolChanged(DrawingToolType type);

private:
    std::unique_ptr<DrawingTool> createTool(DrawingToolType type);

private:
    MyGraphicsView* m_view;
    Canvas* m_canvas;
    std::unique_ptr<DrawingTool> m_currentTool;
    std::unique_ptr<ShapeEditor> m_shapeEditor;
    DrawingToolType m_currentType { DrawingToolType::None };
};

} // xcanvas

#endif //TOOLMANAGER_H
