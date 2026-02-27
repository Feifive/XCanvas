#include "DocumentIoController.h"

#include "AsyncFileTaskRunner.h"
#include "../Canvas/Canvas.h"
#include "ClipboardCommandService.h"
#include "DocumentSessionController.h"
#include "../Import/ImportManager.h"
#include "MessageWidget.h"
#include "../SelectionHudBar.h"
#include "../Serialization/DocumentIO.h"

#include <QEventLoop>
#include <QPointer>
#include <QUndoStack>

DocumentIoController::DocumentIoController(
    QWidget* const                   hostView,
    xcanvas::Canvas* const           canvas,
    AsyncFileTaskRunner* const       fileTaskRunner,
    DocumentSessionController* const documentSessionController,
    ClipboardCommandService* const   clipboardCommandService,
    SelectionHudBar* const           selectionHudBar,
    VoidCallback                     requestFullUpdate,
    VoidCallback                     updateWindowTitle,
    QObject* const                   parent)
    : QObject(parent),
      m_hostView(hostView),
      m_canvas(canvas),
      m_fileTaskRunner(fileTaskRunner),
      m_documentSessionController(documentSessionController),
      m_clipboardCommandService(clipboardCommandService),
      m_selectionHudBar(selectionHudBar),
      m_requestFullUpdate(std::move(requestFullUpdate)),
      m_updateWindowTitle(std::move(updateWindowTitle))
{
}

void DocumentIoController::setClipboardCommandService(ClipboardCommandService* const clipboardCommandService)
{
    m_clipboardCommandService = clipboardCommandService;
}

void DocumentIoController::setSelectionHudBar(SelectionHudBar* const selectionHudBar)
{
    m_selectionHudBar = selectionHudBar;
}

void DocumentIoController::setDocumentSessionController(DocumentSessionController* const documentSessionController)
{
    m_documentSessionController = documentSessionController;
}

void DocumentIoController::openDocumentFileAsync(const QString& path)
{
    if (!m_canvas || !m_fileTaskRunner || !m_fileTaskRunner->beginTask(tr("正在打开工程...")))
    {
        return;
    }

    const QPointer<DocumentIoController> self(this);
    const QString                        filePath = path;
    QMetaObject::invokeMethod(m_fileTaskRunner->workerContext(), [self, filePath]()
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

            if (self->m_fileTaskRunner)
            {
                self->m_fileTaskRunner->endTask();
            }

            if (!ok || !applied)
            {
                const QString errorText = !ok ? err : applyErr;
                MessageWidget::showError(self->m_hostView ? self->m_hostView->window() : nullptr, errorText.isEmpty() ? self->tr("文件打开失败。") : errorText);
                return;
            }

            if (self->m_documentSessionController)
            {
                self->m_documentSessionController->setCurrentDocumentPath(filePath);
            }
            if (self->m_clipboardCommandService)
            {
                self->m_clipboardCommandService->resetState();
            }
            if (self->m_canvas && self->m_canvas->undoStack())
            {
                self->m_canvas->undoStack()->setClean();
            }
            if (self->m_selectionHudBar)
            {
                self->m_selectionHudBar->setVisible(false);
            }
            if (self->m_updateWindowTitle)
            {
                self->m_updateWindowTitle();
            }
            if (self->m_requestFullUpdate)
            {
                self->m_requestFullUpdate();
            }
            MessageWidget::showSuccess(self->m_hostView ? self->m_hostView->window() : nullptr, self->tr("工程已打开"));
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

void DocumentIoController::saveDocumentFileAsync(const QString& path, const bool updateCurrentPath)
{
    if (!m_canvas || !m_fileTaskRunner || m_fileTaskRunner->isTaskRunning())
    {
        return;
    }

    xcanvas::serialization::LoadedDocument snapshot;
    QString                                snapshotErr;
    if (!xcanvas::serialization::buildDocumentSnapshot(m_canvas, &snapshot, &snapshotErr))
    {
        xcanvas::serialization::clearLoadedDocument(&snapshot);
        MessageWidget::showError(m_hostView ? m_hostView->window() : nullptr, snapshotErr.isEmpty() ? tr("文件保存失败。") : snapshotErr);
        return;
    }

    if (!m_fileTaskRunner->beginTask(tr("正在保存工程...")))
    {
        xcanvas::serialization::clearLoadedDocument(&snapshot);
        return;
    }

    const QPointer<DocumentIoController> self(this);
    const QString                        filePath = path;
    QMetaObject::invokeMethod(m_fileTaskRunner->workerContext(), [self, filePath, updateCurrentPath, snapshot = std::move(snapshot)]() mutable
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

            if (self->m_fileTaskRunner)
            {
                self->m_fileTaskRunner->endTask();
            }

            if (!ok)
            {
                MessageWidget::showError(self->m_hostView ? self->m_hostView->window() : nullptr, err.isEmpty() ? self->tr("文件保存失败。") : err);
                return;
            }

            const bool hasCurrentPath = self->m_documentSessionController
                                            ? !self->m_documentSessionController->currentDocumentPath().isEmpty()
                                            : false;
            if (updateCurrentPath || !hasCurrentPath)
            {
                if (self->m_documentSessionController)
                {
                    self->m_documentSessionController->setCurrentDocumentPath(filePath);
                }
            }
            if (self->m_canvas && self->m_canvas->undoStack())
            {
                self->m_canvas->undoStack()->setClean();
            }
            if (self->m_updateWindowTitle)
            {
                self->m_updateWindowTitle();
            }
            MessageWidget::showSuccess(self->m_hostView ? self->m_hostView->window() : nullptr, self->tr("工程已保存"));
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

bool DocumentIoController::saveDocumentFileBlocking(const QString& path)
{
    if (!m_canvas || !m_fileTaskRunner || !m_fileTaskRunner->beginTask(tr("正在保存工程...")))
    {
        return false;
    }

    xcanvas::serialization::LoadedDocument snapshot;
    QString                                err;
    if (!xcanvas::serialization::buildDocumentSnapshot(m_canvas, &snapshot, &err))
    {
        xcanvas::serialization::clearLoadedDocument(&snapshot);
        m_fileTaskRunner->endTask();
        MessageWidget::showError(m_hostView ? m_hostView->window() : nullptr, err.isEmpty() ? tr("文件保存失败。") : err);
        return false;
    }

    bool ok = false;

    QEventLoop loop;
    QMetaObject::invokeMethod(m_fileTaskRunner->workerContext(), [&snapshot, &ok, &err, &loop, path]()
    {
        ok = xcanvas::serialization::writeDocument(snapshot, path, &err);
        xcanvas::serialization::clearLoadedDocument(&snapshot);
        QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
    }, Qt::QueuedConnection);
    loop.exec();

    m_fileTaskRunner->endTask();

    if (!ok)
    {
        MessageWidget::showError(m_hostView ? m_hostView->window() : nullptr, err.isEmpty() ? tr("文件保存失败。") : err);
        return false;
    }

    if (m_documentSessionController)
    {
        m_documentSessionController->setCurrentDocumentPath(path);
    }
    if (m_canvas && m_canvas->undoStack())
    {
        m_canvas->undoStack()->setClean();
    }
    if (m_updateWindowTitle)
    {
        m_updateWindowTitle();
    }
    MessageWidget::showSuccess(m_hostView ? m_hostView->window() : nullptr, tr("工程已保存"));
    return true;
}

void DocumentIoController::importFiles(const QStringList& filePaths, const QPointF& targetCenter)
{
    if (filePaths.isEmpty() || !m_fileTaskRunner || !m_fileTaskRunner->beginTask(tr("正在导入文件...")))
    {
        return;
    }

    const QPointer<DocumentIoController> self(this);
    const QStringList                    paths = filePaths;
    QMetaObject::invokeMethod(m_fileTaskRunner->workerContext(), [self, paths, targetCenter]()
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

            if (self->m_fileTaskRunner)
            {
                self->m_fileTaskRunner->endTask();
            }

            if (allShapes.isEmpty())
            {
                MessageWidget::showWarning(self->m_hostView ? self->m_hostView->window() : nullptr, self->tr("未导入到可用图形。"));
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
            if (self->m_requestFullUpdate)
            {
                self->m_requestFullUpdate();
            }
            MessageWidget::showSuccess(self->m_hostView ? self->m_hostView->window() : nullptr, self->tr("导入完成"));
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}
