#include "MyCanvasView.h"
#include "AppSettings.h"
#include "Controller/AsyncFileTaskRunner.h"
#include "BottomFloatingToolBar.h"
#include "Canvas/Canvas.h"
#include "Canvas/SelectionOutlineStyle.h"
#include "Controller/ClipboardCommandService.h"
#include "ColorPaletteWidget.h"
#include "Controller/DocumentIoController.h"
#include "Controller/DocumentSessionController.h"
#include "EditorSession.h"
#include "MessageWidget.h"
#include "Import/DXF/DXFImporter.h"
#include "Import/Image/ImageImporter.h"
#include "Import/ImportManager.h"
#include "Import/PDF/PDFImporter.h"
#include "SelectionHudBar.h"
#include "Controller/SelectionHudController.h"
#include "Controller/SelectionUiCoordinator.h"
#include "Controller/ShapeInteractionController.h"
#include "Controller/TextEditController.h"
#include "Controller/ViewInteractionController.h"
#include "Controller/ViewLayoutController.h"
#include "Controller/ViewRenderController.h"
#include "Controller/ViewportTransformController.h"
#include "Global.h"
#include "Shape/Shape.h"
#include "ToolManager.h"
#include <QEvent>
#include <QFileDialog>
#include <QHideEvent>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QUndoStack>
#include <QScrollBar>
#include <qtfluentwidgets.h>

MyCanvasView::MyCanvasView(EditorSession* session, QWidget* parent)
    : xcanvas::CanvasView{parent},
      m_canvas(new xcanvas::Canvas(this)),
      m_editorSession(session)
{
    initView();
    initCore();
    initWidgets();
    initControllers();
    initConnections();
    initStartup();
    applyStyle();
}

void MyCanvasView::initView()
{
    setObjectName("MyCanvasView");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAcceptDrops(true);
}

void MyCanvasView::initCore()
{
    m_fileTaskRunner = std::make_unique<AsyncFileTaskRunner>(
        this,
        [this](const bool enabled)
        {
            if (m_editorSession)
            {
                m_editorSession->setFileActionsEnabled(enabled);
            }
        });

    ImportManager::instance().registerImporter(std::make_unique<DXFImporter>());
    ImportManager::instance().registerImporter(std::make_unique<ImageImporter>());
    ImportManager::instance().registerImporter(std::make_unique<PDFImporter>());

    m_toolMgr = std::make_unique<xcanvas::ToolManager>(this, m_canvas, [this]() { updateSelectionHud(); });
}

void MyCanvasView::initWidgets()
{
    m_colorPaletteWidget = new ColorPaletteWidget(this);
    m_colorPaletteWidget->adjustSize();

    m_bottomFloatingToolBar = new BottomFloatingToolBar(m_editorSession, this);
    m_bottomFloatingToolBar->adjustSize();

    m_selectionHudBar = new SelectionHudBar(this);
    m_selectionHudBar->adjustSize();
}

void MyCanvasView::initControllers()
{
    m_viewportTransformController = std::make_unique<ViewportTransformController>(this, this, m_canvas);
    m_viewRenderController = std::make_unique<ViewRenderController>(this, m_canvas, &m_rotateHandle);
    m_viewLayoutController = std::make_unique<ViewLayoutController>(this, m_bottomFloatingToolBar, m_colorPaletteWidget, m_selectionHudBar);
    m_selectionHudController = std::make_unique<SelectionHudController>(m_canvas, m_selectionHudBar, [this]() { requestFullUpdate(); });
    m_selectionUiCoordinator = std::make_unique<SelectionUiCoordinator>(
        m_canvas,
        m_selectionHudBar,
        m_selectionHudController.get(),
        [this]() { updateSelectionHudBarPos(); });

    m_documentSessionController = std::make_unique<DocumentSessionController>(
        this,
        m_canvas,
        [this]() { return m_fileTaskRunner && m_fileTaskRunner->isTaskRunning(); },
        [this]() { MessageWidget::showWarning(window(), tr("正在处理文件，请稍候。")); },
        [this](const QString& path) { openDocumentFileAsync(path); },
        [this](const QString& path, const bool updateCurrentPath) { saveDocumentFileAsync(path, updateCurrentPath); },
        [this](const QString& path) { return saveDocumentFileBlocking(path); },
        [this]()
        {
            if (!m_canvas)
            {
                return;
            }

            m_canvas->shapeManager()->clear();
            m_canvas->layerManager()->clearAllLayers();
            m_canvas->undoStack()->clear();
            m_canvas->undoStack()->setClean();

            if (m_clipboardCommandService)
            {
                m_clipboardCommandService->resetState();
            }
            if (m_selectionHudBar)
            {
                m_selectionHudBar->setVisible(false);
            }
            requestFullUpdate();
        });

    m_clipboardCommandService = std::make_unique<ClipboardCommandService>(
        this,
        m_canvas,
        [this]() { requestFullUpdate(); },
        [this](const QStringList& filePaths, const QPointF& targetCenter)
        {
            if (m_documentIoController)
            {
                m_documentIoController->importFiles(filePaths, targetCenter);
            }
        });

    m_documentIoController = std::make_unique<DocumentIoController>(
        this,
        m_canvas,
        m_fileTaskRunner.get(),
        m_documentSessionController.get(),
        m_clipboardCommandService.get(),
        m_selectionHudBar,
        [this]() { requestFullUpdate(); },
        [this]() { updateWindowTitle(); },
        this);

    m_textEditController = std::make_unique<TextEditController>(
        this,
        m_canvas,
        m_viewRenderController.get(),
        [this]() { updateSelectionHud(); },
        [this]() { requestFullUpdate(); });

    m_shapeInteractionController = std::make_unique<ShapeInteractionController>(
        this,
        m_canvas,
        [this]() { return m_toolMgr && m_toolMgr->currentTool() == DrawingToolType::Select; },
        m_textEditController.get());

    m_viewInteractionController = std::make_unique<ViewInteractionController>(
        this,
        this,
        [this]() { return m_toolMgr && m_toolMgr->currentTool() == DrawingToolType::Select; },
        [this](const QPoint& pos)
        {
            if (m_clipboardCommandService)
            {
                m_clipboardCommandService->showCanvasContextMenu(
                    pos,
                    [this]() { zoomIn(rect().center()); },
                    [this]() { zoomOut(rect().center()); });
            }
        },
        [this]() { return m_clipboardCommandService ? m_clipboardCommandService->groupSelectedShapes() : false; },
        [this]() { return m_clipboardCommandService ? m_clipboardCommandService->ungroupSelectedShapes() : false; },
        [this]() { return m_clipboardCommandService ? m_clipboardCommandService->cutSelectedShapes() : false; },
        [this]() { return m_clipboardCommandService ? m_clipboardCommandService->copySelectedShapes() : false; },
        [this]() { return m_clipboardCommandService ? m_clipboardCommandService->pasteCopiedShapes() : false; },
        [this]()
        {
            if (m_canvas && m_canvas->shapeManager())
            {
                m_canvas->shapeManager()->selectAll();
                requestFullUpdate();
            }
        },
        [this](const QPointF& pos) { zoomIn(pos); },
        [this](const QPointF& pos) { zoomOut(pos); },
        [this](const QString& path) { return isProjectFilePath(path); },
        [this]() { return maybeSaveBeforeProceed(); },
        [this](const QString& path) { return m_documentSessionController ? m_documentSessionController->openDocumentFile(path) : false; },
        [this](const QStringList& filePaths, const QPointF& targetCenter)
        {
            if (m_documentIoController)
            {
                m_documentIoController->importFiles(filePaths, targetCenter);
            }
        });
}

void MyCanvasView::initConnections()
{
    m_selectionOutlineTimer = new QTimer(this);
    m_selectionOutlineTimer->setInterval(75);
    connect(m_selectionOutlineTimer, &QTimer::timeout, this, [this]()
    {
        m_selectionDashPhase = xcanvas::SelectionOutlineStyle::advancePhase(m_selectionDashPhase);
        if (m_viewRenderController)
        {
            m_viewRenderController->setSelectionDashPhase(m_selectionDashPhase);
        }
        requestFullUpdate();
    });

    connect(&AppSettings::instance(), &AppSettings::gridContrastChanged, this, [this]() { requestFullUpdate(); });
    connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
            [this](qfw::Theme) { requestFullUpdate(); });
    connect(this, &xcanvas::CanvasView::cameraChanged, this, [this]()
    {
        if (m_viewLayoutController)
        {
            m_viewLayoutController->onViewportChanged(m_isDestroying);
        }
    });
    connect(m_canvas->layerManager(), &xcanvas::LayerManager::layerVisibilityChanged, this, [this]() { requestFullUpdate(); });
    if (m_editorSession)
    {
        connect(m_editorSession, &EditorSession::switchTool, m_toolMgr.get(), &xcanvas::ToolManager::setTool);
        connect(m_editorSession, &EditorSession::drawingToolLockChanged, m_toolMgr.get(),
                &xcanvas::ToolManager::setDrawingToolLocked);
        connect(m_editorSession, &EditorSession::drawingToolLockChanged, &AppSettings::instance(),
                &AppSettings::setDrawingToolLocked);
        connect(m_toolMgr.get(), &xcanvas::ToolManager::toolChanged, m_editorSession,
                &EditorSession::notifyCurrentToolChanged);
        connect(m_toolMgr.get(), &xcanvas::ToolManager::drawingFinished, m_editorSession,
                &EditorSession::notifyFinishDrawing);
    }
    connect(m_canvas->shapeManager(), &xcanvas::ShapeManager::selectionChanged, this, [this]()
    {
        if (m_selectionUiCoordinator)
        {
            m_selectionUiCoordinator->onSelectionChanged();
        }
        syncSelectionOutlineAnimation();
        requestFullUpdate();
    });
    connect(m_colorPaletteWidget, &ColorPaletteWidget::colorSelected, this, &MyCanvasView::onColorSelected);

    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomIn, this, [this]() { zoomIn(rect().center()); });
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomOut, this, [this]() { zoomOut(rect().center()); });
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomTo, this, &MyCanvasView::zoomTo);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitWidth, this, &MyCanvasView::fitWidth);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitHeight, this, &MyCanvasView::fitHeight);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitCanvas, this, &MyCanvasView::fitCanvas);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitShapes, this, &MyCanvasView::fitShapes);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::undo, this, [this]() { m_canvas->undoStack()->undo(); requestFullUpdate(); });
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::redo, this, [this]() { m_canvas->undoStack()->redo(); requestFullUpdate(); });
    connect(m_canvas->undoStack(), &QUndoStack::canUndoChanged, m_bottomFloatingToolBar, &BottomFloatingToolBar::setCanUndo);
    connect(m_canvas->undoStack(), &QUndoStack::canRedoChanged, m_bottomFloatingToolBar, &BottomFloatingToolBar::setCanRedo);
    connect(m_canvas->undoStack(), &QUndoStack::cleanChanged, this, [this]() { updateWindowTitle(); });
    connect(m_canvas->undoStack(), &QUndoStack::indexChanged, this, [this](int)
    {
        if (m_isDestroying)
        {
            return;
        }
        updateSelectionHud();
        updateSelectionHudBarPos();
        requestFullUpdate();
    });

    if (m_editorSession)
    {
        connect(m_editorSession, &EditorSession::newFileRequested, this, [this]()
        {
            if (m_documentSessionController)
            {
                m_documentSessionController->onNewDocument();
            }
        });
        connect(m_editorSession, &EditorSession::importFileRequested, this, &MyCanvasView::importFile);
        connect(m_editorSession, &EditorSession::saveFileRequested, this, [this]()
        {
            if (m_documentSessionController)
            {
                m_documentSessionController->onSaveDocument();
            }
        });
        connect(m_editorSession, &EditorSession::saveFileAsRequested, this, [this]()
        {
            if (m_documentSessionController)
            {
                m_documentSessionController->onSaveDocumentAs();
            }
        });
    }

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
    connect(m_selectionHudBar, &SelectionHudBar::xEdited, this, [this](const double newX)
    {
        if (m_selectionHudController)
        {
            m_selectionHudController->applySelectionHudX(newX);
        }
    });
    connect(m_selectionHudBar, &SelectionHudBar::yEdited, this, [this](const double newY)
    {
        if (m_selectionHudController)
        {
            m_selectionHudController->applySelectionHudY(newY);
        }
    });
    connect(m_selectionHudBar, &SelectionHudBar::wEdited, this, [this](const double newW)
    {
        if (m_selectionHudController)
        {
            m_selectionHudController->applySelectionHudW(newW);
        }
    });
    connect(m_selectionHudBar, &SelectionHudBar::hEdited, this, [this](const double newH)
    {
        if (m_selectionHudController)
        {
            m_selectionHudController->applySelectionHudH(newH);
        }
    });
    connect(m_selectionHudBar, &SelectionHudBar::angleEdited, this, [this](const double newAngle)
    {
        if (m_selectionHudController)
        {
            m_selectionHudController->applySelectionHudAngle(newAngle);
        }
    });
    connect(m_selectionHudBar, &SelectionHudBar::keepAspectRatioToggled, this, [this](const bool enabled)
    {
        if (m_selectionHudController)
        {
            m_selectionHudController->setKeepAspectRatio(enabled);
        }
    });
}

void MyCanvasView::initStartup()
{
    if (m_editorSession)
    {
        m_editorSession->requestSetDrawingToolLock(AppSettings::instance().drawingToolLocked());
    }

    m_initialFitTimer = new QTimer(this);
    m_initialFitTimer->setSingleShot(true);
    m_initialFitTimer->setInterval(50);
    connect(m_initialFitTimer, &QTimer::timeout, this, [this]()
    {
        if (!m_initialFitPending || !isVisible() || viewportRect().isEmpty())
        {
            return;
        }
        fitCanvas();
        m_initialFitPending = false;
    });

    updateBottomFloatingToolBarPos();
    m_bottomFloatingToolBar->show();
    m_selectionHudBar->setVisible(false);
    m_rotateHandle.load(QStringLiteral(":/Resource/Icons/RotateHandle.svg"));
    updateWindowTitle();
}

void MyCanvasView::applyStyle() {
    setStyleSheet(QStringLiteral(R"(
        MyCanvasView#MyCanvasView {
            border: none;
        }
    )"));
}

MyCanvasView::~MyCanvasView()
{
    m_isDestroying = true;
    if (m_canvas && m_canvas->undoStack())
    {
        disconnect(m_canvas->undoStack(), nullptr, this, nullptr);
    }
    if (m_clipboardCommandService)
    {
        m_clipboardCommandService->clearCopiedShapes();
    }
}

double MyCanvasView::zoomValue() const {
    return zoomScale();
}

void MyCanvasView::requestFullUpdate() const {
    if (m_isDestroying)
    {
        return;
    }

    if (QWidget* vp = viewport())
    {
        vp->update();
    }
}

void MyCanvasView::mousePressEvent(QMouseEvent* event)
{
    focusForPointerPress();
    if (filterRulerMousePress(event))
    {
        return;
    }
    if (m_textEditController && m_textEditController->isEditing())
    {
        if (event->button() == Qt::LeftButton)
        {
            const QPointF scenePos = mapToWorld(event->pos());
            if (m_textEditController->moveCursorToScenePos(scenePos))
            {
                return;
            }
        }
        m_textEditController->finishInlineEdit(true);
    }

    if (m_viewInteractionController)
    {
        m_viewInteractionController->mousePressEvent(event);
    }
    if (event->button() != Qt::MiddleButton)
    {
        m_toolMgr->mousePressEvent(event);
    }
}

void MyCanvasView::mouseMoveEvent(QMouseEvent* event)
{
    if (filterRulerMouseMove(event))
    {
        return;
    }
    if (m_textEditController && m_textEditController->isEditing())
    {
        xcanvas::CanvasView::mouseMoveEvent(event);
        emit mouseMovePos(event->pos());
        return;
    }

    if (m_viewInteractionController)
    {
        m_viewInteractionController->mouseMoveEvent(event);
    }

    if (!(event->buttons() & Qt::MiddleButton))
    {
        m_toolMgr->mouseMoveEvent(event);
    }
    emit mouseMovePos(event->pos());
}

void MyCanvasView::mouseReleaseEvent(QMouseEvent* event)
{
    if (filterRulerMouseRelease(event))
    {
        return;
    }
    if (m_textEditController && m_textEditController->isEditing())
    {
        xcanvas::CanvasView::mouseReleaseEvent(event);
        return;
    }

    const bool shouldOpenContextMenu = m_viewInteractionController ? m_viewInteractionController->mouseReleaseEvent(event) : false;
    if (event->button() != Qt::MiddleButton)
    {
        m_toolMgr->mouseReleaseEvent(event);
    }
    if (shouldOpenContextMenu)
    {
        if (m_clipboardCommandService)
        {
            m_clipboardCommandService->showCanvasContextMenu(
                event->pos(),
                [this]() { zoomIn(rect().center()); },
                [this]() { zoomOut(rect().center()); });
        }
    }
}

void MyCanvasView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event && isInRuler(event->position().toPoint()))
    {
        event->accept();
        return;
    }
    if (m_textEditController && m_textEditController->isEditing())
    {
        xcanvas::CanvasView::mouseDoubleClickEvent(event);
        return;
    }

    if (m_shapeInteractionController && m_shapeInteractionController->mouseDoubleClickEvent(event))
    {
        return;
    }

    xcanvas::CanvasView::mouseDoubleClickEvent(event);
}

void MyCanvasView::keyPressEvent(QKeyEvent* event)
{
    if (m_textEditController && m_textEditController->isEditing())
    {
        m_textEditController->keyPressEvent(event);
        return;
    }

    if (m_viewInteractionController && m_viewInteractionController->keyPressEvent(event))
    {
        return;
    }

    m_toolMgr->keyPressEvent(event);
    xcanvas::CanvasView::keyPressEvent(event);
}

void MyCanvasView::inputMethodEvent(QInputMethodEvent* event)
{
    if (m_textEditController && m_textEditController->isEditing())
    {
        m_textEditController->inputMethodEvent(event);
        return;
    }

    m_toolMgr->inputMethodEvent(event);
}

QVariant MyCanvasView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (m_textEditController && m_textEditController->isEditing())
    {
        return m_textEditController->inputMethodQuery(query);
    }

    return m_toolMgr->inputMethodQuery(query);
}


void MyCanvasView::wheelEvent(QWheelEvent* event)
{
    if (m_viewInteractionController && m_viewInteractionController->wheelEvent(event))
    {
        return;
    }
    xcanvas::CanvasView::wheelEvent(event);
}

void MyCanvasView::showEvent(QShowEvent* event)
{
    xcanvas::CanvasView::showEvent(event);
    scheduleInitialFit();
    syncSelectionOutlineAnimation();
}

void MyCanvasView::hideEvent(QHideEvent* event)
{
    xcanvas::CanvasView::hideEvent(event);
    syncSelectionOutlineAnimation();
}

void MyCanvasView::resizeEvent(QResizeEvent* event)
{
    xcanvas::CanvasView::resizeEvent(event);
    scheduleInitialFit();
    if (m_viewLayoutController)
    {
        m_viewLayoutController->onViewportChanged(m_isDestroying);
    }
}
void MyCanvasView::scrollContentsBy(int dx, int dy)
{
    xcanvas::CanvasView::scrollContentsBy(dx, dy);
    if (m_viewLayoutController)
    {
        m_viewLayoutController->onViewportChanged(m_isDestroying);
    }
}

void MyCanvasView::drawWorld(QPainter& painter, const QRectF& rect)
{
    if (m_viewRenderController)
    {
        m_viewRenderController->drawBackground(&painter, rect);
    }
    if (m_viewRenderController)
    {
        m_viewRenderController->drawForeground(&painter, rect);
    }
    if (m_textEditController && m_textEditController->isEditing())
    {
        m_textEditController->drawPreview(&painter);
    }
    else if (m_toolMgr)
    {
        m_toolMgr->drawPreview(&painter);
    }
}

void MyCanvasView::scheduleInitialFit()
{
    if (m_initialFitPending && m_initialFitTimer)
    {
        m_initialFitTimer->start();
    }
}

void MyCanvasView::syncSelectionOutlineAnimation()
{
    if (!m_selectionOutlineTimer || !m_canvas || !m_canvas->shapeManager())
    {
        return;
    }

    const bool shouldAnimate = isVisible() && m_canvas->shapeManager()->hasSelection();
    if (shouldAnimate)
    {
        if (!m_selectionOutlineTimer->isActive())
        {
            m_selectionOutlineTimer->start();
        }
        return;
    }

    m_selectionOutlineTimer->stop();
    m_selectionDashPhase = 0.0;
    if (m_viewRenderController)
    {
        m_viewRenderController->setSelectionDashPhase(m_selectionDashPhase);
    }
}

QColor MyCanvasView::rulerBaseColor() const
{
    return ViewRenderController::workspaceBackgroundColor();
}

void MyCanvasView::openDocumentFileAsync(const QString& path) const {
    if (m_documentIoController)
    {
        m_documentIoController->openDocumentFileAsync(path);
    }
}

void MyCanvasView::saveDocumentFileAsync(const QString& path, const bool updateCurrentPath) const {
    if (m_documentIoController)
    {
        m_documentIoController->saveDocumentFileAsync(path, updateCurrentPath);
    }
}

bool MyCanvasView::saveDocumentFileBlocking(const QString& path) const {
    return m_documentIoController ? m_documentIoController->saveDocumentFileBlocking(path) : false;
}

void MyCanvasView::resetToNewDocument() const {
    if (m_documentSessionController)
    {
        m_documentSessionController->resetToNewDocument();
    }
}

bool MyCanvasView::maybeSaveBeforeProceed() const {
    return m_documentSessionController ? m_documentSessionController->maybeSaveBeforeProceed() : true;
}

bool MyCanvasView::maybeSaveBeforeClose() const {
    return m_documentSessionController ? m_documentSessionController->maybeSaveBeforeClose() : true;
}

QString MyCanvasView::currentDocumentPath() const
{
    return m_documentSessionController ? m_documentSessionController->currentDocumentPath() : QString();
}

bool MyCanvasView::openDocumentFile(const QString& path) const
{
    if (path.isEmpty() || !m_documentSessionController)
    {
        return false;
    }
    if (m_fileTaskRunner && m_fileTaskRunner->isTaskRunning())
    {
        MessageWidget::showWarning(window(), tr("正在处理文件，请稍候。"));
        return false;
    }
    return m_documentSessionController->openDocumentFile(path);
}

bool MyCanvasView::isProjectFilePath(const QString& path) const
{
    return m_documentSessionController ? m_documentSessionController->isProjectFilePath(path) : false;
}

QString MyCanvasView::projectDisplayName() const
{
    return m_documentSessionController ? m_documentSessionController->projectDisplayName() : QStringLiteral("untitled");
}

void MyCanvasView::updateWindowTitle() const {
    if (m_documentSessionController)
    {
        m_documentSessionController->updateWindowTitle();
        emit documentDisplayNameChanged(m_documentSessionController->projectDisplayName());
    }
}

void MyCanvasView::onColorSelected(const QColor& color) const {
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->layerManager())
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        shape->setColor(color);
        m_canvas->layerManager()->addShapeToLayer(shape);
    }

    requestFullUpdate();
}

void MyCanvasView::updateSelectionHud() const {
    if (m_selectionUiCoordinator)
    {
        m_selectionUiCoordinator->updateSelectionHud(m_isDestroying);
    }
}

xcanvas::LayerManager* MyCanvasView::layerManager() const {
    if (m_canvas)
    {
        return m_canvas->layerManager();
    }
    return nullptr;
}

void MyCanvasView::importFile()
{
    if (m_fileTaskRunner && m_fileTaskRunner->isTaskRunning())
    {
        MessageWidget::showWarning(window(), tr("正在处理文件，请稍候。"));
        return;
    }

    const QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        tr("Import Files"),
        AppSettings::instance().lastOpenedPathOrDocumentsPath(),
        ImportManager::instance().buildDialogFilter());
    if (!filePaths.isEmpty())
    {
        AppSettings::instance().setLastOpenedPath(filePaths.first());
    }
    importFiles(filePaths);
}

void MyCanvasView::importFiles(const QStringList& filePaths, const QPointF& targetCenter) const {
    if (m_documentIoController)
    {
        m_documentIoController->importFiles(filePaths, targetCenter);
    }
}

void MyCanvasView::importFiles(const QStringList& filePaths) const {
    importFiles(filePaths, mapToWorld(viewportRect().center()));
}

void MyCanvasView::updateBottomFloatingToolBarPos() const {
    if (m_viewLayoutController)
    {
        m_viewLayoutController->updateBottomFloatingToolBarPos();
    }
}

void MyCanvasView::updateSelectionHudBarPos() const {
    if (m_viewLayoutController)
    {
        m_viewLayoutController->updateSelectionHudBarPos(m_isDestroying);
    }
}

void MyCanvasView::zoomIn(const QPointF& zoomCenterPoint) const {
    if (!m_viewportTransformController)
    {
        return;
    }
    if (m_viewportTransformController->zoomIn(zoomCenterPoint))
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyCanvasView::zoomOut(const QPointF& zoomCenterPoint) const {
    if (!m_viewportTransformController)
    {
        return;
    }
    if (m_viewportTransformController->zoomOut(zoomCenterPoint))
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyCanvasView::zoomTo(qreal zoomValue) const {
    if (!m_viewportTransformController)
    {
        return;
    }
    if (m_viewportTransformController->zoomTo(zoomValue))
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyCanvasView::fitWidth() const {
    if (m_viewportTransformController && m_viewportTransformController->fitWidth())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyCanvasView::fitHeight() const {
    if (m_viewportTransformController && m_viewportTransformController->fitHeight())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyCanvasView::fitCanvas() const {
    if (m_viewportTransformController && m_viewportTransformController->fitCanvas())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyCanvasView::fitShapes() const {
    if (m_viewportTransformController && m_viewportTransformController->fitShapes())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyCanvasView::dragEnterEvent(QDragEnterEvent* event)
{
    if (m_viewInteractionController)
    {
        m_viewInteractionController->dragEnterEvent(event);
    }
}

void MyCanvasView::dragMoveEvent(QDragMoveEvent* event)
{
    if (m_viewInteractionController)
    {
        m_viewInteractionController->dragMoveEvent(event);
    }
}

void MyCanvasView::dropEvent(QDropEvent* event)
{
    if (m_viewInteractionController)
    {
        m_viewInteractionController->dropEvent(event);
    }
}
