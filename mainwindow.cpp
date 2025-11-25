#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "CanvasWidget.h"
#include "DXFTranslator.h"
#include "DrawingToolsBar.h"
#include "rulerwidget.h"

#include <QButtonGroup>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QLayout>
#include <QResizeEvent>
#include <QScrollBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->widget_toolbar->setStyleSheet("background-color: white");

    // 左侧工具栏
    m_pDrawingToolsBar = new DrawingToolsBar(this);
    ui->verticalLayout_toolbar->addWidget(m_pDrawingToolsBar);

    // 画布
    CanvasWidget* pCanvasWidget = new CanvasWidget(ui->widget_canvas);
    ui->verticalLayout_canvas->addWidget(pCanvasWidget);

    // connect(pGraphicsView, &MyGraphicsView::ToolFinished, m_pDrawingToolsBar, &DrawingToolsBar::OnToolFinished);
    connect(m_pDrawingToolsBar, &DrawingToolsBar::DrawingToolRequest, pCanvasWidget, &CanvasWidget::SetTool);

    // QGraphicsPixmapItem* pPixmapItem = new QGraphicsPixmapItem;
    // pPixmapItem->setPixmap(QPixmap("/Users/ze/Downloads/test.jpg"));
    // pGraphicsView->scene()->addItem(pPixmapItem);

    // connect(m_pDrawingToolsBar, &DrawingToolsBar::Imported, this,
    //         [pGraphicsView](const QString& filePath)
    //         {
    //             DXFTranslator translator;
    //             Shapes*       pShapes = pGraphicsView->GetCurrentShapes();
    //             translator.Load(filePath, pShapes);
    //             qDebug() << "importe shape count:" << pShapes->Count();
    //             for (int i = 0; i < pShapes->Count(); i++)
    //             {
    //                 Shape* pShape = pShapes->GetShape(i);
    //                 pShape->Offset(QPointF(100, 900));
    //                 pGraphicsView->scene()->addItem(pShape);
    //             }
    //         });
}

MainWindow::~MainWindow()
{
    delete ui;
}
