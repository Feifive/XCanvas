#include "EllipseTool.h"
#include "../MyGraphicsView.h"
#include "Canvas.h"
#include "Global.h"
#include <QGraphicsEllipseItem>
#include <QMouseEvent>

xcanvas::EllipseTool::EllipseTool(MyGraphicsView* view, Canvas* canvas) : DrawingTool(view, canvas)
{
}

xcanvas::EllipseTool::~EllipseTool()
{
}

void xcanvas::EllipseTool::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonPress(event);
        return;
    }

    if (m_state == State::Idle)
    {
        m_mousePos    = m_canvasView->mapToScene(event->pos());
        m_previewPath = QPainterPath();
        m_state       = State::Drawing;
    }
}

void xcanvas::EllipseTool::mouseMoveEvent(QMouseEvent* event)
{
    if (m_state == State::Drawing)
    {
        QPointF currentPos = m_canvasView->mapToScene(event->pos());

        QRectF rect(qMin(m_mousePos.x(), currentPos.x()), qMin(m_mousePos.y(), currentPos.y()), qAbs(currentPos.x() - m_mousePos.x()), qAbs(currentPos.y() - m_mousePos.y()));

        m_previewPath = QPainterPath();
        m_previewPath.addEllipse(rect);

        m_canvasView->requestFullUpdate();
    }

    handleRightButtonMove(event);
}

void xcanvas::EllipseTool::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        handleRightButtonRelease(event);
        return;
    }

    if (m_state == State::Drawing)
    {
        const QRectF rect = m_previewPath.boundingRect();
        auto* shape = new Vector();
        if (qFuzzyCompare(rect.width(), rect.height())) {
            shape->setSemantic(VectorSemantic::Circle);
        }
        else {
            shape->setSemantic(VectorSemantic::Ellipse);
        }

        shape->segments() = buildEllipseSegments(rect);
        shape->setSelected(true);

        m_canvas->shapeManager()->deselectAll();
        m_canvas->addShape(shape);

        cancelDrawing();
    }
}

DrawingToolType xcanvas::EllipseTool::toolType()
{
    return DrawingToolType::Ellipse;
}

QVector<xcanvas::Segment> xcanvas::EllipseTool::buildEllipseSegments(const QRectF &rect) {
    QVector<Segment> segments;

    const QPointF c = rect.center();
    const double rx = rect.width()  * 0.5;
    const double ry = rect.height() * 0.5;

    const double ox = rx * KAPPA;
    const double oy = ry * KAPPA;

    // 四个端点（顺时针）
    QPointF p0(c.x() + rx, c.y()); // 右
    QPointF p1(c.x(),      c.y() + ry); // 下
    QPointF p2(c.x() - rx, c.y()); // 左
    QPointF p3(c.x(),      c.y() - ry); // 上

    segments.reserve(5);

    // 起点
    segments.append(Segment::moveTo(p0));

    // 右 → 下
    segments.append(Segment::cubicTo(
        QPointF(p0.x(),      p0.y() + oy),
        QPointF(p1.x() + ox, p1.y()),
        p1));

    // 下 → 左
    segments.append(Segment::cubicTo(
        QPointF(p1.x() - ox, p1.y()),
        QPointF(p2.x(),      p2.y() + oy),
        p2));

    // 左 → 上
    segments.append(Segment::cubicTo(
        QPointF(p2.x(),      p2.y() - oy),
        QPointF(p3.x() - ox, p3.y()),
        p3));

    // 上 → 右
    segments.append(Segment::cubicTo(
        QPointF(p3.x() + ox, p3.y()),
        QPointF(p0.x(),      p0.y() - oy),
        p0));

    return segments;
}
