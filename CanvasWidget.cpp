#include "CanvasWidget.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include "rulerwidget.h"
#include <QGraphicsScene>
#include <QGridLayout>
#include <QScrollBar>

CanvasWidget::CanvasWidget(QWidget* parent) : QWidget{parent}
{
    QGridLayout* pGridLayout = new QGridLayout(this);

    m_pGraphicsView  = new MyGraphicsView(this);
    m_pGraphicsScene = new QGraphicsScene(this);
    m_pGraphicsScene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    m_pGraphicsScene->setSceneRect(0, 0, 20000, 20000);
    m_pGraphicsView->setScene(m_pGraphicsScene);
    m_pGraphicsView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    m_pGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_pGraphicsView->setMouseTracking(true);
    m_pGraphicsView->SetTool(DrawingToolType::Select);
    m_pGraphicsView->centerOn(0, 0);

    m_pRulerHorizontal     = new RulerWidget(Qt::Horizontal, this);
    m_pRulerVertical       = new RulerWidget(Qt::Vertical, this);
    QWidget* pCornerWidget = new QWidget(this);

    m_pRulerHorizontal->AttachView(m_pGraphicsView);
    m_pRulerVertical->AttachView(m_pGraphicsView);
    pCornerWidget->setFixedSize(m_pRulerVertical->width(), m_pRulerHorizontal->height());

    pGridLayout->addWidget(pCornerWidget, 0, 0);
    pGridLayout->addWidget(m_pRulerHorizontal, 0, 1);
    pGridLayout->addWidget(m_pRulerVertical, 1, 0);
    pGridLayout->addWidget(m_pGraphicsView, 1, 1);
    pGridLayout->setColumnStretch(1, 1);
    pGridLayout->setRowStretch(1, 1);
    pGridLayout->setSpacing(0);
    pGridLayout->setContentsMargins(0, 0, 0, 0);

    connect(m_pGraphicsView, &MyGraphicsView::mouseMovePos,
            [this](QPointF pos)
            {
                m_pRulerHorizontal->setSlidingLinePos(pos.x());
                m_pRulerVertical->setSlidingLinePos(pos.y());
            });
    connect(m_pGraphicsView->horizontalScrollBar(), &QScrollBar::valueChanged, [this] { m_pRulerHorizontal->setOffset(m_pGraphicsView->mapToScene(0, 0).x()); });
    connect(m_pGraphicsView->verticalScrollBar(), &QScrollBar::valueChanged, [this] { m_pRulerVertical->setOffset(m_pGraphicsView->mapToScene(0, 0).y()); });
}

void CanvasWidget::SetTool(DrawingToolType toolType)
{
    if (m_pGraphicsView)
    {
        m_pGraphicsView->SetTool(toolType);
    }
}
