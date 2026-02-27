#ifndef DOCUMENTIOCONTROLLER_H
#define DOCUMENTIOCONTROLLER_H

#include <functional>

#include <QObject>
#include <QPointF>
#include <QString>
#include <QStringList>

class AsyncFileTaskRunner;
class ClipboardCommandService;
class DocumentSessionController;
class SelectionHudBar;
class QWidget;

namespace xcanvas
{
class Canvas;
}

class DocumentIoController final : public QObject
{
  public:
    using VoidCallback = std::function<void()>;

    DocumentIoController(
        QWidget*                   hostView,
        xcanvas::Canvas*           canvas,
        AsyncFileTaskRunner*       fileTaskRunner,
        DocumentSessionController* documentSessionController,
        ClipboardCommandService*   clipboardCommandService,
        SelectionHudBar*           selectionHudBar,
        VoidCallback               requestFullUpdate,
        VoidCallback               updateWindowTitle,
        QObject*                   parent = nullptr);

    void setClipboardCommandService(ClipboardCommandService* clipboardCommandService);
    void setSelectionHudBar(SelectionHudBar* selectionHudBar);
    void setDocumentSessionController(DocumentSessionController* documentSessionController);

    void openDocumentFileAsync(const QString& path);
    void saveDocumentFileAsync(const QString& path, bool updateCurrentPath);
    bool saveDocumentFileBlocking(const QString& path);

    void importFiles(const QStringList& filePaths, const QPointF& targetCenter);

  private:
    QWidget*                   m_hostView;
    xcanvas::Canvas*           m_canvas;
    AsyncFileTaskRunner*       m_fileTaskRunner;
    DocumentSessionController* m_documentSessionController;
    ClipboardCommandService*   m_clipboardCommandService;
    SelectionHudBar*           m_selectionHudBar;
    VoidCallback               m_requestFullUpdate;
    VoidCallback               m_updateWindowTitle;
};

#endif// DOCUMENTIOCONTROLLER_H
