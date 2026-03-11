#include "ClipboardCommandService.h"

#include "AppSettings.h"
#include "../Canvas/Canvas.h"
#include "../Import/ImportManager.h"
#include "../Shape/GroupCommand.h"
#include "../Shape/Shape.h"
#include "../Shape/ShapeImage.h"
#include "../Shape/ShapeText.h"
#include <qtfluentwidgets.h>

#include <QAction>
#include <QClipboard>
#include <QFileInfo>
#include <QFont>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMimeData>
#include <QUrl>
#include <QUuid>
#include <QVariant>
#include <map>

ClipboardCommandService::ClipboardCommandService(
    QGraphicsView* const view,
    xcanvas::Canvas* const canvas,
    RequestFullUpdate      requestFullUpdate,
    ImportFilesAt          importFilesAt)
    : m_view(view),
      m_canvas(canvas),
      m_requestFullUpdate(std::move(requestFullUpdate)),
      m_importFilesAt(std::move(importFilesAt)),
      m_pasteSerial(0)
{
}

ClipboardCommandService::~ClipboardCommandService()
{
    clearCopiedShapes();
}

void ClipboardCommandService::clearCopiedShapes()
{
    qDeleteAll(m_copiedShapes);
    m_copiedShapes.clear();
}

void ClipboardCommandService::resetState()
{
    clearCopiedShapes();
    m_pasteSerial = 0;
}

bool ClipboardCommandService::copySelectedShapes()
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

bool ClipboardCommandService::cutSelectedShapes()
{
    if (!copySelectedShapes())
    {
        return false;
    }

    return deleteSelectedShapes();
}

bool ClipboardCommandService::pasteCopiedShapes()
{
    if (!m_canvas || !m_canvas->shapeManager() || !m_view)
    {
        return false;
    }

    const QPointF centerPos = m_view->mapToScene(m_view->viewport()->rect().center());
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
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    return true;
}

bool ClipboardCommandService::pasteCopiedShapesAt(const QPointF& scenePos)
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
    bool   hasRect = false;
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
                hasRect   = true;
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
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    ++m_pasteSerial;
    return true;
}

bool ClipboardCommandService::deleteSelectedShapes()
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
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    return true;
}

bool ClipboardCommandService::groupSelectedShapes()
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

    bool    allSameNonEmptyGroup = true;
    QString firstGroupId;
    bool    firstGroupSet = false;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape || shape->groupId().isEmpty())
        {
            allSameNonEmptyGroup = false;
            break;
        }
        if (!firstGroupSet)
        {
            firstGroupId  = shape->groupId();
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

    m_canvas->undoStack()->push(new xcanvas::GroupCommand(m_canvas->shapeManager(), std::move(beforeGroupId), QObject::tr("Group Shapes")));
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    return true;
}

bool ClipboardCommandService::ungroupSelectedShapes()
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

    m_canvas->undoStack()->push(new xcanvas::GroupCommand(m_canvas->shapeManager(), std::move(beforeGroupId), QObject::tr("Ungroup Shapes")));
    if (m_requestFullUpdate)
    {
        m_requestFullUpdate();
    }
    return true;
}

bool ClipboardCommandService::pasteFromClipboard(const QPointF& scenePos)
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
    const QPointF targetPos   = scenePos + pasteOffset;

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

            QString    localFile = line;
            const QUrl userUrl   = QUrl::fromUserInput(line);
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
        if (m_importFilesAt)
        {
            m_importFilesAt(importPaths, targetPos);
            ++m_pasteSerial;
            return true;
        }
        return false;
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
                if (m_requestFullUpdate)
                {
                    m_requestFullUpdate();
                }
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
            if (m_requestFullUpdate)
            {
                m_requestFullUpdate();
            }
            ++m_pasteSerial;
            return true;
        }
    }

    return false;
}

bool ClipboardCommandService::hasClipboardPasteContent() const
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

void ClipboardCommandService::showCanvasContextMenu(const QPoint& viewPos, const ZoomAction& onZoomIn, const ZoomAction& onZoomOut)
{
    if (!m_view)
    {
        return;
    }

    const bool canCopy  = m_canvas && m_canvas->shapeManager() && m_canvas->shapeManager()->hasSelection();
    const bool canCut   = canCopy;
    const bool canPaste = hasClipboardPasteContent() || !m_copiedShapes.isEmpty();
    const bool canDelete = canCopy;
    const xcanvas::ShapeList selectedShapes = (m_canvas && m_canvas->shapeManager()) ? m_canvas->shapeManager()->selectedShapeList() : xcanvas::ShapeList();
    bool    allSameNonEmptyGroup = selectedShapes.size() >= 2;
    QString firstGroupId;
    bool    firstGroupSet = false;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (!shape || shape->groupId().isEmpty())
        {
            allSameNonEmptyGroup = false;
            break;
        }
        if (!firstGroupSet)
        {
            firstGroupId  = shape->groupId();
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
    bool       canUngroup = false;
    for (xcanvas::Shape* shape : selectedShapes)
    {
        if (shape && !shape->groupId().isEmpty())
        {
            canUngroup = true;
            break;
        }
    }

    auto* menu = new qfw::RoundMenu(QString(), m_view);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (canCut)
    {
        auto* cutAction = new QAction(QObject::tr("剪切"), menu);
        menu->addAction(cutAction);
        cutAction->setShortcut(QKeySequence::Cut);
        cutAction->setShortcutVisibleInContextMenu(true);
        QObject::connect(cutAction, &QAction::triggered, menu, [this]() { cutSelectedShapes(); });
    }

    if (canCopy)
    {
        auto* copyAction = new QAction(QObject::tr("复制"), menu);
        menu->addAction(copyAction);
        copyAction->setShortcut(QKeySequence::Copy);
        copyAction->setShortcutVisibleInContextMenu(true);
        QObject::connect(copyAction, &QAction::triggered, menu, [this]() { copySelectedShapes(); });
    }

    if (canPaste)
    {
        auto* pasteAction = new QAction(QObject::tr("粘贴"), menu);
        menu->addAction(pasteAction);
        pasteAction->setShortcut(QKeySequence::Paste);
        pasteAction->setShortcutVisibleInContextMenu(true);
        QObject::connect(pasteAction, &QAction::triggered, menu, [this, viewPos]() { pasteCopiedShapesAt(m_view->mapToScene(viewPos)); });
    }

    if (canDelete)
    {
        auto* deleteAction = new QAction(QObject::tr("删除"), menu);
        menu->addAction(deleteAction);
        deleteAction->setShortcut(QKeySequence::Delete);
        deleteAction->setShortcutVisibleInContextMenu(true);
        QObject::connect(deleteAction, &QAction::triggered, menu, [this]() { deleteSelectedShapes(); });
    }

    if (canGroup)
    {
        auto* groupAction = new QAction(QObject::tr("成组"), menu);
        menu->addAction(groupAction);
        groupAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
        groupAction->setShortcutVisibleInContextMenu(true);
        QObject::connect(groupAction, &QAction::triggered, menu, [this]() { groupSelectedShapes(); });
    }

    if (canUngroup)
    {
        auto* ungroupAction = new QAction(QObject::tr("取消成组"), menu);
        menu->addAction(ungroupAction);
        ungroupAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
        ungroupAction->setShortcutVisibleInContextMenu(true);
        QObject::connect(ungroupAction, &QAction::triggered, menu, [this]() { ungroupSelectedShapes(); });
    }

    menu->addSeparator();

    auto* selectAllAction = new QAction(QObject::tr("全选"), menu);
    menu->addAction(selectAllAction);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutVisibleInContextMenu(true);
    QObject::connect(selectAllAction, &QAction::triggered, menu, [this]()
    {
        if (m_canvas && m_canvas->shapeManager())
        {
            m_canvas->shapeManager()->selectAll();
            if (m_requestFullUpdate)
            {
                m_requestFullUpdate();
            }
        }
    });

    auto* zoomInAction = new QAction(QObject::tr("放大"), menu);
    menu->addAction(zoomInAction);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    zoomInAction->setShortcutVisibleInContextMenu(true);
    QObject::connect(zoomInAction, &QAction::triggered, menu, [onZoomIn]()
    {
        if (onZoomIn)
        {
            onZoomIn();
        }
    });

    auto* zoomOutAction = new QAction(QObject::tr("缩小"), menu);
    menu->addAction(zoomOutAction);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    zoomOutAction->setShortcutVisibleInContextMenu(true);
    QObject::connect(zoomOutAction, &QAction::triggered, menu, [onZoomOut]()
    {
        if (onZoomOut)
        {
            onZoomOut();
        }
    });

    menu->execAt(m_view->mapToGlobal(viewPos), true, qfw::MenuAnimationType::None);
}
