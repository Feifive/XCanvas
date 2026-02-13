#include "DrawingToolsBar.h"
#include "AppSettings.h"
#include "EventBus.h"
#include "XMenu.h"
#include <QActionGroup>
#include <QButtonGroup>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QKeySequence>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

DrawingToolsBar::DrawingToolsBar(QWidget* parent) : QToolBar{parent}
{
    setObjectName("DrawingToolsBar");
    setOrientation(Qt::Vertical);
    setIconSize(QSize(24, 24));
    setMovable(false);
    setFloatable(false);

    m_pGroup = new QButtonGroup(this);
    m_pGroup->setExclusive(true);

    auto MakeButton = [&](const QString& iconPath, bool addToGroup = true)
    {
        auto* pToolButton = new QToolButton(this);
        pToolButton->setFixedSize(QSize(36, 36));
        pToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        pToolButton->setIcon(QIcon(iconPath));
        if (addToGroup)
        {
            pToolButton->setCheckable(true);
            m_pGroup->addButton(pToolButton);
        }
        addWidget(pToolButton);
        return pToolButton;
    };

    m_pMainMenu     = MakeButton(":/Resource/Icons/MainMenu.svg", false);
    m_pImport       = MakeButton(":/Resource/Icons/Import.svg", false);
    m_pPolylineTool = MakeButton(":/Resource/Icons/Polyline.svg");
    m_pCurveTool    = MakeButton(":/Resource/Icons/Curve.svg");
    m_pRectTool     = MakeButton(":/Resource/Icons/Rect.svg");
    m_pEllipseTool  = MakeButton(":/Resource/Icons/Ellipse.svg");
    m_pPolygonTool  = MakeButton(":/Resource/Icons/Polygon.svg");
    m_pText         = MakeButton(":/Resource/Icons/Text.svg");

    // 垂直弹簧
    auto* pSpring = new QWidget(this);
    pSpring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(pSpring);
    m_pSelectTool = MakeButton(":/Resource/Icons/Select.svg");

    connect(m_pSelectTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Select); });
    connect(m_pImport, &QToolButton::clicked, this, [=] { emit EventBus::instance().importFileRequested(); });
    connect(m_pText, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Text); });
    connect(m_pRectTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Rect); });
    connect(m_pPolylineTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Polyline); });
    connect(m_pEllipseTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Ellipse); });
    connect(m_pCurveTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Curve); });
    connect(m_pPolygonTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Polygon); });
    connect(&EventBus::instance(), &EventBus::finishDrawing, this, &DrawingToolsBar::onFinishDrawing);

    m_pSelectTool->setChecked(true);
    initMainMenu();
}

DrawingToolsBar::~DrawingToolsBar() = default;

void DrawingToolsBar::onFinishDrawing() const {
    m_pSelectTool->setChecked(true);
}

void DrawingToolsBar::initMainMenu()
{
    m_pMainMenu->setPopupMode(QToolButton::InstantPopup);

    auto* group = new QActionGroup(this);
    group->setExclusive(true);

    auto* gridSetting       = new XMenu("画布和网格", this);
    auto  addContrastAction = [&](const QString& text, AppSettings::GridContrast value)
    {
        QAction* action = gridSetting->addAction(text);
        action->setCheckable(true);
        group->addAction(action);

        if (AppSettings::instance().gridContrast() == value)
        {
            action->setChecked(true);
            action->setIcon(QIcon(":/Resource/Icons/PickOn.svg"));
        }

        connect(action, &QAction::triggered, this,
                [action, value]()
                {
                    for (auto a : action->actionGroup()->actions())
                    {
                        a->setIcon(QIcon());
                    }
                    action->setIcon(QIcon(":/Resource/Icons/PickOn.svg"));
                    AppSettings::instance().setGridContrast(value);
                });

        return action;
    };

    auto* mainMenu = new XMenu(this);
    mainMenu->setMinimumSize(180, 30);

    gridSetting->setMinimumSize(150, 30);
    auto* showGrid = new QWidgetAction(this);
    auto* label    = new QLabel(this);
    label->setObjectName("menuSection");
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setText(tr("浅色/深色背景"));
    showGrid->setDefaultWidget(label);
    gridSetting->addAction(showGrid);
    gridSetting->addSeparator();
    addContrastAction("网格-强", AppSettings::GridContrast::High);
    addContrastAction("网格-中", AppSettings::GridContrast::Medium);
    addContrastAction("网格-弱", AppSettings::GridContrast::Low);
    addContrastAction("网格-关闭", AppSettings::GridContrast::Off);
    auto* fileMenu = new XMenu("文件", this);
    auto* newAction = fileMenu->addAction(tr("新建"));
    fileMenu->addSeparator();
    auto* openAction = fileMenu->addAction(tr("打开项目"));
    auto* importAction = fileMenu->addAction(tr("导入"));
    fileMenu->addSeparator();
    auto* saveAction = fileMenu->addAction(tr("保存"));
    auto* saveAsAction = fileMenu->addAction(tr("另存为"));

    auto attachShortcut = [](QAction* action, const QKeySequence& sequence)
    {
        action->setShortcut(sequence);
        action->setShortcutVisibleInContextMenu(true);
    };
    attachShortcut(newAction, QKeySequence::New);
    attachShortcut(openAction, QKeySequence::Open);
    attachShortcut(importAction, QKeySequence(Qt::CTRL | Qt::Key_I));
    attachShortcut(saveAction, QKeySequence::Save);
    attachShortcut(saveAsAction, QKeySequence::SaveAs);

    connect(newAction, &QAction::triggered, this, [] { emit EventBus::instance().newFileRequested(); });
    connect(openAction, &QAction::triggered, this, [] { emit EventBus::instance().openFileRequested(); });
    connect(saveAction, &QAction::triggered, this, [] { emit EventBus::instance().saveFileRequested(); });
    connect(saveAsAction, &QAction::triggered, this, [] { emit EventBus::instance().saveFileAsRequested(); });
    connect(importAction, &QAction::triggered, this, [] { emit EventBus::instance().importFileRequested(); });

    mainMenu->addMenu(fileMenu);
    mainMenu->addMenu(gridSetting);
    m_pMainMenu->setMenu(mainMenu);
}
