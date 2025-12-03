#include "DrawingToolsBar.h"
#include "EventBus.h"
#include <QButtonGroup>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

DrawingToolsBar::DrawingToolsBar(QWidget* parent) : QToolBar{parent}
{
    setOrientation(Qt::Vertical);
    setIconSize(QSize(24, 24));
    setMovable(false);
    setFloatable(false);

    setStyleSheet(R"(
        QToolBar {
            background: transparent;
            spacing: 8px;
            border: none;
        }

        QToolButton {
            padding: 0;
            margin: 0;
            border: none;
            border-radius: 5px;
        }

        QToolButton:hover {
            background: rgb(246,247,249);
        }

        QToolButton:checked {
            background: rgb(239,239,243);
        }
    )");

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
    m_pText         = MakeButton(":/Resource/Icons/Text.svg");
    m_pPolylineTool = MakeButton(":/Resource/Icons/Polyline.svg");
    m_pCurveTool    = MakeButton(":/Resource/Icons/Curve.svg");
    m_pRectTool     = MakeButton(":/Resource/Icons/Rect.svg");
    m_pEllipseTool  = MakeButton(":/Resource/Icons/Ellipse.svg");
    m_pPolygonTool  = MakeButton(":/Resource/Icons/Polygon.svg");
    // 垂直弹簧
    QWidget* pSpring = new QWidget(this);
    pSpring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWidget(pSpring);
    m_pSelectTool = MakeButton(":/Resource/Icons/Select.svg");

    connect(m_pSelectTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Select); });
    connect(m_pImport, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Import); });
    connect(m_pText, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Text); });
    connect(m_pRectTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Rect); });
    connect(m_pPolylineTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Polyline); });
    connect(m_pEllipseTool, &QToolButton::clicked, this, [=] { emit EventBus::instance().switchTool(DrawingToolType::Ellipse); });
    connect(&EventBus::instance(), &EventBus::switchTool, this, &DrawingToolsBar::onSwitchTool);

    m_pSelectTool->setChecked(true);
}

DrawingToolsBar::~DrawingToolsBar()
{
}

void DrawingToolsBar::onSwitchTool(DrawingToolType toolType)
{
    if (toolType == DrawingToolType::Select)
    {
		m_pSelectTool->setChecked(true);
    }
}
