#include "BottomFloatingToolBar.h"
#include "EditorSession.h"
#include "XCanvasIcon.h"
#include "XHoverMenu.h"
#include <QCoreApplication>
#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QMenu>
#include <qtfluentwidgets.h>

constexpr int DELAY_TIME = 20;

BottomFloatingToolBar::BottomFloatingToolBar(EditorSession* session, QWidget* parent)
    : QWidget(parent), m_pZoomMenu(nullptr), m_editorSession(session)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("BottomFloatingToolBar");
    init();
    initMenu();
    applyStyle();
    connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
            [this](qfw::Theme)
            {
                applyStyle();
                if (m_pUndo) { m_pUndo->update(); }
                if (m_pRedo) { m_pRedo->update(); }
                if (m_pZoomOut) { m_pZoomOut->update(); }
                if (m_pZoomTool) { m_pZoomTool->update(); }
                if (m_pZoomIn) { m_pZoomIn->update(); }
                if (m_pZoomMenu) { m_pZoomMenu->update(); }
            });
}

BottomFloatingToolBar::~BottomFloatingToolBar() = default;

void BottomFloatingToolBar::setCanUndo(const bool canUndo) const
{
    if (m_pUndo)
    {
        m_pUndo->setEnabled(canUndo);
    }
}

void BottomFloatingToolBar::setCanRedo(bool canRedo) const
{
    if (m_pRedo)
    {
        m_pRedo->setEnabled(canRedo);
    }
}

void BottomFloatingToolBar::init()
{
    QHBoxLayout* pHLayout = new QHBoxLayout(this);
    pHLayout->setContentsMargins(6, 4, 6, 4);
    pHLayout->setSpacing(4);

    auto makeIconButton = [&](const XCanvasIconType iconType)
    {
        auto* pToolButton = new qfw::TransparentToolButton(this);
        pToolButton->setFixedHeight(32);
        pToolButton->setMinimumWidth(32);
        pToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        pToolButton->setIcon(XCanvasIcon(iconType));
        pHLayout->addWidget(pToolButton);
        return pToolButton;
    };
    auto makeTextButton = [&]()
    {
        auto* pToolButton = new qfw::TransparentToolButton(this);
        pToolButton->setFixedHeight(32);
        pToolButton->setMinimumWidth(32);
        pToolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        pHLayout->addWidget(pToolButton);
        return pToolButton;
    };

    m_pUndo = makeIconButton(XCanvasIconType::Undo);
    m_pRedo = makeIconButton(XCanvasIconType::Redo);

    m_pUndo->setEnabled(false);
    m_pRedo->setEnabled(false);

    QFrame* pSeparator = new QFrame(this);
    pSeparator->setFrameShape(QFrame::VLine);
    pSeparator->setFixedWidth(1);
    pSeparator->setFixedHeight(20);
    pSeparator->setObjectName("BottomFloatingToolBarSeparator");
    pHLayout->addWidget(pSeparator);

    m_pZoomOut  = makeIconButton(XCanvasIconType::ZoomOut);
    m_pZoomTool = makeTextButton();
    m_pZoomIn   = makeIconButton(XCanvasIconType::ZoomIn);

    QAction* zoomInAction = new QAction(this);
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));// Ctrl + =
    addAction(zoomInAction);
    connect(zoomInAction, &QAction::triggered, [this]() { emit zoomIn(); });

    QAction* zoomOutAction = new QAction(this);
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));// Ctrl + -
    addAction(zoomOutAction);
    connect(zoomOutAction, &QAction::triggered, [this]() { emit zoomOut(); });

    QAction* undoAction = new QAction(this);
    undoAction->setShortcut(QKeySequence(QKeySequence::Undo));// Ctrl + Z
    addAction(undoAction);
    connect(undoAction, &QAction::triggered, [this]() { emit undo(); });

    QAction* redoAction = new QAction(this);
    redoAction->setShortcut(QKeySequence(QKeySequence::Redo));// Ctrl + Shift + Z / Ctrl + Y
    addAction(redoAction);
    connect(redoAction, &QAction::triggered, [this]() { emit redo(); });

    const QFontMetrics metrics(m_pZoomTool->font());
    const int          textWidth = metrics.horizontalAdvance("10000%");
    m_pZoomTool->setMinimumWidth(textWidth);
    m_pZoomTool->setText("100%");

    connect(m_pZoomIn, &qfw::TransparentToolButton::clicked, [this]() { emit zoomIn(); });
    connect(m_pZoomOut, &qfw::TransparentToolButton::clicked, [this]() { emit zoomOut(); });
    connect(m_pZoomTool, &qfw::TransparentToolButton::clicked, [this]() { emit fitCanvas(); });
    connect(m_pUndo, &qfw::TransparentToolButton::clicked, [this]() { emit undo(); });
    connect(m_pRedo, &qfw::TransparentToolButton::clicked, [this]() { emit redo(); });
    if (m_editorSession)
    {
        connect(m_editorSession, &EditorSession::zoomChanged, this, &BottomFloatingToolBar::onZoomChanged);
    }
}

void BottomFloatingToolBar::initMenu()
{
    m_pZoomMenu = new XHoverMenu(m_pZoomTool, XHoverMenu::PopupOrientation::Top, this);
    // m_pZoomMenu->setGap(10);

    // 添加缩放选项
    qreal zoomList[] = {80, 40, 10, 8, 6, 4, 2, 1, 0.75, 0.5, 0.25};

    for (qreal& value : zoomList)
    {
        QString  zoom   = QString("%1%").arg(value * 100);
        QAction* action = new QAction(zoom, m_pZoomMenu);
        m_pZoomMenu->addAction(action);
        connect(action, &QAction::triggered, this, [this, value]() { emit zoomTo(value); });
    }

    QAction* action = new QAction("适应宽度", m_pZoomMenu);
    m_pZoomMenu->addAction(action);
    connect(action, &QAction::triggered, [this]() { emit fitWidth(); });
    action = new QAction("适应高度", m_pZoomMenu);
    m_pZoomMenu->addAction(action);
    connect(action, &QAction::triggered, [this]() { emit fitHeight(); });
    action = new QAction("适应画布", m_pZoomMenu);
    m_pZoomMenu->addAction(action);
    connect(action, &QAction::triggered, [this]() { emit fitCanvas(); });
    action = new QAction("适应图形", m_pZoomMenu);
    m_pZoomMenu->addAction(action);
    connect(action, &QAction::triggered, [this]() { emit fitShapes(); });
}

void BottomFloatingToolBar::applyStyle() {
    const bool dark = qfw::isDarkTheme();
    setStyleSheet(QStringLiteral(R"(
        QWidget#BottomFloatingToolBar {
            background-color: %1;
            border-radius: 4px;
            border: 1px solid %2;
        }
        QFrame#BottomFloatingToolBarSeparator {
            background: %2;
            border: none;
        }
    )").arg(dark ? QStringLiteral("#232830")
                 : QStringLiteral("#FFFFFF"),
            dark ? QStringLiteral("#3A404B")
                 : QStringLiteral("#E7E9ED")));
}

void BottomFloatingToolBar::onZoomChanged(const qreal zoomValue)
{
    if (m_pZoomTool)
    {
        if (m_pZoomOut)
        {
            m_pZoomOut->setEnabled(zoomValue != MIN_ZOOM);
        }
        if (m_pZoomIn)
        {
            m_pZoomIn->setEnabled(zoomValue != MAX_ZOOM);
        }
        const int value = std::round(zoomValue * 100);
        m_pZoomTool->setText(QString::number(value) + "%");
    }
}
