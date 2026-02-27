#ifndef DOCUMENTSESSIONCONTROLLER_H
#define DOCUMENTSESSIONCONTROLLER_H

#include <functional>

#include <QString>

class QWidget;

namespace xcanvas
{
class Canvas;
}

class DocumentSessionController final
{
  public:
    using IsBusy           = std::function<bool()>;
    using ShowBusyWarning  = std::function<void()>;
    using OpenDocumentAsync = std::function<void(const QString&)>;
    using SaveDocumentAsync = std::function<void(const QString&, bool)>;
    using SaveDocumentBlocking = std::function<bool(const QString&)>;
    using ResetCanvasState = std::function<void()>;

    DocumentSessionController(
        QWidget*             hostView,
        xcanvas::Canvas*     canvas,
        IsBusy               isBusy,
        ShowBusyWarning      showBusyWarning,
        OpenDocumentAsync    openDocumentAsync,
        SaveDocumentAsync    saveDocumentAsync,
        SaveDocumentBlocking saveDocumentBlocking,
        ResetCanvasState     resetCanvasState);

    void onNewDocument();
    void onOpenDocument();
    void onSaveDocument();
    void onSaveDocumentAs();

    bool maybeSaveBeforeProceed();
    bool maybeSaveBeforeClose();

    bool isProjectFilePath(const QString& path) const;
    bool openDocumentFile(const QString& path);
    bool saveDocumentFile(const QString& path);
    void resetToNewDocument();

    void updateWindowTitle() const;
    QString projectDisplayName() const;

    const QString& currentDocumentPath() const;
    void           setCurrentDocumentPath(const QString& path);
    void           clearCurrentDocumentPath();

  private:
    QWidget*             m_hostView;
    xcanvas::Canvas*     m_canvas;
    IsBusy               m_isBusy;
    ShowBusyWarning      m_showBusyWarning;
    OpenDocumentAsync    m_openDocumentAsync;
    SaveDocumentAsync    m_saveDocumentAsync;
    SaveDocumentBlocking m_saveDocumentBlocking;
    ResetCanvasState     m_resetCanvasState;
    QString              m_currentDocumentPath;
};

#endif// DOCUMENTSESSIONCONTROLLER_H
