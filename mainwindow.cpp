#include "mainwindow.h"

#include <qfile.h>

#include "./ui_mainwindow.h"

#include "CanvasWidget.h"
#include "DrawingToolsBar.h"

#include <QFileSystemWatcher>
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
    if (const int fontId = QFontDatabase::addApplicationFont(":/Resource/Font/MiSans-Medium.ttf"); fontId != -1)
    {
        QFont font("MiSans");
        font.setPixelSize(15);
        QApplication::setFont(font);
    }

    if (QFile qssFile(":/Resource/StyleSheet/Default.qss"); qssFile.open(QFile::ReadOnly))
    {
        const QString styleSheet = qssFile.readAll();
        qApp->setStyleSheet(styleSheet);
    }

    // QFileSystemWatcher* fileWatcher = new QFileSystemWatcher(this);
    // fileWatcher->addPath("D:\\code\\XCanvas\\Resource\\StyleSheet\\Default.qss");
    // connect(fileWatcher, &QFileSystemWatcher::fileChanged, this,
    //         [this]()
    //         {
    //             if (QFile qssFile("D:\\code\\XCanvas\\Resource\\StyleSheet\\Default.qss"); qssFile.open(QFile::ReadOnly))
    //             {
    //                 const QString styleSheet = qssFile.readAll();
    //                 qApp->setStyleSheet(styleSheet);
    //             }
    //         });

    // 左侧工具栏
    m_pDrawingToolsBar = new DrawingToolsBar(this);
    ui->verticalLayout_toolbar->addWidget(m_pDrawingToolsBar);

    // 画布
    CanvasWidget* pCanvasWidget = new CanvasWidget(ui->widget_canvas);
    ui->verticalLayout_canvas->addWidget(pCanvasWidget);
}
