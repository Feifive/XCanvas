#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "DXFTranslator.h"
#include "DrawingToolsBar.h"
#include "MyGraphicsView.h"
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

    // 左侧工具栏
    m_pDrawingToolsBar = new DrawingToolsBar(this);
    ui->verticalLayout_toolbar->addWidget(m_pDrawingToolsBar);

    // 标尺、画布
    RulerWidget*    pRulerHorizontal = new RulerWidget(Qt::Horizontal, ui->centralwidget);
    RulerWidget*    pRulerVertical   = new RulerWidget(Qt::Vertical, ui->centralwidget);
    MyGraphicsView* pGraphicsView    = new MyGraphicsView(ui->centralwidget);
    QWidget*        pCornerWidget    = new QWidget(ui->centralwidget);
    pRulerHorizontal->AttachView(pGraphicsView);
    pRulerVertical->AttachView(pGraphicsView);
    pCornerWidget->setFixedSize(pRulerVertical->width(), pRulerHorizontal->height());

    ui->gridLayout->addWidget(pCornerWidget, 0, 0);
    ui->gridLayout->addWidget(pRulerHorizontal, 0, 1);
    ui->gridLayout->addWidget(pRulerVertical, 1, 0);
    ui->gridLayout->addWidget(pGraphicsView, 1, 1);
    ui->gridLayout->setColumnStretch(1, 1);
    ui->gridLayout->setRowStretch(1, 1);
    ui->gridLayout->setSpacing(0);
    ui->gridLayout->setContentsMargins(0, 0, 0, 0);

    connect(pGraphicsView, &MyGraphicsView::mouseMovePos,
            [pRulerHorizontal, pRulerVertical](QPointF pos)
            {
                pRulerHorizontal->setSlidingLinePos(pos.x());
                pRulerVertical->setSlidingLinePos(pos.y());
            });
    connect(pGraphicsView->horizontalScrollBar(), &QScrollBar::valueChanged, [pGraphicsView, pRulerHorizontal] { pRulerHorizontal->setOffset(pGraphicsView->mapToScene(0, 0).x()); });
    connect(pGraphicsView->verticalScrollBar(), &QScrollBar::valueChanged, [pGraphicsView, pRulerVertical] { pRulerVertical->setOffset(pGraphicsView->mapToScene(0, 0).y()); });

    connect(m_pDrawingToolsBar, &DrawingToolsBar::DrawingToolRequest, pGraphicsView, &MyGraphicsView::SetTool);

    // QGraphicsPixmapItem* pPixmapItem = new QGraphicsPixmapItem;
    // pPixmapItem->setPixmap(QPixmap("/Users/ze/Downloads/test.jpg"));
    // pGraphicsView->scene()->addItem(pPixmapItem);

    connect(m_pDrawingToolsBar, &DrawingToolsBar::Imported, this,
            [pGraphicsView](const QString& filePath)
            {
                DXFTranslator translator;
                Shapes*       pShapes = pGraphicsView->GetCurrentShapes();
                translator.Load(filePath, pShapes);
                qDebug() << "importe shape count:" << pShapes->Count();
                for (int i = 0; i < pShapes->Count(); i++)
                {
                    Shape* pShape = pShapes->GetShape(i);
                    pShape->Offset(QPointF(100, 900));
                    pGraphicsView->scene()->addItem(pShape);
                }
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}
