#include "MyGraphicsView.h"
#include "AppSettings.h"
#include "BottomFloatingToolBar.h"
#include "Canvas/Canvas.h"
#include "ColorPaletteWidget.h"
#include "EventBus.h"
#include "MessageWidget.h"
#include "Import/DXF/DXFImporter.h"
#include "Import/Image/ImageImporter.h"
#include "Import/ImportManager.h"
#include "Import/PDF/PDFImporter.h"
#include "MessageBox.h"
#include "SelectionHudBar.h"
#include "Shape/ShapeImage.h"
#include "Shape/Shape.h"
#include "Shape/ShapeText.h"
#include "Shape/GroupCommand.h"
#include "Shape/TransformCommand.h"
#include "Serialization/DocumentTypes.h"
#include "Serialization/DocumentIO.h"
#include "XMenu.h"
#include "ToolManager.h"
#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QUndoStack>
#include <QThread>
#include <QEventLoop>
#include <QPointer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeySequence>
#include <QMimeData>
#include <QSignalBlocker>
#include <QUrl>
#include <QUuid>
#include <map>

#include "MyMath.h"

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : m_dScaleFactor(1.0),
      m_startPos(-1, -1),
      m_bDragging(false),
      m_canvas(new xcanvas::Canvas(this)),
      QGraphicsView{parent},
      m_bottomFloatingToolBar(nullptr),
      m_rightPressPos(-1, -1),
      m_rightDragged(false),
      m_pasteSerial(0),
      m_fileIoThread(new QThread(this)),
      m_fileIoContext(new QObject()),
      m_fileTaskRunning(false),
      m_fileTaskMessage(nullptr),
      m_keepAspectRatio(true),
      m_isDestroying(false)
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
    connect(m_colorPaletteWidget, &ColorPaletteWidget::colorSelected, this, &MyGraphicsView::onColorSelected);

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
    connect(m_canvas->undoStack(), &QUndoStack::cleanChanged, this, [this]() { updateWindowTitle(); });
    connect(m_canvas->undoStack(), &QUndoStack::indexChanged, this, [this](int) {
        if (m_isDestroying)
        {
            return;
        }
        updateSelectionHud();
        updateSelectionHudBarPos();
        requestFullUpdate();
    });
    connect(&EventBus::instance(), &EventBus::newFileRequested, this, &MyGraphicsView::onNewDocument);
    connect(&EventBus::instance(), &EventBus::importFileRequested, this, &MyGraphicsView::importFile);
    connect(&EventBus::instance(), &EventBus::openFileRequested, this, &MyGraphicsView::onOpenDocument);
    connect(&EventBus::instance(), &EventBus::saveFileRequested, this, &MyGraphicsView::onSaveDocument);
    connect(&EventBus::instance(), &EventBus::saveFileAsRequested, this, &MyGraphicsView::onSaveDocumentAs);

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
    connect(m_selectionHudBar, &SelectionHudBar::xEdited, this, &MyGraphicsView::applySelectionHudX);
    connect(m_selectionHudBar, &SelectionHudBar::yEdited, this, &MyGraphicsView::applySelectionHudY);
    connect(m_selectionHudBar, &SelectionHudBar::wEdited, this, &MyGraphicsView::applySelectionHudW);
    connect(m_selectionHudBar, &SelectionHudBar::hEdited, this, &MyGraphicsView::applySelectionHudH);
    connect(m_selectionHudBar, &SelectionHudBar::angleEdited, this, &MyGraphicsView::applySelectionHudAngle);
    connect(m_selectionHudBar, &SelectionHudBar::keepAspectRatioToggled, this, &MyGraphicsView::onKeepAspectRatioToggled);
    m_selectionHudBar->adjustSize();

    QTimer::singleShot(0, this, [this]() { fitCanvas(); });
    updateBottomFloatingToolBarPos();
    m_bottomFloatingToolBar->show();
    m_selectionHudBar->setVisible(false);

    m_rotateHandle.load(QStringLiteral(":/Resource/Icons/RotateHandle.svg"));
    updateWindowTitle();

    m_fileIoContext->moveToThread(m_fileIoThread);
    connect(m_fileIoThread, &QThread::finished, m_fileIoContext, &QObject::deleteLater);
    m_fileIoThread->start();
}

MyGraphicsView::~MyGraphicsView()
{
    m_isDestroying = true;
    if (m_canvas && m_canvas->undoStack())
    {
        disconnect(m_canvas->undoStack(), nullptr, this, nullptr);
    }
    clearCopiedShapes();
    closeFileTaskLoading();
    if (m_fileIoThread)
    {
        m_fileIoThread->quit();
        m_fileIoThread->wait();
    }
}

double MyGraphicsView::zoomValue()
{
    return transform().m11();
}

void MyGraphicsView::requestFullUpdate()
{
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
    if (event->button() == Qt::MiddleButton)
    {
        // 中键按住拖动画布
        m_startPos  = event->pos();
        m_bDragging = true;
        setCursor(Qt::ClosedHandCursor);// 设置为闭合手型光标
        event->accept();// 标记事件已处理
    }
    else if (event->button() == Qt::RightButton)
    {
        m_rightPressPos = event->pos();
        m_rightDragged  = false;
    }
    m_toolMgr->mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_bDragging)
    {
        // 计算鼠标移动的增量
        QPoint delta = event->pos() - m_startPos.toPoint();
        const QPoint totalRightDelta = event->pos() - m_rightPressPos;
        if (!m_rightDragged && (qAbs(totalRightDelta.x()) > 3 || qAbs(totalRightDelta.y()) > 3))
        {
            m_rightDragged = true;
        }

        // 水平滚动条：反向移动增量（因为视图移动方向与鼠标相反）
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());

        // 垂直滚动条：同上
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        // 更新记录的最后鼠标位置
        m_startPos = event->pos();
        event->accept();
    }

    if (event->buttons() & Qt::RightButton)
    {
        const QPoint totalRightDelta = event->pos() - m_rightPressPos;
        if (!m_rightDragged && (qAbs(totalRightDelta.x()) > 3 || qAbs(totalRightDelta.y()) > 3))
        {
            m_rightDragged = true;
        }
    }

    m_toolMgr->mouseMoveEvent(event);
    emit mouseMovePos(event->pos());
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    bool shouldOpenContextMenu = false;
    if (event->button() == Qt::MiddleButton && m_bDragging)
    {
        m_bDragging = false;
        setCursor(Qt::ArrowCursor);// 恢复默认光标
        event->accept();
    }
    else if (event->button() == Qt::RightButton)
    {
        shouldOpenContextMenu = !m_rightDragged && m_toolMgr && m_toolMgr->currentTool() == DrawingToolType::Select;
        m_rightDragged = false;
    }
    m_toolMgr->mouseReleaseEvent(event);
    if (shouldOpenContextMenu)
    {
        showCanvasContextMenu(event->pos());
    }
}

void MyGraphicsView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_G && event->modifiers().testFlag(Qt::ControlModifier))
    {
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            ungroupSelectedShapes();
        }
        else
        {
            groupSelectedShapes();
        }
        event->accept();
        return;
    }

    if (event->matches(QKeySequence::Cut))
    {
        cutSelectedShapes();
        event->accept();
        return;
    }

    if (event->matches(QKeySequence::Copy))
    {
        copySelectedShapes();
        event->accept();
        return;
    }

    if (event->matches(QKeySequence::Paste))
    {
        pasteCopiedShapes();
        event->accept();
        return;
    }

    if (event->matches(QKeySequence::SelectAll))
    {
        if (m_canvas && m_canvas->shapeManager())
        {
            m_canvas->shapeManager()->selectAll();
            requestFullUpdate();
        }
        event->accept();
        return;
    }

    m_toolMgr->keyPressEvent(event);
    QGraphicsView::keyPressEvent(event);
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    const Qt::KeyboardModifiers modifiers = event->modifiers();
    const QPoint                pixelDelta = event->pixelDelta();
    const QPoint                angleDelta = event->angleDelta();

    if (modifiers & Qt::ControlModifier)
    {
        const QPointF cursorViewPos = mapFromGlobal(QCursor::pos());
        const int     dy            = angleDelta.y() != 0 ? angleDelta.y() : pixelDelta.y();
        if (dy > 0)
        {
            zoomIn(cursorViewPos);
        }
        else if (dy < 0)
        {
            zoomOut(cursorViewPos);
        }
        event->accept();
        return;
    }

    auto scrollStep = [&](const bool horizontal)
    {
        QScrollBar* bar = horizontal ? horizontalScrollBar() : verticalScrollBar();
        if (!bar)
        {
            return;
        }

        int delta = 0;
        if (!pixelDelta.isNull())
        {
            delta = horizontal ? pixelDelta.x() : pixelDelta.y();
        }
        else if (!angleDelta.isNull())
        {
            const int angle = horizontal
                                  ? (angleDelta.x() != 0 ? angleDelta.x() : angleDelta.y())
                                  : (angleDelta.y() != 0 ? angleDelta.y() : angleDelta.x());
            delta = angle / 3;
        }

        if (delta != 0)
        {
            bar->setValue(bar->value() - delta);
        }
    };

    if (modifiers & Qt::ShiftModifier)
    {
        scrollStep(true);
    }
    else
    {
        scrollStep(false);
    }

    event->accept();
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

void MyGraphicsView::onNewDocument()
{
    if (!maybeSaveBeforeProceed())
    {
        return;
    }

    resetToNewDocument();
}

void MyGraphicsView::onOpenDocument()
{
    if (m_fileTaskRunning)
    {
        MessageWidget::showWarning(window(), tr("正在处理文件，请稍候。"));
        return;
    }

    if (!maybeSaveBeforeProceed())
    {
        return;
    }

    const QString filter = tr("XCanvas File (*%1)")
                               .arg(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
    const QString path = QFileDialog::getOpenFileName(this, tr("打开文件"), QString(), filter);
    if (path.isEmpty())
    {
        return;
    }

    openDocumentFileAsync(path);
}

void MyGraphicsView::onSaveDocument()
{
    if (m_fileTaskRunning)
    {
        MessageWidget::showWarning(window(), tr("正在处理文件，请稍候。"));
        return;
    }

    if (m_currentDocumentPath.isEmpty())
    {
        onSaveDocumentAs();
        return;
    }

    saveDocumentFileAsync(m_currentDocumentPath, false);
}

void MyGraphicsView::onSaveDocumentAs()
{
    if (m_fileTaskRunning)
    {
        MessageWidget::showWarning(window(), tr("正在处理文件，请稍候。"));
        return;
    }

    const QString filter = tr("XCanvas File (*%1)")
                               .arg(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
    QString path = QFileDialog::getSaveFileName(this, tr("另存为"), m_currentDocumentPath, filter);
    if (path.isEmpty())
    {
        return;
    }

    if (!path.endsWith(QString::fromLatin1(xcanvas::serialization::kDocumentExtension), Qt::CaseInsensitive)
        && QFileInfo(path).suffix().isEmpty())
    {
        path += QString::fromLatin1(xcanvas::serialization::kDocumentExtension);
    }

    saveDocumentFileAsync(path, true);
}

bool MyGraphicsView::openDocumentFile(const QString& path)
{
    openDocumentFileAsync(path);
    return true;
}

bool MyGraphicsView::saveDocumentFile(const QString& path)
{
    saveDocumentFileAsync(path, true);
    return true;
}

void MyGraphicsView::setFileActionsEnabled(const bool enabled)
{
    emit EventBus::instance().fileActionsEnabledChanged(enabled);
}

void MyGraphicsView::showFileTaskLoading(const QString& text)
{
    closeFileTaskLoading();
    m_fileTaskMessage = MessageWidget::showLoading(window(), text);
}

void MyGraphicsView::closeFileTaskLoading()
{
    if (!m_fileTaskMessage)
    {
        return;
    }
    MessageWidget::removeMessage(m_fileTaskMessage);
    m_fileTaskMessage = nullptr;
}

void MyGraphicsView::openDocumentFileAsync(const QString& path)
{
    if (!m_canvas || !m_fileIoContext || !m_fileIoThread || m_fileTaskRunning)
    {
        return;
    }

    m_fileTaskRunning = true;
    setFileActionsEnabled(false);
    showFileTaskLoading(tr("正在打开工程..."));

    const QPointer<MyGraphicsView> self(this);
    const QString filePath = path;
    QMetaObject::invokeMethod(m_fileIoContext, [self, filePath]()
    {
        xcanvas::serialization::LoadedDocument loaded;
        QString                                err;
        const bool ok = xcanvas::serialization::readDocument(filePath, &loaded, &err);

        QMetaObject::invokeMethod(self, [self, filePath, ok, err, loaded = std::move(loaded)]() mutable
        {
            if (!self)
            {
                xcanvas::serialization::clearLoadedDocument(&loaded);
                return;
            }

            bool    applied = false;
            QString applyErr;
            if (ok)
            {
                applied = xcanvas::serialization::applyDocumentToCanvas(self->m_canvas, std::move(loaded), &applyErr);
            }
            xcanvas::serialization::clearLoadedDocument(&loaded);

            self->closeFileTaskLoading();
            self->setFileActionsEnabled(true);
            self->m_fileTaskRunning = false;

            if (!ok || !applied)
            {
                const QString errorText = !ok ? err : applyErr;
                MessageWidget::showError(self->window(), errorText.isEmpty() ? self->tr("文件打开失败。") : errorText);
                return;
            }

            self->m_currentDocumentPath = filePath;
            self->clearCopiedShapes();
            self->m_pasteSerial = 0;
            self->m_canvas->undoStack()->setClean();
            if (self->m_selectionHudBar)
            {
                self->m_selectionHudBar->setVisible(false);
            }
            self->updateWindowTitle();
            self->requestFullUpdate();
            MessageWidget::showSuccess(self->window(), self->tr("工程已打开"));
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

void MyGraphicsView::saveDocumentFileAsync(const QString& path, const bool updateCurrentPath)
{
    if (!m_canvas || !m_fileIoContext || !m_fileIoThread || m_fileTaskRunning)
    {
        return;
    }

    xcanvas::serialization::LoadedDocument snapshot;
    QString                                snapshotErr;
    if (!xcanvas::serialization::buildDocumentSnapshot(m_canvas, &snapshot, &snapshotErr))
    {
        xcanvas::serialization::clearLoadedDocument(&snapshot);
        MessageWidget::showError(window(), snapshotErr.isEmpty() ? tr("文件保存失败。") : snapshotErr);
        return;
    }

    m_fileTaskRunning = true;
    setFileActionsEnabled(false);
    showFileTaskLoading(tr("正在保存工程..."));

    const QPointer<MyGraphicsView> self(this);
    const QString filePath = path;
    QMetaObject::invokeMethod(m_fileIoContext, [self, filePath, updateCurrentPath, snapshot = std::move(snapshot)]() mutable
    {
        QString err;
        const bool ok = xcanvas::serialization::writeDocument(snapshot, filePath, &err);
        xcanvas::serialization::clearLoadedDocument(&snapshot);

        QMetaObject::invokeMethod(self, [self, filePath, updateCurrentPath, ok, err]()
        {
            if (!self)
            {
                return;
            }

            self->closeFileTaskLoading();
            self->setFileActionsEnabled(true);
            self->m_fileTaskRunning = false;

            if (!ok)
            {
                MessageWidget::showError(self->window(), err.isEmpty() ? self->tr("文件保存失败。") : err);
                return;
            }

            if (updateCurrentPath || self->m_currentDocumentPath.isEmpty())
            {
                self->m_currentDocumentPath = filePath;
            }
            if (self->m_canvas->undoStack())
            {
                self->m_canvas->undoStack()->setClean();
            }
            self->updateWindowTitle();
            MessageWidget::showSuccess(self->window(), self->tr("工程已保存"));
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

bool MyGraphicsView::saveDocumentFileBlocking(const QString& path)
{
    if (!m_canvas || !m_fileIoContext || !m_fileIoThread || m_fileTaskRunning)
    {
        return false;
    }

    xcanvas::serialization::LoadedDocument snapshot;
    QString err;
    if (!xcanvas::serialization::buildDocumentSnapshot(m_canvas, &snapshot, &err))
    {
        xcanvas::serialization::clearLoadedDocument(&snapshot);
        MessageWidget::showError(window(), err.isEmpty() ? tr("文件保存失败。") : err);
        return false;
    }

    bool ok = false;
    m_fileTaskRunning = true;
    showFileTaskLoading(tr("正在保存工程..."));
    setFileActionsEnabled(false);

    QEventLoop loop;
    QMetaObject::invokeMethod(m_fileIoContext, [this, &snapshot, &ok, &err, &loop, path]()
    {
        ok = xcanvas::serialization::writeDocument(snapshot, path, &err);
        xcanvas::serialization::clearLoadedDocument(&snapshot);
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
    }, Qt::QueuedConnection);
    loop.exec();

    closeFileTaskLoading();
    setFileActionsEnabled(true);
    m_fileTaskRunning = false;

    if (!ok)
    {
        MessageWidget::showError(window(), err.isEmpty() ? tr("文件保存失败。") : err);
        return false;
    }

    m_currentDocumentPath = path;
    if (m_canvas->undoStack())
    {
        m_canvas->undoStack()->setClean();
    }
    updateWindowTitle();
    MessageWidget::showSuccess(window(), tr("工程已保存"));
    return true;
}

void MyGraphicsView::resetToNewDocument()
{
    if (!m_canvas)
    {
        return;
    }

    m_canvas->shapeManager()->clear();
    m_canvas->layerManager()->clearAllLayers();
    m_canvas->undoStack()->clear();
    m_canvas->undoStack()->setClean();

    m_currentDocumentPath.clear();
    clearCopiedShapes();
    m_pasteSerial = 0;
    if (m_selectionHudBar)
    {
        m_selectionHudBar->setVisible(false);
    }
    updateWindowTitle();
    requestFullUpdate();
}

bool MyGraphicsView::maybeSaveBeforeProceed()
{
    if (!m_canvas || !m_canvas->undoStack() || m_canvas->undoStack()->isClean())
    {
        return true;
    }

    const MessageBox::ClickedButton result = MessageBox::ask(
        this,
        tr("内容未保存"),
        tr("当前工程已修改，是否先保存？"),
        tr("保存"),
        tr("不保存"));

    if (result == MessageBox::ClickedButton::Secondary)
    {
        return true;
    }

    if (result != MessageBox::ClickedButton::Primary)
    {
        return false;
    }

    if (m_currentDocumentPath.isEmpty())
    {
        const QString filter = tr("XCanvas File (*%1)")
                                   .arg(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
        QString path = QFileDialog::getSaveFileName(this, tr("保存工程"), QString(), filter);
        if (path.isEmpty())
        {
            return false;
        }

        if (!path.endsWith(QString::fromLatin1(xcanvas::serialization::kDocumentExtension), Qt::CaseInsensitive)
            && QFileInfo(path).suffix().isEmpty())
        {
            path += QString::fromLatin1(xcanvas::serialization::kDocumentExtension);
        }
        return saveDocumentFileBlocking(path);
    }

    return saveDocumentFileBlocking(m_currentDocumentPath);
}

bool MyGraphicsView::maybeSaveBeforeClose()
{
    return maybeSaveBeforeProceed();
}

bool MyGraphicsView::isProjectFilePath(const QString& path) const
{
    const QString lowerPath = path.toLower();
    return lowerPath.endsWith(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
}

QString MyGraphicsView::projectDisplayName() const
{
    if (m_currentDocumentPath.isEmpty())
    {
        return QStringLiteral("untitled");
    }

    const QString fileName = QFileInfo(m_currentDocumentPath).fileName();
    const QString baseName = QFileInfo(fileName).completeBaseName();
    if (baseName.isEmpty())
    {
        return QStringLiteral("untitled");
    }
    return baseName;
}

void MyGraphicsView::updateWindowTitle()
{
    QWidget* topLevel = window();
    if (!topLevel || !m_canvas || !m_canvas->undoStack())
    {
        return;
    }

    QString title = projectDisplayName();
    if (!m_canvas->undoStack()->isClean())
    {
        title += QStringLiteral("*");
    }
    topLevel->setWindowTitle(title);
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

void MyGraphicsView::onColorSelected(const QColor& color)
{
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

void MyGraphicsView::applySelectionHudX(const double newX)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  currentX      = sceneToCanvas(selectionRect.topLeft()).x();
    const qreal  dx            = newX - currentX;
    if (qAbs(dx) < 1e-6)
    {
        return;
    }

    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        shape->translate(QPointF(dx, 0));
    }

    if (beforeTransform.empty())
    {
        return;
    }

    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), QStringLiteral("Move Selection X")));
    requestFullUpdate();
    updateSelectionHud();
}

void MyGraphicsView::applySelectionHudY(const double newY)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  currentY      = sceneToCanvas(selectionRect.topLeft()).y();
    const qreal  dy            = newY - currentY;
    if (qAbs(dy) < 1e-6)
    {
        return;
    }

    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        shape->translate(QPointF(0, dy));
    }

    if (beforeTransform.empty())
    {
        return;
    }

    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), QStringLiteral("Move Selection Y")));
    requestFullUpdate();
    updateSelectionHud();
}

void MyGraphicsView::applySelectionHudW(const double newW)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack() || newW <= 0.0)
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  oldW          = selectionRect.width();
    if (oldW < 1e-6)
    {
        return;
    }

    const qreal sx = newW / oldW;
    if (qAbs(sx - 1.0) < 1e-6)
    {
        return;
    }

    const QPointF anchor = selectionRect.topLeft();
    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        if (m_keepAspectRatio)
        {
            shape->scale(sx, sx, anchor);
        }
        else
        {
            shape->scale(sx, 1.0, anchor);
        }
    }

    if (beforeTransform.empty())
    {
        return;
    }

    const QString cmdText = m_keepAspectRatio ? QStringLiteral("Resize Selection (Keep Ratio)") : QStringLiteral("Resize Selection Width");
    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), cmdText));
    requestFullUpdate();
    updateSelectionHud();
}

void MyGraphicsView::applySelectionHudH(const double newH)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack() || newH <= 0.0)
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    const QRectF selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const qreal  oldH          = selectionRect.height();
    if (oldH < 1e-6)
    {
        return;
    }

    const qreal sy = newH / oldH;
    if (qAbs(sy - 1.0) < 1e-6)
    {
        return;
    }

    const QPointF anchor = selectionRect.topLeft();
    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeTransform[shape] = shape->transform();
        if (m_keepAspectRatio)
        {
            shape->scale(sy, sy, anchor);
        }
        else
        {
            shape->scale(1.0, sy, anchor);
        }
    }

    if (beforeTransform.empty())
    {
        return;
    }

    const QString cmdText = m_keepAspectRatio ? QStringLiteral("Resize Selection (Keep Ratio)") : QStringLiteral("Resize Selection Height");
    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), cmdText));
    requestFullUpdate();
    updateSelectionHud();
}

void MyGraphicsView::applySelectionHudAngle(const double newAngle)
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return;
    }

    std::map<xcanvas::Shape*, QTransform> beforeTransform;
    if (selectedShapes.size() == 1)
    {
        xcanvas::Shape* shape = selectedShapes.first();
        if (!shape)
        {
            return;
        }

        qreal delta = newAngle - shape->rotationDeg();
        if (delta > 180.0)
        {
            delta -= 360.0;
        }
        else if (delta < -180.0)
        {
            delta += 360.0;
        }
        if (qAbs(delta) < 1e-6)
        {
            return;
        }

        beforeTransform[shape] = shape->transform();
        shape->rotate(delta, shape->boundingRect().center());
    }
    else
    {
        const qreal delta = newAngle;
        if (qAbs(delta) < 1e-6)
        {
            return;
        }

        const QPointF center = m_canvas->shapeManager()->selectedBoundingRect().center();
        for (xcanvas::Shape* shape : selectedShapes)
        {
            if (!shape)
            {
                continue;
            }
            beforeTransform[shape] = shape->transform();
            shape->rotate(delta, center);
        }
    }

    if (beforeTransform.empty())
    {
        return;
    }

    m_canvas->undoStack()->push(new xcanvas::TransformCommand(m_canvas->shapeManager(), std::move(beforeTransform), QStringLiteral("Rotate Selection")));
    requestFullUpdate();
    updateSelectionHud();
}

void MyGraphicsView::onKeepAspectRatioToggled(const bool enabled)
{
    m_keepAspectRatio = enabled;
}

void MyGraphicsView::updateSelectionHud()
{
    if (m_isDestroying || !m_selectionHudBar || !m_canvas || !m_canvas->shapeManager() || !m_selectionHudBar->isVisible())
    {
        return;
    }

    const QRectF  selectionRect = m_canvas->shapeManager()->selectedBoundingRect();
    const QPointF canvasPos     = sceneToCanvas(selectionRect.topLeft());
    const QSignalBlocker blockerX(m_selectionHudBar->spinX());
    const QSignalBlocker blockerY(m_selectionHudBar->spinY());
    const QSignalBlocker blockerW(m_selectionHudBar->spinW());
    const QSignalBlocker blockerH(m_selectionHudBar->spinH());
    const QSignalBlocker blockerAngle(m_selectionHudBar->spinAngle());

    m_selectionHudBar->spinX()->setValue(canvasPos.x());
    m_selectionHudBar->spinY()->setValue(canvasPos.y());
    m_selectionHudBar->spinW()->setValue(selectionRect.width());
    m_selectionHudBar->spinH()->setValue(selectionRect.height());
    if (const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList(); selectedShapes.size() == 1 && selectedShapes.first())
    {
        m_selectionHudBar->spinAngle()->setValue(selectedShapes.first()->rotationDeg());
    }
    else
    {
        m_selectionHudBar->spinAngle()->setValue(0.0);
    }
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
    if (m_fileTaskRunning)
    {
        MessageWidget::showWarning(window(), tr("正在处理文件，请稍候。"));
        return;
    }

    const QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Import Files"), QString(), ImportManager::instance().buildDialogFilter());
    importFiles(filePaths);
}

void MyGraphicsView::importFiles(const QStringList& filePaths, const QPointF& targetCenter)
{
    if (filePaths.isEmpty() || !m_fileIoContext || !m_fileIoThread || m_fileTaskRunning)
    {
        return;
    }

    m_fileTaskRunning = true;
    setFileActionsEnabled(false);
    showFileTaskLoading(tr("正在导入文件..."));

    const QPointer<MyGraphicsView> self(this);
    const QStringList paths = filePaths;
    QMetaObject::invokeMethod(m_fileIoContext, [self, paths, targetCenter]()
    {
        ImportContext       ctx;
        ctx.targetCenter = targetCenter;

        xcanvas::ShapeList allShapes;
        for (const QString& filePath : paths)
        {
            xcanvas::ShapeList shapeList = ImportManager::instance().importFile(filePath, ctx);
            if (!shapeList.empty())
            {
                allShapes.append(shapeList);
            }
        }

        QMetaObject::invokeMethod(self, [self, targetCenter, allShapes = std::move(allShapes)]() mutable
        {
            if (!self)
            {
                for (xcanvas::Shape* shape : allShapes)
                {
                    delete shape;
                }
                return;
            }

            self->closeFileTaskLoading();
            self->setFileActionsEnabled(true);
            self->m_fileTaskRunning = false;

            if (allShapes.isEmpty())
            {
                MessageWidget::showWarning(self->window(), self->tr("未导入到可用图形。"));
                return;
            }

            self->m_canvas->shapeManager()->deselectAll();

            QRectF rect;
            for (auto* shape : allShapes)
            {
                rect |= shape->boundingRect();
            }

            const QPointF offset = targetCenter - rect.center();
            for (auto* shape : allShapes)
            {
                shape->translate(offset);
            }

            self->m_canvas->addShapes(allShapes);
            self->requestFullUpdate();
            MessageWidget::showSuccess(self->window(), self->tr("导入完成"));
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
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
    if (m_isDestroying || !m_selectionHudBar || !m_selectionHudBar->isVisible())
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
                if (isProjectFilePath(filePath) || ImportManager::instance().canImport(filePath))
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
                if (isProjectFilePath(filePath) || ImportManager::instance().canImport(filePath))
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
    QStringList projectPaths;
    QStringList filePaths;
    for (const QUrl& url : urls)
    {
        if (url.isLocalFile())
        {
            const QString path = url.toLocalFile();
            if (isProjectFilePath(path))
            {
                projectPaths.append(path);
            }
            else
            {
                filePaths.append(path);
            }
        }
    }

    if (projectPaths.isEmpty() && filePaths.isEmpty())
    {
        event->ignore();
        return;
    }

    event->acceptProposedAction();
    if (!projectPaths.isEmpty())
    {
        if (!maybeSaveBeforeProceed())
        {
            return;
        }
        openDocumentFile(projectPaths.first());
        return;
    }

    const QPointF mouseScenePos = mapToScene(event->position().toPoint());
    importFiles(filePaths, mouseScenePos);
}

void MyGraphicsView::showCanvasContextMenu(const QPoint& viewPos)
{
    const bool canCopy = m_canvas && m_canvas->shapeManager() && m_canvas->shapeManager()->hasSelection();
    const bool canCut = canCopy;
    const bool canPaste = hasClipboardPasteContent() || !m_copiedShapes.isEmpty();
    const bool canDelete = canCopy;
    const xcanvas::ShapeList selectedShapes = (m_canvas && m_canvas->shapeManager()) ? m_canvas->shapeManager()->selectedShapeList() : xcanvas::ShapeList();
    bool allSameNonEmptyGroup = selectedShapes.size() >= 2;
    QString firstGroupId;
    bool firstGroupSet = false;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape || shape->groupId().isEmpty())
        {
            allSameNonEmptyGroup = false;
            break;
        }
        if (!firstGroupSet)
        {
            firstGroupId = shape->groupId();
            firstGroupSet = true;
            continue;
        }
        if (shape->groupId() != firstGroupId)
        {
            allSameNonEmptyGroup = false;
            break;
        }
    }
    const bool canGroup = selectedShapes.size() >= 2 && !allSameNonEmptyGroup;
    bool canUngroup = false;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (shape && !shape->groupId().isEmpty())
        {
            canUngroup = true;
            break;
        }
    }

    XMenu menu(this);

    QAction* cutAction = nullptr;
    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* deleteAction = nullptr;
    QAction* groupAction = nullptr;
    QAction* ungroupAction = nullptr;
    QAction* selectAllAction = nullptr;
    QAction* zoomInAction = nullptr;
    QAction* zoomOutAction = nullptr;

    if (canCut)
    {
        cutAction = menu.addAction(tr("剪切"));
        cutAction->setShortcut(QKeySequence::Cut);
        cutAction->setShortcutVisibleInContextMenu(true);
    }

    if (canCopy)
    {
        copyAction = menu.addAction(tr("复制"));
        copyAction->setShortcut(QKeySequence::Copy);
        copyAction->setShortcutVisibleInContextMenu(true);
    }

    if (canPaste)
    {
        pasteAction = menu.addAction(tr("粘贴"));
        pasteAction->setShortcut(QKeySequence::Paste);
        pasteAction->setShortcutVisibleInContextMenu(true);
    }

    if (canDelete)
    {
        deleteAction = menu.addAction(tr("删除"));
        deleteAction->setShortcut(QKeySequence::Delete);
        deleteAction->setShortcutVisibleInContextMenu(true);
    }

    if (canGroup)
    {
        groupAction = menu.addAction(tr("成组"));
        groupAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
        groupAction->setShortcutVisibleInContextMenu(true);
    }

    if (canUngroup)
    {
        ungroupAction = menu.addAction(tr("取消成组"));
        ungroupAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
        ungroupAction->setShortcutVisibleInContextMenu(true);
    }

    menu.addSeparator();

    selectAllAction = menu.addAction(tr("全选"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutVisibleInContextMenu(true);

    zoomInAction = menu.addAction(tr("放大"));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    zoomInAction->setShortcutVisibleInContextMenu(true);

    zoomOutAction = menu.addAction(tr("缩小"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    zoomOutAction->setShortcutVisibleInContextMenu(true);

    QAction* result = menu.exec(mapToGlobal(viewPos));
    if (result == cutAction)
    {
        cutSelectedShapes();
    }
    else if (result == copyAction)
    {
        copySelectedShapes();
    }
    else if (result == pasteAction)
    {
        pasteCopiedShapesAt(mapToScene(viewPos));
    }
    else if (result == deleteAction)
    {
        deleteSelectedShapes();
    }
    else if (result == groupAction)
    {
        groupSelectedShapes();
    }
    else if (result == ungroupAction)
    {
        ungroupSelectedShapes();
    }
    else if (result == selectAllAction)
    {
        if (m_canvas && m_canvas->shapeManager())
        {
            m_canvas->shapeManager()->selectAll();
            requestFullUpdate();
        }
    }
    else if (result == zoomInAction)
    {
        onZoomIn();
    }
    else if (result == zoomOutAction)
    {
        onZoomOut();
    }
}

bool MyGraphicsView::copySelectedShapes()
{
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return false;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return false;
    }

    clearCopiedShapes();
    m_copiedShapes.reserve(selectedShapes.size());

    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        if (xcanvas::Shape* cloned = shape->clone())
        {
            m_copiedShapes.append(cloned);
        }
    }

    m_pasteSerial = 0;
    if (QClipboard* clipboard = QGuiApplication::clipboard())
    {
        clipboard->clear();
    }
    return !m_copiedShapes.isEmpty();
}

bool MyGraphicsView::cutSelectedShapes()
{
    if (!copySelectedShapes())
    {
        return false;
    }

    return deleteSelectedShapes();
}

bool MyGraphicsView::pasteCopiedShapes()
{
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return false;
    }

    const QPointF centerPos = mapToScene(viewport()->rect().center());
    if (pasteFromClipboard(centerPos))
    {
        return true;
    }

    if (m_copiedShapes.isEmpty())
    {
        return false;
    }

    xcanvas::ShapeList shapesToAdd;
    shapesToAdd.reserve(m_copiedShapes.size());

    for (xcanvas::Shape* shape : m_copiedShapes)
    {
        if (!shape)
        {
            continue;
        }
        if (xcanvas::Shape* cloned = shape->clone())
        {
            shapesToAdd.append(cloned);
        }
    }

    if (shapesToAdd.isEmpty())
    {
        return false;
    }

    std::map<QString, QString> groupRemap;
    for (xcanvas::Shape* shape : shapesToAdd)
    {
        const QString oldGroupId = shape->groupId();
        if (oldGroupId.isEmpty())
        {
            continue;
        }
        auto it = groupRemap.find(oldGroupId);
        if (it == groupRemap.end())
        {
            it = groupRemap.emplace(oldGroupId, QUuid::createUuid().toString(QUuid::WithoutBraces)).first;
        }
        shape->setGroupId(it->second);
    }

    const QPointF offset = QPointF(20.0 * (m_pasteSerial + 1), 20.0 * (m_pasteSerial + 1));
    ++m_pasteSerial;
    for (xcanvas::Shape* shape : shapesToAdd)
    {
        shape->translate(offset);
    }

    m_canvas->shapeManager()->deselectAll();
    m_canvas->addShapes(shapesToAdd);
    requestFullUpdate();
    return true;
}

bool MyGraphicsView::pasteCopiedShapesAt(const QPointF& scenePos)
{
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return false;
    }

    if (pasteFromClipboard(scenePos))
    {
        return true;
    }

    if (m_copiedShapes.isEmpty())
    {
        return false;
    }

    xcanvas::ShapeList shapesToAdd;
    shapesToAdd.reserve(m_copiedShapes.size());

    QRectF unionRect;
    bool hasRect = false;
    for (xcanvas::Shape* shape : m_copiedShapes)
    {
        if (!shape)
        {
            continue;
        }
        if (xcanvas::Shape* cloned = shape->clone())
        {
            const QRectF rect = cloned->boundingRect();
            if (!hasRect)
            {
                unionRect = rect;
                hasRect = true;
            }
            else
            {
                unionRect |= rect;
            }
            shapesToAdd.append(cloned);
        }
    }

    if (shapesToAdd.isEmpty())
    {
        return false;
    }

    std::map<QString, QString> groupRemap;
    for (xcanvas::Shape* shape : shapesToAdd)
    {
        const QString oldGroupId = shape->groupId();
        if (oldGroupId.isEmpty())
        {
            continue;
        }
        auto it = groupRemap.find(oldGroupId);
        if (it == groupRemap.end())
        {
            it = groupRemap.emplace(oldGroupId, QUuid::createUuid().toString(QUuid::WithoutBraces)).first;
        }
        shape->setGroupId(it->second);
    }

    if (hasRect && unionRect.isValid())
    {
        const QPointF offset = scenePos - unionRect.center();
        for (xcanvas::Shape* shape : shapesToAdd)
        {
            shape->translate(offset);
        }
    }

    m_canvas->shapeManager()->deselectAll();
    m_canvas->addShapes(shapesToAdd);
    requestFullUpdate();
    ++m_pasteSerial;
    return true;
}

bool MyGraphicsView::deleteSelectedShapes()
{
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return false;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return false;
    }

    m_canvas->removeShapes(selectedShapes);
    requestFullUpdate();
    return true;
}

bool MyGraphicsView::groupSelectedShapes()
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return false;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.size() < 2)
    {
        return false;
    }

    bool allSameNonEmptyGroup = true;
    QString firstGroupId;
    bool firstGroupSet = false;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape || shape->groupId().isEmpty())
        {
            allSameNonEmptyGroup = false;
            break;
        }
        if (!firstGroupSet)
        {
            firstGroupId = shape->groupId();
            firstGroupSet = true;
            continue;
        }
        if (shape->groupId() != firstGroupId)
        {
            allSameNonEmptyGroup = false;
            break;
        }
    }
    if (allSameNonEmptyGroup)
    {
        return false;
    }

    const QString newGroupId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    std::map<xcanvas::Shape*, QString> beforeGroupId;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape)
        {
            continue;
        }
        beforeGroupId[shape] = shape->groupId();
        shape->setGroupId(newGroupId);
    }

    if (beforeGroupId.empty())
    {
        return false;
    }

    m_canvas->undoStack()->push(new xcanvas::GroupCommand(m_canvas->shapeManager(), std::move(beforeGroupId), tr("Group Shapes")));
    requestFullUpdate();
    return true;
}

bool MyGraphicsView::ungroupSelectedShapes()
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_canvas->undoStack())
    {
        return false;
    }

    const xcanvas::ShapeList selectedShapes = m_canvas->shapeManager()->selectedShapeList();
    if (selectedShapes.isEmpty())
    {
        return false;
    }

    std::map<xcanvas::Shape*, QString> beforeGroupId;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape || shape->groupId().isEmpty())
        {
            continue;
        }
        beforeGroupId[shape] = shape->groupId();
        shape->setGroupId(QString());
    }

    if (beforeGroupId.empty())
    {
        return false;
    }

    m_canvas->undoStack()->push(new xcanvas::GroupCommand(m_canvas->shapeManager(), std::move(beforeGroupId), tr("Ungroup Shapes")));
    requestFullUpdate();
    return true;
}

bool MyGraphicsView::pasteFromClipboard(const QPointF& scenePos)
{
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return false;
    }

    const QClipboard* clipboard = QGuiApplication::clipboard();
    if (!clipboard)
    {
        return false;
    }

    const QMimeData* mimeData = clipboard->mimeData();
    if (!mimeData)
    {
        return false;
    }

    const QPointF pasteOffset = QPointF(20.0 * (m_pasteSerial + 1), 20.0 * (m_pasteSerial + 1));
    const QPointF targetPos = scenePos + pasteOffset;

    QStringList importPaths;

    if (mimeData->hasUrls())
    {
        for (const QUrl& url : mimeData->urls())
        {
            if (!url.isLocalFile())
            {
                continue;
            }

            const QString localFile = url.toLocalFile();
            if (ImportManager::instance().canImport(localFile))
            {
                importPaths.append(localFile);
            }
        }
    }

    if (importPaths.isEmpty() && mimeData->hasText())
    {
        const QStringList lines = mimeData->text().split('\n', Qt::SkipEmptyParts);
        for (const QString& rawLine : lines)
        {
            QString line = rawLine.trimmed();
            if (line.isEmpty())
            {
                continue;
            }
            if (line.startsWith('"') && line.endsWith('"') && line.size() > 1)
            {
                line = line.mid(1, line.size() - 2);
            }

            QString localFile = line;
            const QUrl userUrl = QUrl::fromUserInput(line);
            if (userUrl.isValid() && userUrl.isLocalFile())
            {
                localFile = userUrl.toLocalFile();
            }

            if (QFileInfo::exists(localFile) && ImportManager::instance().canImport(localFile))
            {
                importPaths.append(localFile);
            }
        }
    }

    if (!importPaths.isEmpty())
    {
        importFiles(importPaths, targetPos);
        ++m_pasteSerial;
        return true;
    }

    if (mimeData->hasImage())
    {
        const QVariant imageData = mimeData->imageData();
        if (imageData.canConvert<QImage>())
        {
            const QImage image = imageData.value<QImage>();
            if (!image.isNull())
            {
                auto* shape = new xcanvas::ShapeImage(image);
                shape->setSize(image.size());
                shape->translate(targetPos - QPointF(image.width() / 2.0, image.height() / 2.0));
                m_canvas->shapeManager()->deselectAll();
                m_canvas->addShape(shape);
                m_canvas->shapeManager()->selectShape(shape, true);
                requestFullUpdate();
                ++m_pasteSerial;
                return true;
            }
        }
    }

    if (mimeData->hasText())
    {
        const QString text = mimeData->text();
        if (!text.trimmed().isEmpty())
        {
            QFont font;
            font.setFamily("MiSans");
            font.setPixelSize(24);

            auto* shape = new xcanvas::ShapeText();
            shape->setText(text);
            shape->setFont(font);
            shape->setColor(AppSettings::instance().activeColor());
            shape->translate(targetPos);

            m_canvas->shapeManager()->deselectAll();
            m_canvas->addShape(shape);
            m_canvas->shapeManager()->selectShape(shape, true);
            requestFullUpdate();
            ++m_pasteSerial;
            return true;
        }
    }

    return false;
}

bool MyGraphicsView::hasClipboardPasteContent() const
{
    const QClipboard* clipboard = QGuiApplication::clipboard();
    if (!clipboard)
    {
        return false;
    }

    const QMimeData* mimeData = clipboard->mimeData();
    if (!mimeData)
    {
        return false;
    }

    if (mimeData->hasUrls())
    {
        for (const QUrl& url : mimeData->urls())
        {
            if (url.isLocalFile() && ImportManager::instance().canImport(url.toLocalFile()))
            {
                return true;
            }
        }
    }

    if (mimeData->hasImage())
    {
        const QVariant imageData = mimeData->imageData();
        if (imageData.canConvert<QImage>() && !imageData.value<QImage>().isNull())
        {
            return true;
        }
    }

    if (mimeData->hasText())
    {
        return !mimeData->text().trimmed().isEmpty();
    }

    return false;
}

void MyGraphicsView::clearCopiedShapes()
{
    qDeleteAll(m_copiedShapes);
    m_copiedShapes.clear();
}
