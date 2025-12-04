#include "CurveTool.h"
#include "../MyGraphicsView.h"
#include "Shapes.h"
#include "Curve.h"
#include "Polyline.h"

namespace xcanvas
{

CurveTool::CurveTool(MyGraphicsView* pView) : DrawingTool(pView)
{

}

CurveTool::~CurveTool()
{

}

void CurveTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    m_mousePos = m_pView->mapToScene(event->pos());

    if (m_state == State::Idle)
    {
        m_state = State::Drawing;
        m_points.push_back(m_mousePos);
    }
    else
    {
        m_points.push_back(m_mousePos);
    }
}

void CurveTool::mouseMoveEvent(QMouseEvent *event)
{
    if (m_state == State::Idle)
    {
        return;
    }

    m_mousePos = m_pView->mapToScene(event->pos());

    rebuildPreviewPath();

    m_pView->updateCanvas();

    handleRightButtonMove(event);
}

void CurveTool::mouseReleaseEvent(QMouseEvent *event)
{
	handleRightButtonRelease(event);
}

void CurveTool::cancelDrawing()
{
    if (m_state != State::Interrupted)
    {
        return;
    }

    if(m_points.size() <= 1)
    {
        m_previewPath = QPainterPath();
	}
    else
    {
        Shape* pShape = nullptr;
        if (m_points.size() == 2)
        {
            Polyline* pPolyline = new Polyline;
            pPolyline->SetPoints(m_points);
            pShape = pPolyline;
        }
        else
        {
            QVector<QPointF> bezierPoints = computeBezierPoints(m_points);
            
            Curve* pCurve = new Curve;
            pCurve->SetPoints(bezierPoints);
            pShape = pCurve;
        } 

        pShape->setSelected(true);
        m_pView->GetCurrentShapes()->deselectAll();
        m_pView->GetCurrentShapes()->addShape(pShape);
    }

    m_previewPath = QPainterPath();
    m_points.clear();
    m_state = State::Idle;

    m_pView->updateCanvas();
}

QVector<QPointF> CurveTool::computeBezierPoints(const QVector<QPointF>& anchorPoints) const
{
    QVector<QPointF> result;

    int nSize = anchorPoints.size();
    if (nSize < 2)
    {
        return result;
    }

    struct Segment
    {
        QPointF P0, P1;
        QPointF C1, C2;
    };

    QVector<Segment> segments;

    // 初始控制点（1/3公式）
    for (int i = 0; i < nSize - 1; ++i)
    {
        QPointF P0 = anchorPoints[i];
        QPointF P1 = anchorPoints[i + 1];

        QPointF C1 = P0 + (P1 - P0) / 3.0;
        QPointF C2 = P1 - (P1 - P0) / 3.0;

        segments.append({ P0, P1, C1, C2 });
    }

    // 两段之间联动控制点
    for (int i = 1; i < segments.size(); ++i)
    {
        Segment& prev = segments[i - 1];
        Segment& curr = segments[i];

        QPointF* pC1 = &prev.C2;
        QPointF* pC2 = &curr.C1;
        QPointF* pNode = &prev.P1;

        QPointF mid = (*pC1 + *pC2) / 2.0;
        QPointF delta = *pNode - mid;

        prev.C2 += delta;
        curr.C1 += delta;
    }

    // 展开成 [P, C1, C2, P]
    result.push_back(segments[0].P0);
    for (auto& segment : segments)
    {
        result.push_back(segment.C1);
        result.push_back(segment.C2);
        result.push_back(segment.P1);
    }

    return result;
}

QPainterPath CurveTool::buildCurvePath(const QVector<QPointF>& points) const
{
    QPainterPath path;
    if (points.size() < 4)
    {
        // 不足一段贝塞尔：用折线代替
        if (!points.isEmpty())
        {
            path.moveTo(points[0]);
            for (int i = 1; i < points.size(); ++i)
            {
                path.lineTo(points[i]);
            }
        }
        return path;
    }

    path.moveTo(points[0]);
    for (int i = 0; i <= points.size() - 4; i += 3)
    {
        QPointF c1  = points.at(i + 1);
        QPointF c2  = points.at(i + 2);
        QPointF end = points.at(i + 3);
        path.cubicTo(c1, c2, end);
    }

    return path;
}

void CurveTool::rebuildPreviewPath()
{
    m_previewPath = QPainterPath();

    if (m_state == State::Idle || m_points.isEmpty())
    {
        return;
    }

    // 折线 + 鼠标点 的 anchor 列表
    QVector<QPointF> anchors = m_points;
    anchors.append(m_mousePos);

    // 计算贝塞尔控制点
    QVector<QPointF> bezierPoints = computeBezierPoints(anchors);

    // 得到曲线
    m_previewPath = buildCurvePath(bezierPoints);
}

DrawingToolType CurveTool::toolType()
{
    return DrawingToolType::Curve;
}

} // xcanvas
