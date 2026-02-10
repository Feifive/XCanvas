#include "MyGraphicsView.h"
#include "AppSettings.h"
#include "BottomFloatingToolBar.h"
#include "Canvas/Canvas.h"
#include "ColorPaletteWidget.h"
#include "EventBus.h"
#include "Import/DXF/DXFImporter.h"
#include "Import/Image/ImageImporter.h"
#include "Import/ImportManager.h"
#include "Import/PDF/PDFImporter.h"
#include "SelectionHudBar.h"
#include "Shape/Shape.h"
#include "ToolManager.h"
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QUndoStack>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

#include "MyMath.h"

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : m_dScaleFactor(1.0), m_startPos(-1, -1), m_bDragging(false), m_canvas(new xcanvas::Canvas(this)), QGraphicsView{parent}, m_bottomFloatingToolBar(nullptr)
{
    setObjectName("MyGraphicsView");
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);

    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAcceptDrops(true);

    connect(&AppSettings::instance(), &AppSettings::gridContrastChanged, this, [this]() { requestFullUpdate(); });
    connect(m_canvas->layerManager(), &xcanvas::LayerManager::layerVisibilityChanged, this, [this]() { requestFullUpdate(); });

    ImportManager::instance().registerImporter(std::make_unique<DXFImporter>());
    ImportManager::instance().registerImporter(std::make_unique<ImageImporter>());
    ImportManager::instance().registerImporter(std::make_unique<PDFImporter>());

    connect(m_canvas->shapeManager(), &xcanvas::ShapeManager::selectionChanged, this, &MyGraphicsView::onSelectionChanged);

    m_toolMgr = std::make_unique<xcanvas::ToolManager>(this, m_canvas);
    connect(&EventBus::instance(), &EventBus::switchTool, m_toolMgr.get(), &xcanvas::ToolManager::setTool);

    m_colorPaletteWidget = new ColorPaletteWidget(this);
    m_colorPaletteWidget->adjustSize();

    m_bottomFloatingToolBar = new BottomFloatingToolBar(this);
    m_bottomFloatingToolBar->adjustSize();

    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomIn, this, &MyGraphicsView::onZoomIn);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomOut, this, &MyGraphicsView::onZoomOut);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomTo, this, &MyGraphicsView::zoomTo);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitWidth, this, &MyGraphicsView::fitWidth);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitHeight, this, &MyGraphicsView::fitHeight);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitCanvas, this, &MyGraphicsView::fitCanvas);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitShapes, this, &MyGraphicsView::fitShapes);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::undo, this, &MyGraphicsView::onUndo);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::redo, this, &MyGraphicsView::onRedo);
    connect(m_canvas->undoStack(), &QUndoStack::canUndoChanged, m_bottomFloatingToolBar, &BottomFloatingToolBar::setCanUndo);
    connect(m_canvas->undoStack(), &QUndoStack::canRedoChanged, m_bottomFloatingToolBar, &BottomFloatingToolBar::setCanRedo);
    connect(&EventBus::instance(), &EventBus::importFileRequested, this, &MyGraphicsView::importFile);

    m_selectionHudBar = new SelectionHudBar(this);
    connect(m_selectionHudBar, &SelectionHudBar::booleanUnion, m_toolMgr.get(), &xcanvas::ToolManager::booleanUnion);
    connect(m_selectionHudBar, &SelectionHudBar::booleanIntersection, m_toolMgr.get(), &xcanvas::ToolManager::booleanIntersection);
    connect(m_selectionHudBar, &SelectionHudBar::booleanSubtractAB, m_toolMgr.get(), &xcanvas::ToolManager::booleanSubtractAB);
    connect(m_selectionHudBar, &SelectionHudBar::booleanSubtractBA, m_toolMgr.get(), &xcanvas::ToolManager::booleanSubtractBA);
    connect(m_selectionHudBar, &SelectionHudBar::mirrorHorizontal, m_toolMgr.get(), &xcanvas::ToolManager::mirrorHorizontal);
    connect(m_selectionHudBar, &SelectionHudBar::mirrorVertical, m_toolMgr.get(), &xcanvas::ToolManager::mirrorVertical);
    connect(m_selectionHudBar, &SelectionHudBar::alignLeft, m_toolMgr.get(), &xcanvas::ToolManager::alignLeft);
    connect(m_selectionHudBar, &SelectionHudBar::alignRight, m_toolMgr.get(), &xcanvas::ToolManager::alignRight);
    connect(m_selectionHudBar, &SelectionHudBar::alignTop, m_toolMgr.get(), &xcanvas::ToolManager::alignTop);
    connect(m_selectionHudBar, &SelectionHudBar::alignBottom, m_toolMgr.get(), &xcanvas::ToolManager::alignBottom);
    connect(m_selectionHudBar, &SelectionHudBar::alignHorizontalCenter, m_toolMgr.get(), &xcanvas::ToolManager::alignHorizontalCenter);
    connect(m_selectionHudBar, &SelectionHudBar::alignVerticalCenter, m_toolMgr.get(), &xcanvas::ToolManager::alignVerticalCenter);
    connect(m_selectionHudBar, &SelectionHudBar::alignCenter, m_toolMgr.get(), &xcanvas::ToolManager::alignCenter);
    m_selectionHudBar->adjustSize();

    QTimer::singleShot(0, this, [this]() { fitCanvas(); });
    updateBottomFloatingToolBarPos();
    m_bottomFloatingToolBar->show();
    m_selectionHudBar->setVisible(false);

    m_rotateHandle.load(QStringLiteral(":/Resource/Icons/RotateHandle.svg"));
}

MyGraphicsView::~MyGraphicsView()
{
}

double MyGraphicsView::zoomValue()
{
    return transform().m11();
}

void MyGraphicsView::requestFullUpdate()
{
    viewport()->update();
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
    m_toolMgr->mousePressEvent(event);
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
    m_toolMgr->mouseMoveEvent(event);
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
    m_toolMgr->mouseReleaseEvent(event);
}

void MyGraphicsView::keyPressEvent(QKeyEvent* event)
{
    m_toolMgr->keyPressEvent(event);
    QGraphicsView::keyPressEvent(event);
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    const QPointF cursorViewPos = mapFromGlobal(QCursor::pos());

    if (event->angleDelta().y() > 0)
    {
        zoomIn(cursorViewPos);
    }
    else
    {
        zoomOut(cursorViewPos);
    }
}

void MyGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateBottomFloatingToolBarPos();
    updateSelectionHudBarPos();
}
void MyGraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);

    updateBottomFloatingToolBarPos();
    updateSelectionHudBarPos();
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
    if (m_toolMgr)
    {
        m_toolMgr->drawPreview(painter);
    }
}

void MyGraphicsView::onZoomIn()
{
    const QPoint zoomCenterPos = rect().center();
    zoomIn(zoomCenterPos);
}

void MyGraphicsView::onZoomOut()
{
    const QPoint zoomCenterPos = rect().center();
    zoomOut(zoomCenterPos);
}

void MyGraphicsView::onUndo()
{
    m_canvas->undoStack()->undo();
    requestFullUpdate();
}

void MyGraphicsView::onRedo()
{
    m_canvas->undoStack()->redo();
    requestFullUpdate();
}

void MyGraphicsView::onSelectionChanged()
{
    if (!m_selectionHudBar || !m_canvas || !m_canvas->shapeManager())
    {
        return;
    }

    const auto* shapeManager = m_canvas->shapeManager();
    const bool  hasSelection = shapeManager->hasSelection();
    m_selectionHudBar->setVisible(hasSelection);
    if (hasSelection)
    {
        m_selectionHudBar->setSummary(shapeManager->selectionSummary());
        updateSelectionHud();
        updateSelectionHudBarPos();
    }
}

void MyGraphicsView::updateSelectionHud()
{
    if (!m_selectionHudBar || !m_canvas || !m_canvas->shapeManager() || !m_selectionHudBar->isVisible())
    {
        return;
    }

    const QRectF  selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const QPointF canvasPos     = sceneToCanvas(selectionRect.topLeft());
    m_selectionHudBar->spinX()->setValue(canvasPos.x());
    m_selectionHudBar->spinY()->setValue(canvasPos.y());
    m_selectionHudBar->spinW()->setValue(selectionRect.width());
    m_selectionHudBar->spinH()->setValue(selectionRect.height());
}

xcanvas::LayerManager* MyGraphicsView::layerManager()
{
    if (m_canvas)
    {
        return m_canvas->layerManager();
    }
    return nullptr;
}

void MyGraphicsView::drawShapes(QPainter* painter, const QRectF& visibleRect)
{
    drawNormalShapes(painter, visibleRect);
    drawSelectedShapes(painter, visibleRect);
}

void MyGraphicsView::drawNormalShapes(QPainter* painter, const QRectF& visibleRect)
{
    QList<xcanvas::LayerParameter*> layers = m_canvas->layerManager()->getOrderedLayers();
    for (auto* layer : layers)
    {
        if (!layer->visible)
            continue;

        if (layer->mode == xcanvas::ProcessMode::Cut)
        {
            painter->save();

            QPen pen(layer->color);
            pen.setWidth(1);
            pen.setCosmetic(true);
            pen.setStyle(Qt::SolidLine);

            painter->setPen(pen);

            for (const auto shape : layer->shapes)
            {
                if (visibleRect.intersects(shape->boundingRect()) && !shape->isSelected())
                {
                    shape->draw(painter);
                }
            }

            painter->restore();
        }
        else if (layer->mode == xcanvas::ProcessMode::Scan)
        {
            painter->save();

            painter->setPen(Qt::NoPen);
            painter->setBrush(layer->color);

            QPainterPath path;
            path.setFillRule(Qt::OddEvenFill);
            for (const auto shape : layer->shapes)
            {
                if (visibleRect.intersects(shape->boundingRect()))
                {
                    path.addPath(shape->path());
                }
            }
            painter->drawPath(path);

            painter->restore();
        }
        else if (layer->mode == xcanvas::ProcessMode::Image)
        {
            for (const auto shape : layer->shapes)
            {
                shape->draw(painter);
            }
        }
    }
}

void MyGraphicsView::drawSelectedShapes(QPainter* painter, const QRectF& visibleRect)
{
    // 最后绘制选中的形状（显示在最上层）
    const QSet<xcanvas::Shape*> selected = m_canvas->shapeManager()->selectedShapes();
    if (selected.isEmpty())
        return;

    painter->save();

    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);

    QPen pen(QColor(244, 155, 33));
    pen.setWidth(1);
    pen.setCosmetic(true);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (xcanvas::Shape* shape : selected)
    {
        if (!shape->isVisible())
        {
            m_canvas->shapeManager()->deselectShape(shape);
            continue;
        }
        if (visibleRect.intersects(shape->boundingRect()))
        {
            painter->drawPath(shape->path());
        }
    }

    // 绘制trace
    if (!selected.isEmpty())
    {
        drawTrace(painter);
    }

    painter->restore();
}

void MyGraphicsView::drawGrid(QPainter* p)
{
    if (!scene() || AppSettings::instance().gridContrast() == AppSettings::GridContrast::Off)
    {
        return;
    }

    // 计算画布在场景坐标中的可见部分
    const QRectF canvasSceneRect  = m_canvas->canvasRect();// 画布本身（场景坐标）
    const QRectF viewSceneRect    = mapToScene(viewport()->rect()).boundingRect();// 视口对应的场景区域
    const QRectF visibleSceneRect = canvasSceneRect.intersected(viewSceneRect);// 画布中可见的部分

    if (!visibleSceneRect.isValid() || visibleSceneRect.isEmpty())
    {
        return;
    }

    const double scale      = this->zoomValue();
    const double step       = gridStep(scale);// 网格步长（场景单位）
    const int    majorCount = 10;

    QColor minorColor;
    QColor majorColor;

    switch (AppSettings::instance().gridContrast())
    {
    case AppSettings::GridContrast::Low:
    {
        majorColor = QColor("#E0E0E0");
        minorColor = QColor("#ECECEC");
    }
    break;
    case AppSettings::GridContrast::Medium:
    {
        majorColor = QColor("#D0D0D0");
        minorColor = QColor("#E4E4E4");
    }
    break;
    case AppSettings::GridContrast::High:
    {
        majorColor = QColor("#C0C0C0");
        minorColor = QColor("#DCDCDC");
    }
    break;
    default:
        break;
    }

    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);

    // 使用视口坐标绘制网格线
    const QTransform oldWorldTransform = p->worldTransform();
    p->setWorldTransform(QTransform());// 切换到视口坐标

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
        QPointF v  = mapFromScene(QPointF(x, visibleSceneRect.top()));
        double  vx = std::round(v.x()) - 0.5;

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
        QPointF v  = mapFromScene(QPointF(visibleSceneRect.left(), y));
        double  vy = std::round(v.y()) - 0.5;

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
    static const double steps[] = {0.1, 0.5, 1.0, 5.0, 10.0, 50.0, 100.0, 500.0};

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

void MyGraphicsView::drawTrace(QPainter* painter)
{
    if (!m_canvas->shapeManager() || m_canvas->shapeManager()->isEmpty())
    {
        return;
    }

    const QRectF rect = m_canvas->shapeManager()->selectedBoundingRect();
    if (!rect.isValid())
    {
        return;
    }

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#90909B"));

    const auto [resizeRects, rotateRect] = xcanvas::geometryMath::traceRects(rect, zoomValue());
    for (const QRectF& r : resizeRects)
    {
        painter->drawRect(r);
    }

    // 画旋转图标
    const QTransform old = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    const QRectF viewRotateRect = mapFromScene(rotateRect).boundingRect();
    m_rotateHandle.render(painter, viewRotateRect);
    painter->setWorldTransform(old);

    painter->restore();
}

void MyGraphicsView::drawCanvas(QPainter* painter)
{
    painter->save();

    const QTransform oldWorldTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    painter->fillRect(rect(), QColor("#E7E9ED"));
    painter->setWorldTransform(oldWorldTransform);

    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawRect(m_canvas->canvasRect());

    painter->restore();
}

void MyGraphicsView::importFile()
{
    const QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Import Files"), QString(), ImportManager::instance().buildDialogFilter());
    importFiles(filePaths);
}

void MyGraphicsView::importFiles(const QStringList& filePaths, const QPointF& targetCenter)
{
    if (filePaths.isEmpty())
    {
        return;
    }

    ImportContext ctx;
    ctx.targetCenter = targetCenter;

    xcanvas::ShapeList allShapes;
    for (const QString& filePath : filePaths)
    {
        xcanvas::ShapeList shapeList = ImportManager::instance().importFile(filePath, ctx);
        if (!shapeList.empty())
        {
            allShapes.append(shapeList);
        }
    }

    if (!allShapes.isEmpty())
    {
        m_canvas->shapeManager()->deselectAll();

        // 统一居中
        QRectF rect;
        for (auto* shape : allShapes)
        {
            rect |= shape->boundingRect();
        }

        const QPointF offset = ctx.targetCenter - rect.center();
        for (auto* shape : allShapes)
        {
            shape->translate(offset);
        }

        m_canvas->addShapes(allShapes);
        requestFullUpdate();
    }
}

void MyGraphicsView::importFiles(const QStringList& filePaths)
{
    importFiles(filePaths, mapToScene(viewport()->rect().center()));
}

void MyGraphicsView::updateBottomFloatingToolBarPos()
{
    constexpr int margin = 12;
    QSize         barSize;
    int           x = 0, y = 0;
    if (m_bottomFloatingToolBar)
    {
        barSize = m_bottomFloatingToolBar->sizeHint();
        x       = width() - barSize.width() - margin;
        y       = height() - barSize.height() - margin;
        m_bottomFloatingToolBar->move(x, y);
    }

    if (m_colorPaletteWidget)
    {
        barSize = m_colorPaletteWidget->sizeHint();
        int x   = 0;
        int y   = height() - barSize.height() - margin;
        m_colorPaletteWidget->move(x, y);
    }
}

void MyGraphicsView::updateSelectionHudBarPos()
{
    if (!m_selectionHudBar && !m_selectionHudBar->isVisible())
    {
        return;
    }

    constexpr int marginTop = 12;
    const QSize   barSize   = m_selectionHudBar->sizeHint();
    const int     x         = (width() - barSize.width()) / 2;
    constexpr int y         = marginTop;

    m_selectionHudBar->move(x, y);
}

void MyGraphicsView::zoomIn(const QPointF& zoomCenterPoint)
{
    if (m_dScaleFactor >= MAX_ZOOM)
        return;

    constexpr qreal dScale = 1.1;

    const QPointF sceneAnchor = viewportTransform().inverted().map(zoomCenterPoint);

    scale(dScale, dScale);
    m_dScaleFactor *= dScale;
    m_dScaleFactor = qBound(MIN_ZOOM, m_dScaleFactor, MAX_ZOOM);

    const QPointF newSceneAnchor = viewportTransform().inverted().map(zoomCenterPoint);

    const QPointF delta = newSceneAnchor - sceneAnchor;
    translate(delta.x(), delta.y());

    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::zoomOut(const QPointF& zoomCenterPoint)
{
    if (m_dScaleFactor <= MIN_ZOOM)
        return;

    constexpr qreal dScale = 1.0 / 1.1;

    const QPointF sceneAnchor = viewportTransform().inverted().map(zoomCenterPoint);

    scale(dScale, dScale);
    m_dScaleFactor *= dScale;
    m_dScaleFactor = qBound(MIN_ZOOM, m_dScaleFactor, MAX_ZOOM);

    const QPointF newSceneAnchor = viewportTransform().inverted().map(zoomCenterPoint);

    const QPointF delta = newSceneAnchor - sceneAnchor;
    translate(delta.x(), delta.y());

    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::zoomTo(qreal zoomValue)
{
    if (zoomValue <= 0)
        return;

    qreal targetScale = qBound(MIN_ZOOM, zoomValue, MAX_ZOOM);

    const QPointF viewAnchor = viewport()->rect().center();

    const QPointF sceneAnchor = viewportTransform().inverted().map(viewAnchor);

    const qreal factor = targetScale / m_dScaleFactor;
    if (qFuzzyCompare(factor, 1.0))
        return;

    scale(factor, factor);
    m_dScaleFactor = targetScale;

    const QPointF newSceneAnchor = viewportTransform().inverted().map(viewAnchor);

    const QPointF delta = newSceneAnchor - sceneAnchor;
    translate(delta.x(), delta.y());

    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitWidth()
{
    const qreal scale = viewport()->width() / m_canvas->canvasRect().width();
    zoomTo(scale);
    centerOn(m_canvas->canvasRect().center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitHeight()
{
    const qreal scale = viewport()->height() / m_canvas->canvasRect().height();
    zoomTo(scale);
    centerOn(m_canvas->canvasRect().center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitCanvas()
{
    const qreal scaleW = viewport()->width() / m_canvas->canvasRect().width();
    const qreal scaleH = viewport()->height() / m_canvas->canvasRect().height();
    const qreal scale  = qMin(scaleW, scaleH);
    zoomTo(scale);
    centerOn(m_canvas->canvasRect().center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::fitShapes()
{
    if (!m_canvas->shapeManager() || m_canvas->shapeManager()->isEmpty())
    {
        return;
    }

    const QRectF rect = m_canvas->shapeManager()->boundingRect();

    const qreal scaleW = viewport()->width() / rect.width();
    const qreal scaleH = viewport()->height() / rect.height();
    const qreal scale  = qMin(scaleW, scaleH);
    zoomTo(scale);
    centerOn(rect.center());
    emit EventBus::instance().zoomChanged(m_dScaleFactor);
}

void MyGraphicsView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        // 检查是否有支持的文件格式
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls)
        {
            if (url.isLocalFile())
            {
                const QString filePath = url.toLocalFile();
                if (ImportManager::instance().canImport(filePath))
                {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void MyGraphicsView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        // 检查是否有支持的文件格式
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls)
        {
            if (url.isLocalFile())
            {
                const QString filePath = url.toLocalFile();
                if (ImportManager::instance().canImport(filePath))
                {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void MyGraphicsView::dropEvent(QDropEvent* event)
{
    if (!event->mimeData()->hasUrls())
    {
        event->ignore();
        return;
    }

    const QList<QUrl> urls = event->mimeData()->urls();
    QStringList filePaths;
    for (const QUrl& url : urls)
    {
        if (url.isLocalFile())
        {
            filePaths.append(url.toLocalFile());
        }
    }

    if (filePaths.isEmpty())
    {
        event->ignore();
        return;
    }

    event->acceptProposedAction();
    const QPointF mouseScenePos = mapToScene(event->pos());
    importFiles(filePaths, mouseScenePos);
}
