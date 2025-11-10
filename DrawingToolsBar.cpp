#include "DrawingToolsBar.h"
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

DrawingToolsBar::DrawingToolsBar(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout*  pVBoxLayout  = new QVBoxLayout(this);
    pVBoxLayout->setSpacing(12);
    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    pButtonGroup->setExclusive(true);

    QPushButton* pSelectTool      = new QPushButton("Select", this);
    pSelectTool->setCheckable(true);
    QPushButton* pDrawRectTool    = new QPushButton("Rect", this);
    pDrawRectTool->setCheckable(true);
    QPushButton* pDrawLineTool    = new QPushButton("Line", this);
    pDrawLineTool->setCheckable(true);
    QPushButton* pDrawEllipseTool = new QPushButton("Ellipse", this);
    pDrawEllipseTool->setCheckable(true);

    pVBoxLayout->addWidget(pSelectTool);
    pVBoxLayout->addWidget(pDrawRectTool);
    pVBoxLayout->addWidget(pDrawLineTool);
    pVBoxLayout->addWidget(pDrawEllipseTool);
    pVBoxLayout->addStretch();

    pButtonGroup->addButton(pSelectTool);
    pButtonGroup->addButton(pDrawRectTool);
    pButtonGroup->addButton(pDrawLineTool);
    pButtonGroup->addButton(pDrawEllipseTool);

    connect(pSelectTool, &QPushButton::clicked, this, [=](bool checked)
    {
        emit DrawingToolRequest(DrawingToolType::Select);
    });
    connect(pDrawRectTool, &QPushButton::clicked, this, [=](bool checked)
    {
        emit DrawingToolRequest(DrawingToolType::Rect);
    });
    connect(pDrawLineTool, &QPushButton::clicked, this, [=](bool checked)
    {
        emit DrawingToolRequest(DrawingToolType::Line);
    });
    connect(pDrawEllipseTool, &QPushButton::clicked, this, [=](bool checked)
    {
        emit DrawingToolRequest(DrawingToolType::Ellipse);
    });
}

DrawingToolsBar::~DrawingToolsBar()
{

}
