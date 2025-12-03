#ifndef CURVETOOL_H
#define CURVETOOL_H

#include "DrawingTool.h"

class MyGraphicsView;

namespace xcanvas
{
    class CurveTool : public DrawingTool
    {
        Q_OBJECT
    public:
        explicit CurveTool(MyGraphicsView* pView);
        ~CurveTool() override;

        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        DrawingToolType toolType() override;
    };

} // xcanvas

#endif //CURVETOOL_H
