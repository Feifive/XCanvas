#include "DocumentSessionController.h"

#include "AppSettings.h"
#include "../Canvas/Canvas.h"
#include "../Serialization/DocumentTypes.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDialog>
#include <QPushButton>
#include <QUndoStack>
#include <QWidget>
#include <qtfluentwidgets.h>

DocumentSessionController::DocumentSessionController(
    QWidget* const             hostView,
    xcanvas::Canvas* const     canvas,
    IsBusy                     isBusy,
    ShowBusyWarning            showBusyWarning,
    OpenDocumentAsync          openDocumentAsync,
    SaveDocumentAsync          saveDocumentAsync,
    SaveDocumentBlocking       saveDocumentBlocking,
    ResetCanvasState           resetCanvasState)
    : m_hostView(hostView),
      m_canvas(canvas),
      m_isBusy(std::move(isBusy)),
      m_showBusyWarning(std::move(showBusyWarning)),
      m_openDocumentAsync(std::move(openDocumentAsync)),
      m_saveDocumentAsync(std::move(saveDocumentAsync)),
      m_saveDocumentBlocking(std::move(saveDocumentBlocking)),
      m_resetCanvasState(std::move(resetCanvasState))
{
}

void DocumentSessionController::onNewDocument()
{
    if (!maybeSaveBeforeProceed())
    {
        return;
    }

    resetToNewDocument();
}

void DocumentSessionController::onOpenDocument()
{
    if (m_isBusy && m_isBusy())
    {
        if (m_showBusyWarning)
        {
            m_showBusyWarning();
        }
        return;
    }

    if (!maybeSaveBeforeProceed())
    {
        return;
    }

    const QString filter = QObject::tr("XCanvas File (*%1)")
                               .arg(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
    const QString path = QFileDialog::getOpenFileName(
        m_hostView,
        QObject::tr("打开文件"),
        AppSettings::instance().lastOpenedPathOrDocumentsPath(),
        filter);
    if (path.isEmpty())
    {
        return;
    }

    AppSettings::instance().setLastOpenedPath(path);

    if (m_openDocumentAsync)
    {
        m_openDocumentAsync(path);
    }
}

void DocumentSessionController::onSaveDocument()
{
    if (m_isBusy && m_isBusy())
    {
        if (m_showBusyWarning)
        {
            m_showBusyWarning();
        }
        return;
    }

    if (m_currentDocumentPath.isEmpty())
    {
        onSaveDocumentAs();
        return;
    }

    if (m_saveDocumentAsync)
    {
        m_saveDocumentAsync(m_currentDocumentPath, false);
    }
}

void DocumentSessionController::onSaveDocumentAs()
{
    if (m_isBusy && m_isBusy())
    {
        if (m_showBusyWarning)
        {
            m_showBusyWarning();
        }
        return;
    }

    const QString filter = QObject::tr("XCanvas File (*%1)")
                               .arg(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
    QString path = QFileDialog::getSaveFileName(
        m_hostView,
        QObject::tr("另存为"),
        AppSettings::instance().lastOpenedPathOrDocumentsPath(),
        filter);
    if (path.isEmpty())
    {
        return;
    }

    if (!path.endsWith(QString::fromLatin1(xcanvas::serialization::kDocumentExtension), Qt::CaseInsensitive)
        && QFileInfo(path).suffix().isEmpty())
    {
        path += QString::fromLatin1(xcanvas::serialization::kDocumentExtension);
    }

    AppSettings::instance().setLastOpenedPath(path);

    if (m_saveDocumentAsync)
    {
        m_saveDocumentAsync(path, true);
    }
}

bool DocumentSessionController::maybeSaveBeforeProceed()
{
    if (!m_canvas || !m_canvas->undoStack() || m_canvas->undoStack()->isClean())
    {
        return true;
    }

    qfw::MessageBox messageBox(
        QObject::tr("内容未保存"),
        QObject::tr("当前工程已修改，是否先保存？"),
        m_hostView ? m_hostView->window() : nullptr);
    bool isDontSaveClicked = false;
    if (messageBox.ui() && messageBox.ui()->yesButton && messageBox.ui()->cancelButton)
    {
        messageBox.ui()->yesButton->setText(QObject::tr("保存"));
        messageBox.ui()->cancelButton->setText(QObject::tr("取消"));

        auto* dontSaveButton = new QPushButton(QObject::tr("不保存"), messageBox.ui()->buttonGroup);
        dontSaveButton->setObjectName(QStringLiteral("cancelButton"));
        dontSaveButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
        messageBox.ui()->buttonLayout->insertWidget(1, dontSaveButton, 1, Qt::AlignVCenter);
        QObject::connect(dontSaveButton, &QPushButton::clicked, &messageBox,
                         [&messageBox, &isDontSaveClicked]()
                         {
                             isDontSaveClicked = true;
                             messageBox.reject();
                         });
    }

    const int execResult = messageBox.exec();

    if (execResult == QDialog::Accepted)
    {
        if (m_currentDocumentPath.isEmpty())
        {
            const QString filter = QObject::tr("XCanvas File (*%1)")
                                       .arg(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
            QString path = QFileDialog::getSaveFileName(
                m_hostView,
                QObject::tr("保存工程"),
                AppSettings::instance().lastOpenedPathOrDocumentsPath(),
                filter);
            if (path.isEmpty())
            {
                return false;
            }

            if (!path.endsWith(QString::fromLatin1(xcanvas::serialization::kDocumentExtension), Qt::CaseInsensitive)
                && QFileInfo(path).suffix().isEmpty())
            {
                path += QString::fromLatin1(xcanvas::serialization::kDocumentExtension);
            }
            AppSettings::instance().setLastOpenedPath(path);
            return m_saveDocumentBlocking ? m_saveDocumentBlocking(path) : false;
        }

        return m_saveDocumentBlocking ? m_saveDocumentBlocking(m_currentDocumentPath) : false;
    }

    if (isDontSaveClicked)
    {
        return true;
    }

    return false;
}

bool DocumentSessionController::maybeSaveBeforeClose()
{
    return maybeSaveBeforeProceed();
}

bool DocumentSessionController::isProjectFilePath(const QString& path) const
{
    const QString lowerPath = path.toLower();
    return lowerPath.endsWith(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
}

bool DocumentSessionController::openDocumentFile(const QString& path)
{
    if (m_openDocumentAsync)
    {
        m_openDocumentAsync(path);
        return true;
    }
    return false;
}

bool DocumentSessionController::saveDocumentFile(const QString& path)
{
    if (m_saveDocumentAsync)
    {
        m_saveDocumentAsync(path, true);
        return true;
    }
    return false;
}

void DocumentSessionController::resetToNewDocument()
{
    if (m_resetCanvasState)
    {
        m_resetCanvasState();
    }
    m_currentDocumentPath.clear();
    updateWindowTitle();
}

void DocumentSessionController::updateWindowTitle() const
{
    QWidget* topLevel = m_hostView ? m_hostView->window() : nullptr;
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

QString DocumentSessionController::projectDisplayName() const
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

const QString& DocumentSessionController::currentDocumentPath() const
{
    return m_currentDocumentPath;
}

void DocumentSessionController::setCurrentDocumentPath(const QString& path)
{
    m_currentDocumentPath = path;
}

void DocumentSessionController::clearCurrentDocumentPath()
{
    m_currentDocumentPath.clear();
}
