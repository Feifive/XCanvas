#include "CanvasWidget.h"
#include "EditorSession.h"
#include "Global.h"
#include "MyGraphicsView.h"
#include "RulerWidget.h"
#include <qtfluentwidgets.h>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QScrollBar>

CanvasWidget::CanvasWidget(EditorSession* session, QWidget* parent) : QWidget{parent}
{
    QGridLayout* pGridLayout = new QGridLayout(this);

    m_pGraphicsView  = new MyGraphicsView(session, this);
    m_pGraphicsScene = new QGraphicsScene(this);
    m_pGraphicsScene->setSceneRect(0, 0, 20000, 20000);
    m_pGraphicsView->setScene(m_pGraphicsScene);
    m_pGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_pGraphicsView->setMouseTracking(true);
    m_pGraphicsView->centerOn(10000, 10000);

    m_pRulerHorizontal     = new RulerWidget(Qt::Horizontal, this);
    m_pRulerVertical       = new RulerWidget(Qt::Vertical, this);
    m_pCornerWidget = new QWidget(this);

    m_pRulerHorizontal->attachView(m_pGraphicsView);
    m_pRulerVertical->attachView(m_pGraphicsView);
    m_pCornerWidget->setFixedSize(m_pRulerVertical->width(), m_pRulerHorizontal->height());

    pGridLayout->addWidget(m_pCornerWidget, 0, 0);
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

    if (session)
    {
        connect(session, &EditorSession::zoomChanged,
                m_pRulerHorizontal, QOverload<>::of(&RulerWidget::update));
        connect(session, &EditorSession::zoomChanged,
                m_pRulerVertical, QOverload<>::of(&RulerWidget::update));
    }
    connect(m_pGraphicsView, &MyGraphicsView::documentDisplayNameChanged,
            this, &CanvasWidget::documentDisplayNameChanged);

    connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
            [this](qfw::Theme)
            {
                applyTheme();
                if (m_pRulerHorizontal)
                {
                    m_pRulerHorizontal->update();
                }
                if (m_pRulerVertical)
                {
                    m_pRulerVertical->update();
                }
                if (m_pGraphicsView)
                {
                    m_pGraphicsView->requestFullUpdate();
                }
            });

    applyTheme();
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

QString CanvasWidget::currentDocumentPath() const
{
    return m_pGraphicsView ? m_pGraphicsView->currentDocumentPath() : QString();
}

bool CanvasWidget::openDocumentFile(const QString& path) const
{
    return m_pGraphicsView ? m_pGraphicsView->openDocumentFile(path) : false;
}

void CanvasWidget::applyTheme()
{
    if (!m_pCornerWidget)
    {
        return;
    }

    const bool dark = qfw::isDarkTheme();
    m_pCornerWidget->setStyleSheet(QStringLiteral("background-color: %1;")
                                       .arg(dark ? QStringLiteral("#23262B")
                                                 : QStringLiteral("#E7E9ED")));
}
