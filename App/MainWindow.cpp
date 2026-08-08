#include "MainWindow.h"

#include "EditorPageManager.h"
#include "Common/XTitleBar.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFontDatabase>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : qfw::FluentWindow(parent), m_pPageStack(nullptr), m_pTabBar(nullptr), m_editorPageManager(nullptr)
{
    init();
}

MainWindow::~MainWindow()
{
}

void MainWindow::showEvent(QShowEvent* event)
{
    qfw::FluentWindow::showEvent(event);
    fixTitleBarGeometry();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    qfw::FluentWindow::resizeEvent(event);
    fixTitleBarGeometry();
}

void MainWindow::init()
{
    setWindowTitle(QStringLiteral("XCanvas"));

    if (const int fontId = QFontDatabase::addApplicationFont(":/Resource/Font/MiSans-Medium.ttf"); fontId != -1)
    {
        QFont font("MiSans");
        font.setPixelSize(13);
        QApplication::setFont(font);
    }

    initMultiPageLayout();
}

void MainWindow::initMultiPageLayout()
{
    auto* xTitleBar = new XTitleBar(this);
    setTitleBar(xTitleBar);

    auto* contentHost = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentHost);
    contentLayout->setContentsMargins(0, xTitleBar->height(), 0, 0);
    contentLayout->setSpacing(0);

    m_pPageStack = new QStackedWidget(contentHost);
    contentLayout->addWidget(m_pPageStack);
    setContentWidget(contentHost);

    m_pTabBar = xTitleBar->tabBar();

    m_editorPageManager = new EditorPageManager(this, m_pPageStack, m_pTabBar, this);
    connect(m_pTabBar, &qfw::TabBar::currentChanged, m_editorPageManager, &EditorPageManager::onTabChanged);
    connect(m_pTabBar, &qfw::TabBar::tabCloseRequested, m_editorPageManager, &EditorPageManager::onTabCloseRequested);
    connect(m_pTabBar, &qfw::TabBar::tabAddRequested, m_editorPageManager, &EditorPageManager::onTabAddRequested);
    connect(m_pTabBar, &qfw::TabBar::tabMoved, m_editorPageManager, &EditorPageManager::onTabMoved);
    m_editorPageManager->initDefaultPage();
}

void MainWindow::fixTitleBarGeometry()
{
    if (!titleBar())
    {
        return;
    }

    titleBar()->move(0, 0);
    titleBar()->resize(width(), titleBar()->height());
    titleBar()->updateGeometry();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_editorPageManager && !m_editorPageManager->maybeSaveAllBeforeClose())
    {
        event->ignore();
        return;
    }

    qfw::FluentWindow::closeEvent(event);
}
