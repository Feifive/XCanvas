#include "MyGraphicsView.h"
#include "AppSettings.h"
#include "Controller/AsyncFileTaskRunner.h"
#include "BottomFloatingToolBar.h"
#include "Canvas/Canvas.h"
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
#include "Controller/ViewInteractionController.h"
#include "Controller/ViewLayoutController.h"
#include "Controller/ViewRenderController.h"
#include "Controller/ViewportTransformController.h"
#include "Global.h"
#include "Shape/Shape.h"
#include "Shape/ShapeText.h"
#include "Shape/EditTextCommand.h"
#include "ToolManager.h"
#include <QGraphicsTextItem>
#include <QEvent>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QTextCursor>
#include <QUndoStack>
#include <qtfluentwidgets.h>

MyGraphicsView::MyGraphicsView(EditorSession* session, QWidget* parent)
    : QGraphicsView{parent},
      m_canvas(new xcanvas::Canvas(this)),
      m_bottomFloatingToolBar(nullptr),
      m_inlineTextEditor(nullptr),
      m_inlineEditingShape(nullptr),
      m_isDestroying(false),
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

void MyGraphicsView::initView()
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
}

void MyGraphicsView::initCore()
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

    m_toolMgr = std::make_unique<xcanvas::ToolManager>(this, m_canvas);
}

void MyGraphicsView::initWidgets()
{
    m_colorPaletteWidget = new ColorPaletteWidget(this);
    m_colorPaletteWidget->adjustSize();

    m_bottomFloatingToolBar = new BottomFloatingToolBar(m_editorSession, this);
    m_bottomFloatingToolBar->adjustSize();

    m_selectionHudBar = new SelectionHudBar(this);
    m_selectionHudBar->adjustSize();
}

void MyGraphicsView::initControllers()
{
    m_viewportTransformController = std::make_unique<ViewportTransformController>(this, m_canvas);
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

    m_viewInteractionController = std::make_unique<ViewInteractionController>(
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

void MyGraphicsView::initConnections()
{
    connect(&AppSettings::instance(), &AppSettings::gridContrastChanged, this, [this]() { requestFullUpdate(); });
    connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
            [this](qfw::Theme) { requestFullUpdate(); });
    connect(m_canvas->layerManager(), &xcanvas::LayerManager::layerVisibilityChanged, this, [this]() { requestFullUpdate(); });
    if (m_editorSession)
    {
        connect(m_editorSession, &EditorSession::switchTool, m_toolMgr.get(), &xcanvas::ToolManager::setTool);
    }
    if (m_editorSession)
    {
        connect(m_toolMgr.get(), &xcanvas::ToolManager::drawingFinished, m_editorSession,
                &EditorSession::notifyFinishDrawing);
    }
    connect(m_canvas->shapeManager(), &xcanvas::ShapeManager::selectionChanged, this, [this]()
    {
        if (m_selectionUiCoordinator)
        {
            m_selectionUiCoordinator->onSelectionChanged();
        }
    });
    connect(m_colorPaletteWidget, &ColorPaletteWidget::colorSelected, this, &MyGraphicsView::onColorSelected);

    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomIn, this, [this]() { zoomIn(rect().center()); });
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomOut, this, [this]() { zoomOut(rect().center()); });
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::zoomTo, this, &MyGraphicsView::zoomTo);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitWidth, this, &MyGraphicsView::fitWidth);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitHeight, this, &MyGraphicsView::fitHeight);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitCanvas, this, &MyGraphicsView::fitCanvas);
    connect(m_bottomFloatingToolBar, &BottomFloatingToolBar::fitShapes, this, &MyGraphicsView::fitShapes);
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
        connect(m_editorSession, &EditorSession::importFileRequested, this, &MyGraphicsView::importFile);
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

void MyGraphicsView::initStartup()
{
    QTimer::singleShot(0, this, [this]() { fitCanvas(); });
    updateBottomFloatingToolBarPos();
    m_bottomFloatingToolBar->show();
    m_selectionHudBar->setVisible(false);
    m_rotateHandle.load(QStringLiteral(":/Resource/Icons/RotateHandle.svg"));
    updateWindowTitle();
}

void MyGraphicsView::applyStyle() {
    setStyleSheet(QStringLiteral(R"(
        QGraphicsView#MyGraphicsView {
            border: none;
        }
    )"));
}

MyGraphicsView::~MyGraphicsView()
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

double MyGraphicsView::zoomValue() const {
    return transform().m11();
}

void MyGraphicsView::requestFullUpdate() const {
    if (m_isDestroying)
    {
        return;
    }

    if (QWidget* vp = viewport())
    {
        vp->update();
    }
}

void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (m_inlineTextEditor)
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    if (m_viewInteractionController)
    {
        m_viewInteractionController->mousePressEvent(event);
    }
    m_toolMgr->mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_inlineTextEditor)
    {
        QGraphicsView::mouseMoveEvent(event);
        emit mouseMovePos(event->pos());
        return;
    }

    if (m_viewInteractionController)
    {
        m_viewInteractionController->mouseMoveEvent(event);
    }

    m_toolMgr->mouseMoveEvent(event);
    emit mouseMovePos(event->pos());
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_inlineTextEditor)
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    const bool shouldOpenContextMenu = m_viewInteractionController ? m_viewInteractionController->mouseReleaseEvent(event) : false;
    m_toolMgr->mouseReleaseEvent(event);
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

void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (m_inlineTextEditor)
    {
        QGraphicsView::mouseDoubleClickEvent(event);
        return;
    }

    if (!event || event->button() != Qt::LeftButton || !m_toolMgr || m_toolMgr->currentTool() != DrawingToolType::Select)
    {
        QGraphicsView::mouseDoubleClickEvent(event);
        return;
    }

    xcanvas::Shape* shape = findTopShapeAtScenePos(mapToScene(event->pos()));
    if (!shape || shape->type() != xcanvas::ShapeType::Text)
    {
        QGraphicsView::mouseDoubleClickEvent(event);
        return;
    }

    auto* textShape = static_cast<xcanvas::ShapeText*>(shape);
    m_canvas->shapeManager()->selectShape(textShape, true);
    beginInlineTextEdit(textShape);
    event->accept();
}

void MyGraphicsView::keyPressEvent(QKeyEvent* event)
{
    if (m_inlineTextEditor)
    {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    if (m_viewInteractionController && m_viewInteractionController->keyPressEvent(event))
    {
        return;
    }

    m_toolMgr->keyPressEvent(event);
    QGraphicsView::keyPressEvent(event);
}

bool MyGraphicsView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_inlineTextEditor && event)
    {
        if (event->type() == QEvent::FocusOut)
        {
            QTimer::singleShot(0, this, [this]() { finishInlineTextEdit(true); });
        }
        else if (event->type() == QEvent::KeyPress)
        {
            const auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent && keyEvent->key() == Qt::Key_Escape)
            {
                finishInlineTextEdit(false);
                return true;
            }
        }
    }

    return QGraphicsView::eventFilter(watched, event);
}

xcanvas::Shape* MyGraphicsView::findTopShapeAtScenePos(const QPointF& scenePos) const
{
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return nullptr;
    }

    const xcanvas::ShapeList shapeList = m_canvas->shapeManager()->shapes();
    const double tolerance = 6.0 / zoomValue();
    for (int i = shapeList.size() - 1; i >= 0; --i)
    {
        xcanvas::Shape* shape = shapeList.at(i);
        if (!shape || !shape->isVisible())
        {
            continue;
        }
        if (shape->hitTest(scenePos, tolerance))
        {
            return shape;
        }
    }
    return nullptr;
}

void MyGraphicsView::beginInlineTextEdit(xcanvas::ShapeText* shape)
{
    if (!shape || !scene() || m_inlineTextEditor)
    {
        return;
    }

    m_inlineEditingShape = shape;
    m_inlineOriginalText = shape->text();

    m_inlineTextEditor = new QGraphicsTextItem(shape->text());
    m_inlineTextEditor->document()->setDocumentMargin(0);
    m_inlineTextEditor->setTextInteractionFlags(Qt::TextEditorInteraction);
    m_inlineTextEditor->setFont(shape->font());
    m_inlineTextEditor->setDefaultTextColor(shape->color());
    m_inlineTextEditor->setTransform(shape->transform());
    m_inlineTextEditor->setZValue(Z_VALUE_HIGHLIGHT + 1);
    m_inlineTextEditor->installEventFilter(this);

    scene()->addItem(m_inlineTextEditor);
    m_inlineTextEditor->setFocus(Qt::MouseFocusReason);

    QTextCursor cursor = m_inlineTextEditor->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_inlineTextEditor->setTextCursor(cursor);

    if (m_viewRenderController)
    {
        m_viewRenderController->setSuppressedShape(shape);
        m_viewRenderController->setSelectionHandlesVisible(false);
    }

    updateSelectionHud();
    requestFullUpdate();
}

void MyGraphicsView::finishInlineTextEdit(const bool commit)
{
    if (!m_inlineTextEditor)
    {
        return;
    }

    const QString editedText = m_inlineTextEditor->toPlainText();
    xcanvas::ShapeText* editingShape = m_inlineEditingShape;

    m_inlineTextEditor->removeEventFilter(this);
    if (scene())
    {
        scene()->removeItem(m_inlineTextEditor);
    }
    delete m_inlineTextEditor;
    m_inlineTextEditor = nullptr;

    if (editingShape)
    {
        if (commit && editedText != m_inlineOriginalText && m_canvas && m_canvas->undoStack() && m_canvas->shapeManager())
        {
            m_canvas->undoStack()->push(
                new xcanvas::EditTextCommand(m_canvas->shapeManager(), editingShape, m_inlineOriginalText, editedText));
        }
    }

    m_inlineEditingShape = nullptr;
    m_inlineOriginalText.clear();
    if (m_viewRenderController)
    {
        m_viewRenderController->setSuppressedShape(nullptr);
        m_viewRenderController->setSelectionHandlesVisible(true);
    }
    updateSelectionHud();
    requestFullUpdate();
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    if (m_viewInteractionController && m_viewInteractionController->wheelEvent(event))
    {
        return;
    }
    QGraphicsView::wheelEvent(event);
}

void MyGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    if (m_viewLayoutController)
    {
        m_viewLayoutController->onViewportChanged(m_isDestroying);
    }
}
void MyGraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    if (m_viewLayoutController)
    {
        m_viewLayoutController->onViewportChanged(m_isDestroying);
    }
}

void MyGraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (m_viewRenderController)
    {
        m_viewRenderController->drawBackground(painter, rect);
    }
}

void MyGraphicsView::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (m_viewRenderController)
    {
        m_viewRenderController->drawForeground(painter, rect);
    }
    if (m_toolMgr)
    {
        m_toolMgr->drawPreview(painter);
    }
}

void MyGraphicsView::openDocumentFileAsync(const QString& path) const {
    if (m_documentIoController)
    {
        m_documentIoController->openDocumentFileAsync(path);
    }
}

void MyGraphicsView::saveDocumentFileAsync(const QString& path, const bool updateCurrentPath) const {
    if (m_documentIoController)
    {
        m_documentIoController->saveDocumentFileAsync(path, updateCurrentPath);
    }
}

bool MyGraphicsView::saveDocumentFileBlocking(const QString& path) const {
    return m_documentIoController ? m_documentIoController->saveDocumentFileBlocking(path) : false;
}

void MyGraphicsView::resetToNewDocument() const {
    if (m_documentSessionController)
    {
        m_documentSessionController->resetToNewDocument();
    }
}

bool MyGraphicsView::maybeSaveBeforeProceed() const {
    return m_documentSessionController ? m_documentSessionController->maybeSaveBeforeProceed() : true;
}

bool MyGraphicsView::maybeSaveBeforeClose() const {
    return m_documentSessionController ? m_documentSessionController->maybeSaveBeforeClose() : true;
}

QString MyGraphicsView::currentDocumentPath() const
{
    return m_documentSessionController ? m_documentSessionController->currentDocumentPath() : QString();
}

bool MyGraphicsView::openDocumentFile(const QString& path) const
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

bool MyGraphicsView::isProjectFilePath(const QString& path) const
{
    return m_documentSessionController ? m_documentSessionController->isProjectFilePath(path) : false;
}

QString MyGraphicsView::projectDisplayName() const
{
    return m_documentSessionController ? m_documentSessionController->projectDisplayName() : QStringLiteral("untitled");
}

void MyGraphicsView::updateWindowTitle() const {
    if (m_documentSessionController)
    {
        m_documentSessionController->updateWindowTitle();
        emit documentDisplayNameChanged(m_documentSessionController->projectDisplayName());
    }
}

void MyGraphicsView::onColorSelected(const QColor& color) const {
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

void MyGraphicsView::updateSelectionHud() const {
    if (m_selectionUiCoordinator)
    {
        m_selectionUiCoordinator->updateSelectionHud(m_isDestroying);
    }
}

xcanvas::LayerManager* MyGraphicsView::layerManager() const {
    if (m_canvas)
    {
        return m_canvas->layerManager();
    }
    return nullptr;
}

void MyGraphicsView::importFile()
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

void MyGraphicsView::importFiles(const QStringList& filePaths, const QPointF& targetCenter) const {
    if (m_documentIoController)
    {
        m_documentIoController->importFiles(filePaths, targetCenter);
    }
}

void MyGraphicsView::importFiles(const QStringList& filePaths) const {
    importFiles(filePaths, mapToScene(viewport()->rect().center()));
}

void MyGraphicsView::updateBottomFloatingToolBarPos() const {
    if (m_viewLayoutController)
    {
        m_viewLayoutController->updateBottomFloatingToolBarPos();
    }
}

void MyGraphicsView::updateSelectionHudBarPos() const {
    if (m_viewLayoutController)
    {
        m_viewLayoutController->updateSelectionHudBarPos(m_isDestroying);
    }
}

void MyGraphicsView::zoomIn(const QPointF& zoomCenterPoint) const {
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

void MyGraphicsView::zoomOut(const QPointF& zoomCenterPoint) const {
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

void MyGraphicsView::zoomTo(qreal zoomValue) const {
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

void MyGraphicsView::fitWidth() const {
    if (m_viewportTransformController && m_viewportTransformController->fitWidth())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyGraphicsView::fitHeight() const {
    if (m_viewportTransformController && m_viewportTransformController->fitHeight())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyGraphicsView::fitCanvas() const {
    if (m_viewportTransformController && m_viewportTransformController->fitCanvas())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyGraphicsView::fitShapes() const {
    if (m_viewportTransformController && m_viewportTransformController->fitShapes())
    {
        const qreal scaleFactor = m_viewportTransformController->scaleFactor();
        if (m_editorSession)
        {
            m_editorSession->notifyZoomChanged(scaleFactor);
        }
    }
}

void MyGraphicsView::dragEnterEvent(QDragEnterEvent* event)
{
    if (m_viewInteractionController)
    {
        m_viewInteractionController->dragEnterEvent(event);
    }
}

void MyGraphicsView::dragMoveEvent(QDragMoveEvent* event)
{
    if (m_viewInteractionController)
    {
        m_viewInteractionController->dragMoveEvent(event);
    }
}

void MyGraphicsView::dropEvent(QDropEvent* event)
{
    if (m_viewInteractionController)
    {
        m_viewInteractionController->dropEvent(event);
    }
}
