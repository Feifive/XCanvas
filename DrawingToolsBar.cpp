#include "DrawingToolsBar.h"
#include "AppSettings.h"
#include "EditorSession.h"
#include "ToolGroupButton.h"
#include <QActionGroup>
#include <QButtonGroup>
#include <QFileDialog>
#include <QKeySequence>
#include <QSignalBlocker>

DrawingToolsBar::DrawingToolsBar(EditorSession* session, QWidget* parent)
    : QToolBar{parent}, m_editorSession(session)
{
    setObjectName("DrawingToolsBar");
    setOrientation(Qt::Vertical);
    setMovable(false);
    setFloatable(false);
    applyStyle();
    createToolBar();
}

DrawingToolsBar::~DrawingToolsBar() = default;

void DrawingToolsBar::createToolBar() {
    m_pGroup = new QButtonGroup(this);
    m_pGroup->setExclusive(true);

    m_pMainMenu       = makeActionButton(XCanvasIconType::MainMenu);
    m_pImport         = makeActionButton(XCanvasIconType::Import);
    m_pShapeToolGroup = createToolGroupButton();
    m_pText           = makeToggleButton(XCanvasIconType::Text);

    addWidget(m_pMainMenu);
    addWidget(m_pImport);
    addWidget(m_pShapeToolGroup);
    addWidget(m_pText);

    // 垂直弹簧
    auto* pSpring = new QWidget(this);
    pSpring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(pSpring);
    m_pSelectTool = makeToggleButton(XCanvasIconType::Select);
    m_pDrawingToolLock = new qfw::TransparentToggleToolButton(this);
    m_pDrawingToolLock->setFixedSize(QSize(36, 36));
    m_pDrawingToolLock->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_pDrawingToolLock->setIcon(XCanvasIcon(XCanvasIconType::DrawingToolLock));
    addWidget(m_pSelectTool);
    addWidget(m_pDrawingToolLock);

    setupToolTip(m_pMainMenu, tr("主菜单"));
    setupToolTip(m_pImport, tr("导入"));
    setupToolTip(m_pText, tr("文字"));
    setupToolTip(m_pSelectTool, tr("选择"));
    setupToolTip(m_pDrawingToolLock, tr("锁定绘图工具"));

    connect(m_pSelectTool, &QToolButton::clicked, this, [this]
    {
        if (m_editorSession)
        {
            m_editorSession->requestSwitchTool(DrawingToolType::Select);
        }
    });
    connect(m_pDrawingToolLock, &QToolButton::clicked, this, [this](const bool checked)
    {
        if (m_editorSession)
        {
            m_editorSession->requestSetDrawingToolLock(checked);
        }
    });
    connect(m_pImport, &QToolButton::clicked, this, [this]
    {
        if (m_editorSession)
        {
            m_editorSession->requestImportFile();
        }
    });
    connect(m_pText, &QToolButton::clicked, this, [this]
    {
        if (m_editorSession)
        {
            m_editorSession->requestSwitchTool(DrawingToolType::Text);
        }
    });
    connect(m_pShapeToolGroup, &ToolGroupButton::toolSelected, this, [this](const DrawingToolType type) {
        if (m_editorSession)
            m_editorSession->requestSwitchTool(type);
    });
    if (m_editorSession)
    {
        connect(m_editorSession, &EditorSession::currentToolChanged, this, &DrawingToolsBar::syncCurrentTool);
        connect(m_editorSession, &EditorSession::drawingToolLockChanged, m_pDrawingToolLock,
                &qfw::TransparentToggleToolButton::setChecked);
    }

    syncCurrentTool(DrawingToolType::Select);
    initMainMenu();
}

void DrawingToolsBar::syncCurrentTool(const DrawingToolType type) const {
    if (!m_pGroup)
        return;

    const QSignalBlocker groupBlocker(m_pGroup);
    const QSignalBlocker selectBlocker(m_pSelectTool);
    const QSignalBlocker textBlocker(m_pText);

    m_pSelectTool->setChecked(false);
    m_pText->setChecked(false);

    m_pShapeToolGroup->setCurrentTool(type);

    switch (type)
    {
    case DrawingToolType::Select:
        m_pSelectTool->setChecked(true);
        break;
    case DrawingToolType::Text:
        m_pText->setChecked(true);
        break;
    default:
        break;
    }
}

void DrawingToolsBar::onFileActionsEnabledChanged(const bool enabled) const
{
    if (m_openAction)
    {
        m_openAction->setEnabled(enabled);
    }
    if (m_importAction)
    {
        m_importAction->setEnabled(enabled);
    }
    if (m_saveAction)
    {
        m_saveAction->setEnabled(enabled);
    }
    if (m_saveAsAction)
    {
        m_saveAsAction->setEnabled(enabled);
    }
    if (m_pImport)
    {
        m_pImport->setEnabled(enabled);
    }
}

void DrawingToolsBar::applyStyle()
{
    setStyleSheet(QStringLiteral(R"(
        QToolBar#DrawingToolsBar {
            background: transparent;
            spacing: 8px;
            border: none;
        }
    )"));
}

void DrawingToolsBar::setupToolTip(QWidget *button, const QString &text) {
    if (!button)
    {
        return;
    }
    button->setToolTip(text);
    button->setToolTipDuration(5000);
    button->installEventFilter(new qfw::ToolTipFilter(button, 500, qfw::ToolTipPosition::Right));
}

qfw::TransparentToggleToolButton * DrawingToolsBar::
makeToggleButton(const XCanvasIconType iconType) {
    auto* pToolButton = new qfw::TransparentToggleToolButton(this);
    pToolButton->setFixedSize(QSize(36, 36));
    pToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pToolButton->setIcon(XCanvasIcon(iconType));
    if (m_pGroup) {
        m_pGroup->addButton(pToolButton);
    }
    return pToolButton;
}

qfw::TransparentToolButton * DrawingToolsBar::makeActionButton(const XCanvasIconType iconType) {
    auto* pToolButton = new qfw::TransparentToolButton(this);
    pToolButton->setFixedSize(QSize(36, 36));
    pToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pToolButton->setIcon(XCanvasIcon(iconType));
    return pToolButton;
}

void DrawingToolsBar::initMainMenu()
{
    connect(m_pMainMenu, &QToolButton::clicked, this,
            [this]()
            {
                const QPoint pos = m_pMainMenu->mapToGlobal(QPoint(m_pMainMenu->width(), 0));
                createMainMenu(pos);
            });
}

qfw::RoundMenu* DrawingToolsBar::createMainMenu(const QPoint& pos)
{
    if (!m_pMainRoundMenu)
    {
        m_pMainRoundMenu = new qfw::RoundMenu(QString(), this);
        m_pMainRoundMenu->setMinimumSize(180, 30);

        auto* group = new QActionGroup(m_pMainRoundMenu);
        group->setExclusive(true);

        auto* gridSetting       = new qfw::CheckableMenu("画布和网格", m_pMainRoundMenu, qfw::MenuIndicatorType::Radio);
        auto  addContrastAction = [&](const QString& text, AppSettings::GridContrast value)
        {
            auto* action = new qfw::Action(text, gridSetting);
            action->setCheckable(true);
            gridSetting->addAction(action);
            group->addAction(action);

            if (AppSettings::instance().gridContrast() == value)
            {
                action->setChecked(true);
            }

            connect(action, &QAction::triggered, this,
                    [value]()
                    {
                        AppSettings::instance().setGridContrast(value);
                    });

            return action;
        };

        gridSetting->setMinimumSize(150, 30);
        addContrastAction("网格-强", AppSettings::GridContrast::High);
        addContrastAction("网格-中", AppSettings::GridContrast::Medium);
        addContrastAction("网格-弱", AppSettings::GridContrast::Low);
        addContrastAction("网格-关闭", AppSettings::GridContrast::Off);

        auto* fileMenu = new qfw::RoundMenu("文件", m_pMainRoundMenu);
        auto* newAction = new qfw::Action(tr("新建"), fileMenu);
        fileMenu->addAction(newAction);
        fileMenu->addSeparator();
        m_openAction = new qfw::Action(tr("打开项目"), fileMenu);
        fileMenu->addAction(m_openAction);
        m_importAction = new qfw::Action(tr("导入"), fileMenu);
        fileMenu->addAction(m_importAction);
        fileMenu->addSeparator();
        m_saveAction = new qfw::Action(tr("保存"), fileMenu);
        fileMenu->addAction(m_saveAction);
        m_saveAsAction = new qfw::Action(tr("另存为"), fileMenu);
        fileMenu->addAction(m_saveAsAction);

        auto attachShortcut = [](QAction* action, const QKeySequence& sequence)
        {
            action->setShortcut(sequence);
            action->setShortcutVisibleInContextMenu(true);
        };
        attachShortcut(newAction, QKeySequence::New);
        attachShortcut(m_openAction, QKeySequence::Open);
        attachShortcut(m_importAction, QKeySequence(Qt::CTRL | Qt::Key_I));
        attachShortcut(m_saveAction, QKeySequence::Save);
        attachShortcut(m_saveAsAction, QKeySequence::SaveAs);

        connect(newAction, &QAction::triggered, this, [this]
        {
            if (m_editorSession)
            {
                m_editorSession->requestNewFile();
            }
        });
        connect(m_openAction, &QAction::triggered, this, [this]
        {
            if (m_editorSession)
            {
                m_editorSession->requestOpenFile();
            }
        });
        connect(m_saveAction, &QAction::triggered, this, [this]
        {
            if (m_editorSession)
            {
                m_editorSession->requestSaveFile();
            }
        });
        connect(m_saveAsAction, &QAction::triggered, this, [this]
        {
            if (m_editorSession)
            {
                m_editorSession->requestSaveFileAs();
            }
        });
        connect(m_importAction, &QAction::triggered, this, [this]
        {
            if (m_editorSession)
            {
                m_editorSession->requestImportFile();
            }
        });
        if (m_editorSession)
        {
            connect(m_editorSession, &EditorSession::fileActionsEnabledChanged, this,
                    &DrawingToolsBar::onFileActionsEnabledChanged);
        }

        m_pMainRoundMenu->addMenu(fileMenu);
        m_pMainRoundMenu->addMenu(gridSetting);
    }

    m_pMainRoundMenu->execAt(pos, true);
    return m_pMainRoundMenu;
}

ToolGroupButton* DrawingToolsBar::createToolGroupButton()
{
    const QVector<ToolGroupButton::ToolEntry> shapeTools =
    {
        { DrawingToolType::Polyline, XCanvasIconType::Polyline, tr("折线")   },
        { DrawingToolType::Curve,    XCanvasIconType::Curve,    tr("曲线")   },
        { DrawingToolType::Rect,     XCanvasIconType::Rect,     tr("矩形")   },
        { DrawingToolType::Ellipse,  XCanvasIconType::Ellipse,  tr("椭圆")   },
        { DrawingToolType::Polygon,  XCanvasIconType::Polygon,  tr("多边形") },
    };
    ToolGroupButton* shapeToolGroup = new ToolGroupButton(shapeTools, this);
    return shapeToolGroup;
}
