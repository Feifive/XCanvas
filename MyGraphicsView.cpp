#include "MyGraphicsView.h"
#include "DXFTranslator.h"
#include "DrawingTool/DrawingTool.h"
#include "DrawingTool/EllipseTool.h"
#include "DrawingTool/PolylineTool.h"
#include "DrawingTool/RectTool.h"
#include "DrawingTool/SelectTool.h"
#include "DrawingTool/TextTool.h"
#include "DrawingTool/CurveTool.h"
#include "Shape/Shape.h"
#include "Shape/Shapes.h"
#include "EventBus.h"
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

#define MIN_ZOOM 0.05
#define MAX_ZOOM 100.0

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : m_dScaleFactor(1.0), m_eToolType(DrawingToolType::None), 
    m_startPos(-1, -1), m_bDragging(false), m_pBaseDrawingTool(nullptr), m_pShapes(new xcanvas::Shapes), QGraphicsView{parent}
{
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    connect(&EventBus::instance(), &EventBus::switchTool, this, &MyGraphicsView::setTool);
}

MyGraphicsView::~MyGraphicsView()
{
    if (m_pBaseDrawingTool)
    {
        delete m_pBaseDrawingTool;
        m_pBaseDrawingTool = nullptr;
    }
}

void MyGraphicsView::setTool(DrawingToolType type)
{
    switch (type)
    {
    case DrawingToolType::Import:
        ImportFile();
        return;
    default:
        break;
    }

    if (m_pBaseDrawingTool)
    {
        delete m_pBaseDrawingTool;
        m_pBaseDrawingTool = nullptr;
    }

    switch (type)
    {
    case DrawingToolType::Select:
    {
        m_pBaseDrawingTool = new xcanvas::SelectTool(this);
    }
    break;
    case DrawingToolType::Text:
    {
        m_pBaseDrawingTool = new xcanvas::TextTool(this);
    }
    break;
    case DrawingToolType::Rect:
    {
        m_pBaseDrawingTool = new xcanvas::RectTool(this);
    }
    break;
    case DrawingToolType::Ellipse:
    {
        m_pBaseDrawingTool = new xcanvas::EllipseTool(this);
    }
    break;
    case DrawingToolType::Polyline:
    {
        m_pBaseDrawingTool = new xcanvas::PolylineTool(this);
    }
    break;
    case DrawingToolType::Curve:
    {
        m_pBaseDrawingTool = new xcanvas::CurveTool(this);
    }
    break;
    default:
        m_pBaseDrawingTool = nullptr;
        break;
    }
    m_eToolType = type;

    updateCanvas();
}

xcanvas::Shapes* MyGraphicsView::GetCurrentShapes()
{
    return m_pShapes;
}

double MyGraphicsView::scale()
{
    return transform().m11();
}

void MyGraphicsView::updateCanvas()
{
    viewport()->update();
}

void MyGraphicsView::updateShape(xcanvas::Shape* shape)
{
}

void MyGraphicsView::updateSelectedShapes()
{
}

void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        // 记录初始鼠标位置并进入拖动状态
        m_startPos  = event->pos();
        m_bDragging = true;
        setCursor(Qt::ClosedHandCursor);// 设置为闭合手型光标
        event->accept();// 标记事件已处理
    }

    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->mousePressEvent(event);
    }
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_bDragging)
    {
        // 计算鼠标移动的增量
        QPoint delta = event->pos() - m_startPos.toPoint();

        // 水平滚动条：反向移动增量（因为视图移动方向与鼠标相反）
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());

        // 垂直滚动条：同上
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        // 更新记录的最后鼠标位置
        m_startPos = event->pos();
        event->accept();
    }

    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->mouseMoveEvent(event);
    }

    emit mouseMovePos(event->pos());
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton && m_bDragging)
    {
        m_bDragging = false;
        setCursor(Qt::ArrowCursor);// 恢复默认光标
        event->accept();
    }

    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->mouseReleaseEvent(event);
    }
}

void MyGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->keyPressEvent(event);
    }

    QGraphicsView::keyPressEvent(event);
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    // 获取当前光标在视图中的坐标
    const QPointF cursorViewPos             = event->position();
    const QPointF cursorScenePosBeforeScale = mapToScene(cursorViewPos.toPoint());

    // 计算缩放因子（根据滚轮方向调整）
    const double dScale = event->angleDelta().y() > 0 ? 1.1 : 1 / 1.1;

    // 计算新缩放因子并应用缩放
    if ((m_dScaleFactor == MAX_ZOOM && dScale == 1.1) || (m_dScaleFactor == MIN_ZOOM && dScale == 1 / 1.1))
    {
        return;
    }
    m_dScaleFactor *= dScale;
    m_dScaleFactor = qBound(MIN_ZOOM, m_dScaleFactor, MAX_ZOOM);
    QTransform transform;
    transform.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transform);

    // 计算缩放后的视图中心调整
    const QPointF cursorScenePos = mapToScene(cursorViewPos.toPoint());
    const QPointF viewCenter     = mapToScene(viewport()->rect().center());
    const QPointF adjustedCenter = viewCenter + (cursorScenePosBeforeScale - cursorScenePos);

    // 更新视图中心
    centerOn(adjustedCenter);

    updateCanvas();
    emit transformChanged();
}

void MyGraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
{
    drawGrid(painter);

    drawNormalShapes(painter, rect);
}

void MyGraphicsView::drawForeground(QPainter* painter, const QRectF& rect)
{
    drawSelectedShapes(painter, rect);
    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->drawPreview(painter);
    }
}

void MyGraphicsView::drawShapes(QPainter* painter, const QRectF& visibleRect)
{
    drawNormalShapes(painter, visibleRect);
    drawSelectedShapes(painter, visibleRect);
}

void MyGraphicsView::drawNormalShapes(QPainter* painter, const QRectF& visibleRect)
{
    painter->save();

    // 遍历所有形状，只绘制未选中的
    for (xcanvas::Shape* shape : m_pShapes->shapes())
    {
        if (shape->isSelected()) 
        {
            continue;
        }

        // 视口裁剪：只绘制可见的形状
        if (!visibleRect.intersects(shape->boundingRect())) 
        {
            continue;
        }

        // 绘制形状
        shape->draw(painter);
    }

    painter->restore();
}

void MyGraphicsView::drawSelectedShapes(QPainter* painter, const QRectF& visibleRect)
{
    painter->save();

    // 最后绘制选中的形状（显示在最上层）
    QVector<xcanvas::Shape*> selected = m_pShapes->selectedShapes();

    for (xcanvas::Shape* shape : selected) 
    {
        if (!visibleRect.intersects(shape->boundingRect())) 
        {
            continue;
        }

        shape->draw(painter);
    }

    // 绘制trace
    if (!selected.isEmpty()) 
    {
		drawTrace(painter);
    }

    painter->restore();
}

void MyGraphicsView::drawGrid(QPainter *p)
{
    if (!scene())
    {
        return;
    }

    const QRect  viewRect   = viewport()->rect();
    const QRectF sceneRect  = mapToScene(viewRect).boundingRect();
    const double scale      = this->scale();
    const double step       = gridStep(scale);
    const int    majorCount = 10;

    const QColor minorColor(230, 230, 230);
    const QColor majorColor(200, 200, 200);
    const QColor bgColor(Qt::white);

    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);

    QTransform oldWorldTransform = p->worldTransform();
    p->setWorldTransform(QTransform());

    p->fillRect(viewRect, bgColor);

    double s0 = sceneRect.left();
    double s1 = sceneRect.right();

    qint64 firstIndex = qFloor((s0 + 1e-12) / step);
    double x0         = firstIndex * step;

    QVector<QLineF> minorLines, majorLines;

    // 竖线
    for (double x = x0; x <= s1 + step; x += step, firstIndex++)
    {
        QPointF v = mapFromScene(QPointF(x, 0));
        double vx = std::round(v.x()) - 0.5;

        if ((firstIndex % majorCount) == 0)
        {
            majorLines.append(QLineF(vx, viewRect.top(), vx, viewRect.bottom()));
        }
        else
        {
            minorLines.append(QLineF(vx, viewRect.top(), vx, viewRect.bottom()));
        }
    }

    // 横线
    firstIndex = qFloor((sceneRect.top() + 1e-12) / step);
    double y0 = firstIndex * step;

    for (double y = y0; y <= sceneRect.bottom() + step; y += step, firstIndex++)
    {
        QPointF v = mapFromScene(QPointF(0, y));
        double vy = std::round(v.y()) - 0.5;

        if ((firstIndex % majorCount) == 0)
        {
            majorLines.append(QLineF(viewRect.left(), vy, viewRect.right(), vy));
        }
        else
        {
            minorLines.append(QLineF(viewRect.left(), vy, viewRect.right(), vy));
        }
    }

    p->setPen(QPen(minorColor, 0));
    p->drawLines(minorLines);

    p->setPen(QPen(majorColor, 0));
    p->drawLines(majorLines);

    p->setWorldTransform(oldWorldTransform);
    p->restore();
}

double MyGraphicsView::gridStep(double scale) const
{
    static const double steps[] = {0.1, 0.5, 1.0, 5.0,10.0, 50.0, 100.0, 500.0};

    const double minPixelSpacing = 10.0;

    for (double step : steps)
    {
        if (step * scale >= minPixelSpacing)
        {
            return step;
        }
    }

    return 500.0;
}

void MyGraphicsView::traceRects(const QRectF& rect, QRectF rects[9])
{
#define SIZE 3
    double dScale = scale();
    double dRectSize = SIZE / dScale;
    double dRectWidth = dRectSize * 2;

    rects[ERECT_TOP_LEFT] = QRectF(rect.left() - dRectWidth, rect.top() - dRectWidth, dRectWidth, dRectWidth);
    rects[ERECT_TOP_MID] = QRectF(rect.center().x() - dRectSize, rect.top() - dRectWidth, dRectWidth, dRectWidth);
    rects[ERECT_TOP_RIGHT] = QRectF(rect.right(), rect.top() - dRectWidth, dRectWidth, dRectWidth);
    rects[ERECT_MID_LEFT] = QRectF(rect.left() - dRectWidth, rect.center().y() - dRectSize, dRectWidth, dRectWidth);
    rects[ERECT_CENTER] = QRectF(rect.center().x() - dRectSize, rect.center().y() - dRectSize, dRectWidth, dRectWidth);
    rects[ERECT_MID_RIGHT] = QRectF(rect.right(), rect.center().y() - dRectSize, dRectWidth, dRectWidth);
    rects[ERECT_BOTTOM_LEFT] = QRectF(rect.left() - dRectWidth, rect.bottom(), dRectWidth, dRectWidth);
    rects[ERECT_BOTTOM_MID] = QRectF(rect.center().x() - dRectSize, rect.bottom(), dRectWidth, dRectWidth);
    rects[ERECT_BOTTOM_RIGHT] = QRectF(rect.right(), rect.bottom(), dRectWidth, dRectWidth);
}

void MyGraphicsView::drawTrace(QPainter* painter)
{
    if (!m_pShapes || m_pShapes->isEmpty())
    {
        return;
    }

    QRectF rect = m_pShapes->selectedBoundingRect();
    if (!rect.isValid())
    {
        return;
    }

    double dScale = scale();
    double dLineLength = 6 / dScale;

    painter->save();

    QPen pen(Qt::black);
    pen.setWidthF(2 / dScale);
    pen.setCosmetic(false);
    painter->setPen(pen);
    painter->setBrush(Qt::black);

    QRectF rects[9];
    traceRects(rect, rects);

    for (int i = 0; i < 9; ++i)
    {
        if (i == ERECT_CENTER)
        {
            continue;
        }
        painter->drawRect(rects[i]);
    }

    QPointF center = rect.center();

    painter->drawLine(QPointF(center.x() - dLineLength, center.y() - dLineLength),
        QPointF(center.x() + dLineLength, center.y() + dLineLength));

    painter->drawLine(QPointF(center.x() + dLineLength, center.y() - dLineLength),
        QPointF(center.x() - dLineLength, center.y() + dLineLength));

    painter->restore();
}

void MyGraphicsView::ImportFile()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Import File"), "", tr("All supported (*.dxf)"));
    if (filePath.isEmpty())
    {
        return;
    }

    DXFTranslator translator;
    xcanvas::Shapes* pShapes = new xcanvas::Shapes;
    translator.Load(filePath, pShapes);
    pShapes->selectAll();
    m_pShapes->deselectAll();
	m_pShapes->append(pShapes->shapes());
    QRectF rect = pShapes->selectedBoundingRect();
    QPointF viewCenter = mapToScene(viewport()->rect().center());
    QPointF offset = viewCenter - rect.center();
    pShapes->translate(offset);
    
    delete pShapes;
    pShapes = nullptr;

    updateCanvas();
}
