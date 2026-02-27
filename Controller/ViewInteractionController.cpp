#include "ViewInteractionController.h"

#include "../Import/ImportManager.h"

#include <QCursor>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMimeData>
#include <QScrollBar>
#include <QUrl>
#include <QWheelEvent>
#include <QtMath>

ViewInteractionController::ViewInteractionController(
    QGraphicsView* const  view,
    BoolAction            isSelectTool,
    ShowMenuAction        showCanvasContextMenu,
    BoolAction            groupSelectedShapes,
    BoolAction            ungroupSelectedShapes,
    BoolAction            cutSelectedShapes,
    BoolAction            copySelectedShapes,
    BoolAction            pasteCopiedShapes,
    VoidAction            selectAllAction,
    ZoomAtAction          zoomInAction,
    ZoomAtAction          zoomOutAction,
    IsProjectPath         isProjectFilePath,
    BoolAction            maybeSaveBeforeProceed,
    OpenDocumentFile      openDocumentFile,
    ImportFilesAt         importFilesAt)
    : m_view(view),
      m_startPos(-1, -1),
      m_dragging(false),
      m_rightPressPos(-1, -1),
      m_rightDragged(false),
      m_isSelectTool(std::move(isSelectTool)),
      m_showCanvasContextMenu(std::move(showCanvasContextMenu)),
      m_groupSelectedShapes(std::move(groupSelectedShapes)),
      m_ungroupSelectedShapes(std::move(ungroupSelectedShapes)),
      m_cutSelectedShapes(std::move(cutSelectedShapes)),
      m_copySelectedShapes(std::move(copySelectedShapes)),
      m_pasteCopiedShapes(std::move(pasteCopiedShapes)),
      m_selectAllAction(std::move(selectAllAction)),
      m_zoomInAction(std::move(zoomInAction)),
      m_zoomOutAction(std::move(zoomOutAction)),
      m_isProjectFilePath(std::move(isProjectFilePath)),
      m_maybeSaveBeforeProceed(std::move(maybeSaveBeforeProceed)),
      m_openDocumentFile(std::move(openDocumentFile)),
      m_importFilesAt(std::move(importFilesAt))
{
}

void ViewInteractionController::mousePressEvent(QMouseEvent* event)
{
    if (!m_view || !event)
    {
        return;
    }

    if (event->button() == Qt::MiddleButton)
    {
        m_startPos = event->pos();
        m_dragging = true;
        m_view->setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    else if (event->button() == Qt::RightButton)
    {
        m_rightPressPos = event->pos();
        m_rightDragged  = false;
    }
}

void ViewInteractionController::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_view || !event)
    {
        return;
    }

    if (m_dragging)
    {
        const QPoint delta = event->pos() - m_startPos.toPoint();
        const QPoint totalRightDelta = event->pos() - m_rightPressPos;
        if (!m_rightDragged && (qAbs(totalRightDelta.x()) > 3 || qAbs(totalRightDelta.y()) > 3))
        {
            m_rightDragged = true;
        }

        if (QScrollBar* hbar = m_view->horizontalScrollBar())
        {
            hbar->setValue(hbar->value() - delta.x());
        }
        if (QScrollBar* vbar = m_view->verticalScrollBar())
        {
            vbar->setValue(vbar->value() - delta.y());
        }

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
}

bool ViewInteractionController::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_view || !event)
    {
        return false;
    }

    bool shouldOpenContextMenu = false;
    if (event->button() == Qt::MiddleButton && m_dragging)
    {
        m_dragging = false;
        m_view->setCursor(Qt::ArrowCursor);
        event->accept();
    }
    else if (event->button() == Qt::RightButton)
    {
        shouldOpenContextMenu = !m_rightDragged && m_isSelectTool && m_isSelectTool();
        m_rightDragged        = false;
    }
    return shouldOpenContextMenu;
}

bool ViewInteractionController::keyPressEvent(QKeyEvent* event)
{
    if (!event)
    {
        return false;
    }

    if (event->key() == Qt::Key_G && event->modifiers().testFlag(Qt::ControlModifier))
    {
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            if (m_ungroupSelectedShapes)
            {
                m_ungroupSelectedShapes();
            }
        }
        else
        {
            if (m_groupSelectedShapes)
            {
                m_groupSelectedShapes();
            }
        }
        event->accept();
        return true;
    }

    if (event->matches(QKeySequence::Cut))
    {
        if (m_cutSelectedShapes)
        {
            m_cutSelectedShapes();
        }
        event->accept();
        return true;
    }

    if (event->matches(QKeySequence::Copy))
    {
        if (m_copySelectedShapes)
        {
            m_copySelectedShapes();
        }
        event->accept();
        return true;
    }

    if (event->matches(QKeySequence::Paste))
    {
        if (m_pasteCopiedShapes)
        {
            m_pasteCopiedShapes();
        }
        event->accept();
        return true;
    }

    if (event->matches(QKeySequence::SelectAll))
    {
        if (m_selectAllAction)
        {
            m_selectAllAction();
        }
        event->accept();
        return true;
    }

    return false;
}

bool ViewInteractionController::wheelEvent(QWheelEvent* event)
{
    if (!m_view || !event)
    {
        return false;
    }

    const Qt::KeyboardModifiers modifiers  = event->modifiers();
    const QPoint                pixelDelta = event->pixelDelta();
    const QPoint                angleDelta = event->angleDelta();

    if (modifiers & Qt::ControlModifier)
    {
        const QPointF cursorViewPos = m_view->mapFromGlobal(QCursor::pos());
        const int     dy            = angleDelta.y() != 0 ? angleDelta.y() : pixelDelta.y();
        if (dy > 0)
        {
            if (m_zoomInAction)
            {
                m_zoomInAction(cursorViewPos);
            }
        }
        else if (dy < 0)
        {
            if (m_zoomOutAction)
            {
                m_zoomOutAction(cursorViewPos);
            }
        }
        event->accept();
        return true;
    }

    auto scrollStep = [&](const bool horizontal)
    {
        QScrollBar* bar = horizontal ? m_view->horizontalScrollBar() : m_view->verticalScrollBar();
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
    return true;
}

void ViewInteractionController::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event || !event->mimeData()->hasUrls())
    {
        if (event)
        {
            event->ignore();
        }
        return;
    }

    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls)
    {
        if (url.isLocalFile())
        {
            const QString filePath = url.toLocalFile();
            if ((m_isProjectFilePath && m_isProjectFilePath(filePath)) || ImportManager::instance().canImport(filePath))
            {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void ViewInteractionController::dragMoveEvent(QDragMoveEvent* event)
{
    if (!event || !event->mimeData()->hasUrls())
    {
        if (event)
        {
            event->ignore();
        }
        return;
    }

    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls)
    {
        if (url.isLocalFile())
        {
            const QString filePath = url.toLocalFile();
            if ((m_isProjectFilePath && m_isProjectFilePath(filePath)) || ImportManager::instance().canImport(filePath))
            {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void ViewInteractionController::dropEvent(QDropEvent* event)
{
    if (!m_view || !event || !event->mimeData()->hasUrls())
    {
        if (event)
        {
            event->ignore();
        }
        return;
    }

    const QList<QUrl> urls = event->mimeData()->urls();
    QStringList       projectPaths;
    QStringList       filePaths;
    for (const QUrl& url : urls)
    {
        if (url.isLocalFile())
        {
            const QString path = url.toLocalFile();
            if (m_isProjectFilePath && m_isProjectFilePath(path))
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
        if (m_maybeSaveBeforeProceed && !m_maybeSaveBeforeProceed())
        {
            return;
        }
        if (m_openDocumentFile)
        {
            m_openDocumentFile(projectPaths.first());
        }
        return;
    }

    if (m_importFilesAt)
    {
        const QPointF mouseScenePos = m_view->mapToScene(event->position().toPoint());
        m_importFilesAt(filePaths, mouseScenePos);
    }
}
