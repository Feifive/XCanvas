#include "CanvasWidget.h"
#include "EventBus.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include "RulerWidget.h"
#include <QGraphicsScene>
#include <QGridLayout>
#include <QScrollBar>

CanvasWidget::CanvasWidget(QWidget* parent) : QWidget{parent}
{
    QGridLayout* pGridLayout = new QGridLayout(this);

    m_pGraphicsView  = new MyGraphicsView(this);
    m_pGraphicsScene = new QGraphicsScene(this);
    m_pGraphicsScene->setSceneRect(0, 0, 20000, 20000);
    m_pGraphicsView->setScene(m_pGraphicsScene);
    m_pGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_pGraphicsView->setMouseTracking(true);
    m_pGraphicsView->centerOn(10000, 10000);

    m_pRulerHorizontal     = new RulerWidget(Qt::Horizontal, this);
    m_pRulerVertical       = new RulerWidget(Qt::Vertical, this);
    QWidget* pCornerWidget = new QWidget(this);
    pCornerWidget->setStyleSheet("background-color: #E7E9ED");

    m_pRulerHorizontal->attachView(m_pGraphicsView);
    m_pRulerVertical->attachView(m_pGraphicsView);
    pCornerWidget->setFixedSize(m_pRulerVertical->width(), m_pRulerHorizontal->height());

    pGridLayout->addWidget(pCornerWidget, 0, 0);
    pGridLayout->addWidget(m_pRulerHorizontal, 0, 1);
    pGridLayout->addWidget(m_pRulerVertical, 1, 0);
    pGridLayout->addWidget(m_pGraphicsView, 1, 1);
    pGridLayout->setColumnStretch(1, 1);
    pGridLayout->setRowStretch(1, 1);
    pGridLayout->setSpacing(0);
    pGridLayout->setContentsMargins(0, 0, 0, 0);

    // 滚动或缩放后，标尺会在 paintEvent 里自动通过 mapToScene / transform() 重新计算
    connect(m_pGraphicsView->horizontalScrollBar(), &QScrollBar::valueChanged, m_pRulerHorizontal, QOverload<>::of(&RulerWidget::update));
    connect(m_pGraphicsView->verticalScrollBar(), &QScrollBar::valueChanged, m_pRulerVertical, QOverload<>::of(&RulerWidget::update));

    connect(&EventBus::instance(), &EventBus::zoomChanged,// Qt6 才有；没有的话你在 zoom 那里手动调用
            m_pRulerHorizontal, QOverload<>::of(&RulerWidget::update));
    connect(&EventBus::instance(), &EventBus::zoomChanged, m_pRulerVertical, QOverload<>::of(&RulerWidget::update));
}

xcanvas::LayerManager* CanvasWidget::layerManager()
{
    if (m_pGraphicsView)
    {
        return m_pGraphicsView->layerManager();
    }
    return nullptr;
}

bool CanvasWidget::maybeSaveBeforeClose()
{
    if (!m_pGraphicsView)
    {
        return true;
    }
    return m_pGraphicsView->maybeSaveBeforeClose();
}
