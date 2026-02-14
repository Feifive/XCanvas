#include "mainwindow.h"

#include <qfile.h>

#include "./ui_mainwindow.h"

#include "CanvasWidget.h"
#include "DrawingToolsBar.h"
#include "Layer/LayerPanel.h"

#include <QCloseEvent>
#include <QFileSystemWatcher>
#include <QFontDatabase>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_pDrawingToolsBar(nullptr), m_pCanvasWidget(nullptr)
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
    if (const int fontId = QFontDatabase::addApplicationFont(":/Resource/Font/MiSans-Medium.ttf"); fontId != -1)
    {
        QFont font("MiSans");
        font.setPixelSize(13);
        QApplication::setFont(font);
    }

    if (QFile qssFile(":/Resource/StyleSheet/Default.qss"); qssFile.open(QFile::ReadOnly))
    {
        const QString styleSheet = qssFile.readAll();
        qApp->setStyleSheet(styleSheet);
    }

    QFileSystemWatcher* fileWatcher = new QFileSystemWatcher(this);
    fileWatcher->addPath("/Users/ze/Desktop/QtProjects/XCanvas/Resource/StyleSheet/Default.qss");
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this,
            [this]()
            {
                if (QFile qssFile("/Users/ze/Desktop/QtProjects/XCanvas/Resource/StyleSheet/Default.qss"); qssFile.open(QFile::ReadOnly))
                {
                    const QString styleSheet = qssFile.readAll();
                    qApp->setStyleSheet(styleSheet);
                }
            });

    // 左侧工具栏
    m_pDrawingToolsBar = new DrawingToolsBar(ui->widget_toolbar);
    ui->verticalLayout_toolbar->addWidget(m_pDrawingToolsBar);

    // 画布
    m_pCanvasWidget = new CanvasWidget(ui->widget_canvas);
    ui->verticalLayout_canvas->addWidget(m_pCanvasWidget);

    LayerPanel* pLayerPanel = new LayerPanel(m_pCanvasWidget->layerManager(), ui->widget_rightPanel);
    ui->verticalLayout_rightPanel->addWidget(pLayerPanel);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_pCanvasWidget && !m_pCanvasWidget->maybeSaveBeforeClose())
    {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}
