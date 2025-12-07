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
#include "BottomFloatingToolBar.h"
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>

#define MIN_ZOOM 0.05
#define MAX_ZOOM 100.0

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : m_dScaleFactor(1.0), m_eToolType(DrawingToolType::None), 
    m_startPos(-1, -1), m_bDragging(false), m_pBaseDrawingTool(nullptr), m_pShapes(new xcanvas::Shapes), QGraphicsView{parent}
    ,m_pFloatingToolBar(nullptr), m_CanvasRect(QRectF(10000, 10000, 1280, 720))
{
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 工具栏挂在 viewport 上，这样就是画布上的浮动控件
    m_pFloatingToolBar = new BottomFloatingToolBar(viewport());
    m_pFloatingToolBar->adjustSize();  // 先根据布局算下自己的尺寸

    connect(m_pFloatingToolBar, &BottomFloatingToolBar::zoomIn, this, &MyGraphicsView::zoomIn);
    connect(m_pFloatingToolBar, &BottomFloatingToolBar::zoomOut, this, &MyGraphicsView::zoomOut);
    connect(m_pFloatingToolBar, &BottomFloatingToolBar::zoomTo, this, &MyGraphicsView::zoomTo);
    connect(m_pFloatingToolBar, &BottomFloatingToolBar::fitWidth, this, &MyGraphicsView::fitWidth);
    connect(m_pFloatingToolBar, &BottomFloatingToolBar::fitHeight, this, &MyGraphicsView::fitHeight);
    connect(m_pFloatingToolBar, &BottomFloatingToolBar::fitCanvas, this, &MyGraphicsView::fitCanvas);
    connect(m_pFloatingToolBar, &BottomFloatingToolBar::fitShapes, this, &MyGraphicsView::fitShapes);
    connect(&EventBus::instance(), &EventBus::switchTool, this, &MyGraphicsView::setTool);

    QTimer::singleShot(0, this, [this](){ fitCanvas();});
    updateBottomFloatingToolBarPos();
    m_pFloatingToolBar->show();
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
    if (event->angleDelta().y() > 0) {
        zoomIn();
    }
    else {
        zoomOut();
    }
}

void MyGraphicsView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    updateBottomFloatingToolBarPos();
}
void MyGraphicsView::scrollContentsBy(int dx, int dy) {
    QGraphicsView::scrollContentsBy(dx, dy);

    updateBottomFloatingToolBarPos();
}

void MyGraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
{
    drawCanvas(painter);
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

    // 计算画布在场景坐标中的可见部分
    const QRectF canvasSceneRect   = m_CanvasRect;                                   // 画布本身（场景坐标）
    const QRectF viewSceneRect     = mapToScene(viewport()->rect()).boundingRect();  // 视口对应的场景区域
    const QRectF visibleSceneRect  = canvasSceneRect.intersected(viewSceneRect);     // 画布中可见的部分

    if (!visibleSceneRect.isValid() || visibleSceneRect.isEmpty())
    {
        return;
    }

    const double scale      = this->scale();
    const double step       = gridStep(scale);   // 网格步长（场景单位）
    const int    majorCount = 10;

    const QColor minorColor(230, 230, 230);
    const QColor majorColor(200, 200, 200);

    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);

    // 使用视口坐标绘制网格线
    const QTransform oldWorldTransform = p->worldTransform();
    p->setWorldTransform(QTransform()); // 切换到视口坐标

    // 画布在视口中的矩形（只画这里）
    const QRect canvasViewRect = mapFromScene(visibleSceneRect).boundingRect();

    // 限制绘制范围到画布区域
    p->setClipRect(canvasViewRect);

    // 竖直方向网格线（X）
    const double s0x = visibleSceneRect.left();
    const double s1x = visibleSceneRect.right();

    qint64 firstIndexX = qFloor((s0x + 1e-12) / step);
    double x0          = firstIndexX * step;

    QVector<QLineF> minorLines;
    QVector<QLineF> majorLines;

    for (double x = x0; x <= s1x + step; x += step, ++firstIndexX)
    {
        // 把场景中的 (x, 任意y) 映射到视口
        QPointF v = mapFromScene(QPointF(x, visibleSceneRect.top()));
        double vx = std::round(v.x()) - 0.5;

        if ((firstIndexX % majorCount) == 0)
        {
            majorLines.append(QLineF(vx, canvasViewRect.top(), vx, canvasViewRect.bottom()));
        }
        else
        {
            minorLines.append(QLineF(vx, canvasViewRect.top(), vx, canvasViewRect.bottom()));
        }
    }

    // 水平方向网格线（Y）
    const double s0y = visibleSceneRect.top();
    const double s1y = visibleSceneRect.bottom();

    qint64 firstIndexY = qFloor((s0y + 1e-12) / step);
    double y0          = firstIndexY * step;

    for (double y = y0; y <= s1y + step; y += step, ++firstIndexY)
    {
        QPointF v = mapFromScene(QPointF(visibleSceneRect.left(), y));
        double vy = std::round(v.y()) - 0.5;

        if ((firstIndexY % majorCount) == 0)
        {
            majorLines.append(QLineF(canvasViewRect.left(), vy, canvasViewRect.right(), vy));
        }
        else
        {
            minorLines.append(QLineF(canvasViewRect.left(), vy, canvasViewRect.right(), vy));
        }
    }

    // 画线
    p->setPen(QPen(minorColor, 0));
    p->drawLines(minorLines);

    p->setPen(QPen(majorColor, 0));
    p->drawLines(majorLines);

    // 还原状态
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

void MyGraphicsView::drawCanvas(QPainter *painter) {
    painter->save();

    const QTransform oldWorldTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    painter->fillRect(rect(), QColor("#E7E9ED"));
    painter->setWorldTransform(oldWorldTransform);

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawRect(m_CanvasRect);

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

void MyGraphicsView::updateBottomFloatingToolBarPos() {
    if (!m_pFloatingToolBar)
        return;

    constexpr int margin = 12; // 距离右下角的间距
    const QSize vpSize = viewport()->size();
    const QSize barSize = m_pFloatingToolBar->sizeHint();

    int x = vpSize.width()  - barSize.width()  - margin;
    int y = vpSize.height() - barSize.height() - margin;

    m_pFloatingToolBar->move(x, y);
}

void MyGraphicsView::zoomIn()
{
    QPointF cursorViewPos = mapFromGlobal(QCursor::pos());

    if (m_pFloatingToolBar) {
        if (m_pFloatingToolBar->rect().contains(m_pFloatingToolBar->mapFromGlobal(QCursor::pos()))) {
            cursorViewPos = rect().center();
        }
    }

    const QPointF cursorScenePosBeforeScale = mapToScene(cursorViewPos.toPoint());
    constexpr double dScale = 1.1;
    if (m_dScaleFactor == MAX_ZOOM) {
        return;
    }

    m_dScaleFactor *= dScale;
    m_dScaleFactor = qBound(MIN_ZOOM, m_dScaleFactor, MAX_ZOOM);

    QTransform transform;
    transform.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transform);

    const QPointF cursorScenePos = mapToScene(cursorViewPos.toPoint());
    const QPointF viewCenter     = mapToScene(viewport()->rect().center());
    const QPointF adjustedCenter = viewCenter + (cursorScenePosBeforeScale - cursorScenePos);

    centerOn(adjustedCenter);
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::zoomOut()
{
    QPointF cursorViewPos = mapFromGlobal(QCursor::pos());

    if (m_pFloatingToolBar) {
        if (m_pFloatingToolBar->rect().contains(m_pFloatingToolBar->mapFromGlobal(QCursor::pos()))) {
            cursorViewPos = rect().center();
        }
    }

    const QPointF cursorScenePosBeforeScale = mapToScene(cursorViewPos.toPoint());
    constexpr double dScale = 1.0 / 1.1;

    if (m_dScaleFactor == MIN_ZOOM) {
        return;
    }

    m_dScaleFactor *= dScale;
    m_dScaleFactor = qBound(MIN_ZOOM, m_dScaleFactor, MAX_ZOOM);

    QTransform transform;
    transform.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transform);

    const QPointF cursorScenePos = mapToScene(cursorViewPos.toPoint());
    const QPointF viewCenter     = mapToScene(viewport()->rect().center());
    const QPointF adjustedCenter = viewCenter + (cursorScenePosBeforeScale - cursorScenePos);

    centerOn(adjustedCenter);
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::zoomTo(qreal zoomValue) {
    if (zoomValue <= 0) {
        return;
    }

    qreal targetScale = zoomValue;

    targetScale = qBound(MIN_ZOOM, targetScale, MAX_ZOOM);

    QPointF cursorViewPos = viewport()->rect().center();
    QPointF scenePosBefore = mapToScene(cursorViewPos.toPoint());

    m_dScaleFactor = targetScale;

    QTransform transform;
    transform.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transform);

    QPointF scenePosAfter = mapToScene(cursorViewPos.toPoint());
    QPointF viewCenter = mapToScene(viewport()->rect().center());
    QPointF adjustedCenter = viewCenter + (scenePosBefore - scenePosAfter);

    centerOn(adjustedCenter);
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitWidth()
{
    qreal scale = viewport()->width() / m_CanvasRect.width();
    zoomTo(scale);
    centerOn(m_CanvasRect.center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitHeight()
{
    qreal scale = viewport()->height() / m_CanvasRect.height();
    zoomTo(scale);
    centerOn(m_CanvasRect.center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitCanvas()
{
    qreal scaleW = viewport()->width()  / m_CanvasRect.width();
    qreal scaleH = viewport()->height() / m_CanvasRect.height();
    qreal scale  = qMin(scaleW, scaleH);
    zoomTo(scale);
    centerOn(m_CanvasRect.center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitShapes() {
    if (!m_pShapes || m_pShapes->isEmpty()) {
        return;
    }

    QRectF rect = m_pShapes->boundingRect();

    qreal scaleW = viewport()->width() / rect.width();
    qreal scaleH = viewport()->height() / rect.height();
    qreal scale  = qMin(scaleW, scaleH);
    zoomTo(scale);
    centerOn(rect.center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}
