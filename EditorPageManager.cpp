#include "EditorPageManager.h"

#include "CanvasWidget.h"
#include "Common/AppSettings.h"
#include "Common/EditorSession.h"
#include "DrawingToolsBar.h"
#include "Layer/LayerPanel.h"
#include "Serialization/DocumentTypes.h"

#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

EditorPageManager::EditorPageManager(
    QWidget* const hostWindow,
    QStackedWidget* const pageStack,
    qfw::TabBar* const tabBar,
    QObject* const parent)
    : QObject(parent),
      m_hostWindow(hostWindow),
      m_pageStack(pageStack),
      m_tabBar(tabBar),
      m_nextPageNumber(1)
{
}

void EditorPageManager::initDefaultPage()
{
    addEditorPage(tr("Editor 1"));
    refreshTabToolTips();
}

void EditorPageManager::openProjectInNewPage()
{
    const QString filter = QObject::tr("XCanvas File (*%1)")
                               .arg(QString::fromLatin1(xcanvas::serialization::kDocumentExtension));
    const QString path = QFileDialog::getOpenFileName(
        m_hostWindow,
        QObject::tr("打开文件"),
        AppSettings::instance().lastOpenedPathOrDocumentsPath(),
        filter);
    if (path.isEmpty())
    {
        return;
    }

    AppSettings::instance().setLastOpenedPath(path);
    addEditorPage();
    if (!m_tabBar)
    {
        return;
    }

    const int newIndex = m_tabBar->currentIndex();
    if (!isValidPageIndex(newIndex))
    {
        return;
    }

    CanvasWidget* const canvasWidget = m_editorPages[newIndex].canvasWidget;
    if (canvasWidget)
    {
        canvasWidget->openDocumentFile(path);
    }
}

bool EditorPageManager::maybeSaveAllBeforeClose()
{
    for (int index = 0; index < m_editorPages.size(); ++index)
    {
        if (m_tabBar && m_pageStack)
        {
            m_tabBar->setCurrentIndex(index);
            m_pageStack->setCurrentIndex(index);
        }

        const EditorPageContext& context = m_editorPages[index];
        if (context.canvasWidget && !context.canvasWidget->maybeSaveBeforeClose())
        {
            return false;
        }
    }
    return true;
}

void EditorPageManager::onTabChanged(const int index)
{
    if (!m_pageStack || !isValidPageIndex(index))
    {
        return;
    }
    m_pageStack->setCurrentIndex(index);
    refreshTabToolTips();
}

void EditorPageManager::onTabCloseRequested(const int index)
{
    if (!m_tabBar || !m_pageStack || m_pageStack->count() <= 1 || !isValidPageIndex(index))
    {
        return;
    }

    CanvasWidget* const canvas = m_editorPages[index].canvasWidget;
    if (canvas && !canvas->maybeSaveBeforeClose())
    {
        return;
    }

    removeEditorPageAt(index);
    m_tabBar->removeTab(index);
    refreshTabToolTips();
}

void EditorPageManager::onTabAddRequested()
{
    addEditorPage();
}

void EditorPageManager::onTabMoved(const int from, const int to)
{
    if (!m_pageStack || from == to || !isValidPageIndex(from) || !isValidPageIndex(to))
    {
        return;
    }

    QWidget* const movedPage = m_pageStack->widget(from);
    m_pageStack->removeWidget(movedPage);
    m_pageStack->insertWidget(to, movedPage);

    m_editorPages.move(from, to);
    refreshTabToolTips();
    if (m_tabBar)
    {
        m_pageStack->setCurrentIndex(m_tabBar->currentIndex());
    }
}

bool EditorPageManager::eventFilter(QObject* watched, QEvent* event)
{
    if (event && event->type() == QEvent::Enter && qobject_cast<qfw::TabItem*>(watched))
    {
        refreshTabToolTips();
    }
    return QObject::eventFilter(watched, event);
}

EditorPageManager::EditorPageContext EditorPageManager::createEditorPage()
{
    EditorPageContext context;
    auto* page = new QWidget(m_hostWindow);
    context.page = page;
    context.session = new EditorSession(page);

    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    auto* contentSplitter = new QSplitter(Qt::Horizontal, page);
    contentSplitter->setChildrenCollapsible(false);
    contentSplitter->setHandleWidth(1);
    pageLayout->addWidget(contentSplitter);

    auto* toolbarPanel = new QWidget(contentSplitter);
    auto* toolbarLayout = new QVBoxLayout(toolbarPanel);
    toolbarLayout->setContentsMargins(4, 0, 2, 2);
    toolbarLayout->setSpacing(0);

    auto* canvasPanel = new QWidget(contentSplitter);
    auto* canvasLayout = new QVBoxLayout(canvasPanel);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(0);

    auto* rightPanel = new QWidget(contentSplitter);
    auto* rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    rightPanelLayout->setSpacing(0);

    context.drawingToolsBar = new DrawingToolsBar(context.session, toolbarPanel);
    toolbarLayout->addWidget(context.drawingToolsBar);

    const int toolbarContentWidth = qMax(context.drawingToolsBar->sizeHint().width(), 36);
    const int toolbarPanelWidth = toolbarContentWidth
                                  + toolbarLayout->contentsMargins().left()
                                  + toolbarLayout->contentsMargins().right();
    toolbarPanel->setMinimumWidth(toolbarPanelWidth);

    context.canvasWidget = new CanvasWidget(context.session, canvasPanel);
    canvasLayout->addWidget(context.canvasWidget);

    connect(context.session, &EditorSession::openFileRequested, this, &EditorPageManager::openProjectInNewPage);
    connect(context.canvasWidget, &CanvasWidget::documentDisplayNameChanged, this,
            [this, canvas = context.canvasWidget](const QString&)
            {
                updateTabTitleForCanvas(canvas);
                refreshTabToolTips();
            });

    auto* layerPanel = new LayerPanel(context.canvasWidget->layerManager(), rightPanel);
    rightPanelLayout->addWidget(layerPanel);

    contentSplitter->setStretchFactor(0, 0);
    contentSplitter->setStretchFactor(1, 1);
    contentSplitter->setStretchFactor(2, 0);
    contentSplitter->setSizes({toolbarPanelWidth, 1000, 280});

    return context;
}

void EditorPageManager::addEditorPage(const QString& title)
{
    if (!m_pageStack || !m_tabBar)
    {
        return;
    }

    const QString pageTitle =
        title.isEmpty() ? tr("Editor %1").arg(m_nextPageNumber) : title;
    const QString routeKey = QStringLiteral("editor_%1").arg(m_nextPageNumber++);
    EditorPageContext context = createEditorPage();
    if (!context.page)
    {
        return;
    }

    m_pageStack->addWidget(context.page);
    m_editorPages.push_back(context);
    m_tabBar->addTab(routeKey, pageTitle, qfw::FluentIconEnum::Document);
    refreshTabToolTips();

    const int index = m_pageStack->count() - 1;
    m_tabBar->setCurrentIndex(index);
    m_pageStack->setCurrentIndex(index);
}

void EditorPageManager::removeEditorPageAt(const int index)
{
    if (!m_pageStack || !isValidPageIndex(index))
    {
        return;
    }

    QWidget* const page = m_pageStack->widget(index);
    m_pageStack->removeWidget(page);
    page->deleteLater();
    m_editorPages.removeAt(index);
}

void EditorPageManager::refreshTabToolTips()
{
    if (!m_tabBar)
    {
        return;
    }

    for (const QPointer<qfw::TabItem>& tabItem : m_trackedTabItems)
    {
        if (tabItem)
        {
            tabItem->removeEventFilter(this);
        }
    }
    m_trackedTabItems.clear();

    const QList<qfw::TabItem*> tabItems = m_tabBar->items();
    for (int index = 0; index < tabItems.size(); ++index)
    {
        qfw::TabItem* const tabItem = tabItems.at(index);
        if (!tabItem)
        {
            continue;
        }

        tabItem->installEventFilter(this);
        m_trackedTabItems.push_back(tabItem);

        if (!tabItem->property("xcanvas_path_tooltip_filter_installed").toBool())
        {
            tabItem->installEventFilter(new qfw::ToolTipFilter(tabItem, 500, qfw::ToolTipPosition::Bottom));
            tabItem->setProperty("xcanvas_path_tooltip_filter_installed", true);
        }

        QString pathText;
        if (isValidPageIndex(index))
        {
            CanvasWidget* const canvasWidget = m_editorPages[index].canvasWidget;
            if (canvasWidget)
            {
                const QString path = canvasWidget->currentDocumentPath();
                if (!path.isEmpty())
                {
                    pathText = QDir::toNativeSeparators(path);
                }
            }
        }
        m_tabBar->setTabToolTip(index, pathText);
    }
}

void EditorPageManager::updateTabTitleForPage(const int index)
{
    if (!m_tabBar || !isValidPageIndex(index))
    {
        return;
    }

    const CanvasWidget* const canvasWidget = m_editorPages[index].canvasWidget;
    const QString path = canvasWidget ? canvasWidget->currentDocumentPath() : QString();
    if (path.isEmpty())
    {
        return;
    }

    const QFileInfo fileInfo(path);
    const QString   baseName = fileInfo.completeBaseName();
    const QString   title = baseName.isEmpty() ? fileInfo.fileName() : baseName;
    m_tabBar->setTabText(index, title);
}

void EditorPageManager::updateTabTitleForCanvas(CanvasWidget* const canvasWidget)
{
    if (!canvasWidget)
    {
        return;
    }

    for (int index = 0; index < m_editorPages.size(); ++index)
    {
        if (m_editorPages[index].canvasWidget == canvasWidget)
        {
            updateTabTitleForPage(index);
            return;
        }
    }
}

bool EditorPageManager::isValidPageIndex(const int index) const
{
    return index >= 0 && index < m_editorPages.size();
}
