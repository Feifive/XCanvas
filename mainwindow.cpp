#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "CanvasWidget.h"
#include "DrawingToolsBar.h"

#include <QFontDatabase>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    ui->widget_toolbar->setStyleSheet("background-color: white");

    // 左侧工具栏
    m_pDrawingToolsBar = new DrawingToolsBar(this);
    ui->verticalLayout_toolbar->addWidget(m_pDrawingToolsBar);

    // 画布
    CanvasWidget* pCanvasWidget = new CanvasWidget(ui->widget_canvas);
    ui->verticalLayout_canvas->addWidget(pCanvasWidget);

    // 设置字体
    QFont font("PingFang SC");
    font.setPixelSize(15);
    qApp->setFont(font);
}
