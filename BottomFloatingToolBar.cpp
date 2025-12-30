#include "BottomFloatingToolBar.h"
#include "EventBus.h"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPushButton>
#include <QFrame>
#include <QMenu>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QCursor>
#include <QTimer>

constexpr int DELAY_TIME = 20;

BottomFloatingToolBar::BottomFloatingToolBar(QWidget *parent) : QWidget(parent), m_pCloseTimer(nullptr), m_pZoomMenu(nullptr) {
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("BottomFloatingToolBar");
    init();
    initMenu();
    styleSheet();
}

BottomFloatingToolBar::~BottomFloatingToolBar() = default;

void BottomFloatingToolBar::setCanUndo(const bool canUndo) const {
    if (m_pUndo)
    {
		m_pUndo->setEnabled(canUndo);
    }
}

void BottomFloatingToolBar::setCanRedo(bool canRedo) const {
    if (m_pRedo)
    {
        m_pRedo->setEnabled(canRedo);
	}
}

void BottomFloatingToolBar::init() {
    QHBoxLayout* pHLayout = new QHBoxLayout(this);
    pHLayout->setContentsMargins(6, 4, 6, 4);
    pHLayout->setSpacing(4);

    auto MakeButton = [&](const QString& iconPath)
    {
        QToolButton* pToolButton = new QToolButton(this);
        pToolButton->setFixedHeight(32);
        pToolButton->setMinimumWidth(32);
        pToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        pToolButton->setIcon(QIcon(iconPath));
        pHLayout->addWidget(pToolButton);
        return pToolButton;
    };

    m_pUndo = MakeButton(":/Resource/Icons/Undo.svg");
    m_pRedo = MakeButton(":/Resource/Icons/Redo.svg");

	m_pUndo->setEnabled(false);
	m_pRedo->setEnabled(false);

    QFrame* pSeparator = new QFrame(this);
    pSeparator->setFrameShape(QFrame::VLine);
    pSeparator->setFixedWidth(1);
    pSeparator->setFixedHeight(20);
    pSeparator->setStyleSheet("background: #E7E9ED; border: none;");
    pHLayout->addWidget(pSeparator);

    m_pZoomOut  = MakeButton(":/Resource/Icons/ZoomOut.svg");
    m_pZoomTool = MakeButton("");
    m_pZoomIn   = MakeButton(":/Resource/Icons/ZoomIn.svg");

    QAction* zoomInAction = new QAction(this);
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));  // Ctrl + =
    addAction(zoomInAction);
    connect(zoomInAction, &QAction::triggered, [this](){ emit zoomIn(); });

    QAction* zoomOutAction = new QAction(this);
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus)); // Ctrl + -
    addAction(zoomOutAction);
    connect(zoomOutAction, &QAction::triggered, [this](){ emit zoomOut(); });

    QAction* undoAction = new QAction(this);
    undoAction->setShortcut(QKeySequence(QKeySequence::Undo));  // Ctrl + Z
    addAction(undoAction);
    connect(undoAction, &QAction::triggered, [this](){ emit undo(); });

    QAction* redoAction = new QAction(this);
    redoAction->setShortcut(QKeySequence(QKeySequence::Redo));  // Ctrl + Shift + Z / Ctrl + Y
    addAction(redoAction);
    connect(redoAction, &QAction::triggered, [this](){ emit redo(); });

    const QFontMetrics metrics(m_pZoomTool->font());
    const int textWidth = metrics.horizontalAdvance("10000%");
    m_pZoomTool->setMinimumWidth(textWidth);
    m_pZoomTool->setText("100%");

    connect(m_pZoomIn, &QToolButton::clicked, [this](){emit zoomIn();});
    connect(m_pZoomOut, &QToolButton::clicked, [this](){emit zoomOut();});
    connect(m_pZoomTool, &QToolButton::clicked, [this](){emit fitCanvas();});
    connect(m_pUndo, &QToolButton::clicked, [this](){emit undo();});
    connect(m_pRedo, &QToolButton::clicked, [this](){emit redo();});
    connect(&EventBus::instance(), &EventBus::zoomChanged, this, &BottomFloatingToolBar::onZoomChanged);
}

void BottomFloatingToolBar::initMenu() {
    m_pZoomMenu = new QMenu(this);

    // 添加缩放选项
    qreal zoomList[] = {80, 40, 10, 8, 6, 4, 2, 1, 0.75, 0.5, 0.25};

    for (qreal& value : zoomList)
    {
        QString zoom = QString("%1%").arg(value * 100);
        const QAction* action = m_pZoomMenu->addAction(zoom);
        connect(action, &QAction::triggered, this, [this, value]() {emit zoomTo(value); });
    }

    const QAction* action = m_pZoomMenu->addAction("适应宽度");
    connect(action, &QAction::triggered, [this](){emit fitWidth();});
    action = m_pZoomMenu->addAction("适应高度");
    connect(action, &QAction::triggered, [this](){emit fitHeight();});
    action = m_pZoomMenu->addAction("适应画布");
    connect(action, &QAction::triggered, [this](){emit fitCanvas();});
    action = m_pZoomMenu->addAction("适应图形");
    connect(action, &QAction::triggered, [this](){emit fitShapes();});

    m_pZoomTool->installEventFilter(this);
    m_pZoomMenu->installEventFilter(this);

    m_pCloseTimer = new QTimer(this);
    m_pCloseTimer->setSingleShot(true);
    connect(m_pCloseTimer, &QTimer::timeout, this, &BottomFloatingToolBar::onCloseTimerTimeout);
}

bool BottomFloatingToolBar::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_pZoomTool)
    {
        if (event->type() == QEvent::Enter)
        {
            if (m_pZoomMenu)
            {
                if (m_pZoomIn->isActiveWindow()) {
                    m_pCloseTimer->stop();
                    const QPoint pos = m_pZoomTool->mapToGlobal(QPoint(-30, -m_pZoomMenu->sizeHint().height() - 10));
                    m_pZoomMenu->popup(pos);
                }
            }
        }
        else if (event->type() == QEvent::Leave) {
            m_pCloseTimer->start(DELAY_TIME);
        }
    }
    else if (obj == m_pZoomMenu)
    {
        if (event->type() == QEvent::Enter) {
            m_pCloseTimer->stop();
        }

        if (event->type() == QEvent::MouseMove) {
            if (!m_pZoomTool->rect().contains(m_pZoomTool->mapFromGlobal(QCursor::pos()))) {
                QEvent leaveEvent = QEvent(QEvent::Leave);
                qApp->sendEvent(m_pZoomTool, &leaveEvent);
            }
        }

        if (event->type() == QEvent::MouseButtonPress) {
            if (m_pZoomTool->rect().contains(m_pZoomTool->mapFromGlobal(QCursor::pos()))) {
                fitCanvas();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
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

void BottomFloatingToolBar::onCloseTimerTimeout()
{
    if (m_pZoomMenu && m_pZoomTool && m_pZoomMenu->isVisible())
    {
        if (!m_pZoomMenu->rect().contains(m_pZoomMenu->mapFromGlobal(QCursor::pos()))
            && !m_pZoomTool->rect().contains(m_pZoomTool->mapFromGlobal(QCursor::pos()))) 
        {
            m_pZoomMenu->hide();
        }
    }
}

void BottomFloatingToolBar::styleSheet()
{
    setStyleSheet(R"(
        #BottomFloatingToolBar {
            background-color: #FFF;
            border-radius: 5px;
            border: 1px solid #E7E9ED;
        }

        QToolButton {
            border: none;
            border-radius: 5px;
        }

        QToolButton::menu-indicator {
            width: 0px;
        }

        QToolButton:hover {
            background: #F6F6F9;
        }
    )");

    if (m_pZoomMenu)
    {
        m_pZoomMenu->setStyleSheet(R"(
            QMenu {
                background: #FFFFFF;
                border-radius: 5px;
                padding: 4px;
            }

            QMenu::item {
                font-size: 12px;
                padding: 8px 20px;
                color: #000;
                border-radius: 5px;
            }

            QMenu::item:selected {
                background: #F3F3F5;
            }
        )");
    }
}
