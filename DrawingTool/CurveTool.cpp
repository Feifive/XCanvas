#include "CurveTool.h"
#include "../MyGraphicsView.h"

namespace xcanvas
{

CurveTool::CurveTool(MyGraphicsView *pView) : DrawingTool(pView)
{

}

CurveTool::~CurveTool()
{

}

void CurveTool::mousePressEvent(QMouseEvent *event)
{
    DrawingTool::mousePressEvent(event);
}

void CurveTool::mouseMoveEvent(QMouseEvent *event)
{
    DrawingTool::mouseMoveEvent(event);
}

void CurveTool::mouseReleaseEvent(QMouseEvent *event)
{
    DrawingTool::mouseReleaseEvent(event);
}

DrawingToolType CurveTool::toolType()
{
    return DrawingToolType::Curve;
}

} // xcanvas
