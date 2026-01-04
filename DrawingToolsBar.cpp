#include "DrawingToolsBar.h"
#include "EventBus.h"
#include <QButtonGroup>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QLabel>

DrawingToolsBar::DrawingToolsBar(QWidget* parent) : QToolBar{parent}
{
    setOrientation(Qt::Vertical);
    setIconSize(QSize(24, 24));
    setMovable(false);
    setFloatable(false);

    m_pGroup = new QButtonGroup(this);
    m_pGroup->setExclusive(true);

    auto MakeButton = [&](const QString& iconPath, bool addToGroup = true)
    {
        QToolButton* pToolButton = new QToolButton(this);
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

    m_pMainMenu->setPopupMode(QToolButton::InstantPopup);

    auto* mainMenu = new QMenu(this);
    mainMenu->setMinimumSize(180, 30);
    auto* gridSetting = new QMenu("画布和网格", this);
    gridSetting->setMinimumSize(150, 30);
    auto* showGrid = new QWidgetAction(this);
    auto* label = new QLabel(this);
    label->setObjectName("menuSection");
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setText(tr("显示/隐藏网格"));
    showGrid->setDefaultWidget(label);
    gridSetting->addAction(showGrid);
    gridSetting->addSeparator();
    gridSetting->addAction(QIcon(":/Resource/Icons/PickOn.svg"), "网格-强");
    gridSetting->addAction("网格-中");
    gridSetting->addAction("网格-弱");
    auto* fileMenu = new QMenu("文件", this);
    auto* editeMenu = new QMenu("编辑", this);
    mainMenu->addMenu(fileMenu);
    mainMenu->addMenu(editeMenu);
    mainMenu->addMenu(gridSetting);
    m_pMainMenu->setMenu(mainMenu);

    // 垂直弹簧
    auto* pSpring = new QWidget(this);
    pSpring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(pSpring);
    m_pSelectTool = MakeButton(":/Resource/Icons/Select.svg");

    connect(m_pSelectTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Select); });
    connect(m_pImport, &QToolButton::clicked, this, [=] { emit EventBus::instance().importFileRequested();});
    connect(m_pText, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Text); });
    connect(m_pRectTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Rect); });
    connect(m_pPolylineTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Polyline); });
    connect(m_pEllipseTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Ellipse); });
    connect(m_pCurveTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Curve); });
    connect(m_pPolygonTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Polygon); });
    connect(&EventBus::instance(), &EventBus::finishDrawing, this, &DrawingToolsBar::onFinishDrawing);

    m_pSelectTool->setChecked(true);
}

DrawingToolsBar::~DrawingToolsBar()
{
}

void DrawingToolsBar::onFinishDrawing()
{
    m_pSelectTool->setChecked(true);
}
