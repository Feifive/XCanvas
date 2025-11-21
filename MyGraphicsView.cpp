#include "MyGraphicsView.h"
#include "BaseDrawingTool.h"
#include "EllipseDrawingTool.h"
#include "PolylineDrawingTool.h"
#include "RectDrawingTool.h"
#include "SelectDrawingTool.h"
#include <QDebug>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

#define MIN_ZOOM 0.1
#define MAX_ZOOM 10.0

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : m_dScaleFactor(1.0), m_eToolType(DrawingToolType::None), m_startPos(-1, -1), m_bDragging(false), m_pBaseDrawingTool(nullptr), m_pShapes(new Shapes), m_bInitPosition(false), QGraphicsView{parent}
{
    m_pScene = new QGraphicsScene;
    m_pScene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    m_pScene->setSceneRect(0, 0, 20000, 20000);

    setScene(m_pScene);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setMouseTracking(true);
}

MyGraphicsView::~MyGraphicsView()
{
    if (m_pBaseDrawingTool)
    {
        delete m_pBaseDrawingTool;
        m_pBaseDrawingTool = nullptr;
    }
}

void MyGraphicsView::SetTool(DrawingToolType type)
{
    if (m_pBaseDrawingTool)
    {
        delete m_pBaseDrawingTool;
        m_pBaseDrawingTool = nullptr;
    }

    switch (type)
    {
    case DrawingToolType::Select:
    {
        m_pBaseDrawingTool = new SelectDrawingTool(this);
    }
    break;
    case DrawingToolType::Rect:
    {
        m_pBaseDrawingTool = new RectDrawingTool(this);
    }
    break;
    case DrawingToolType::Ellipse:
    {
        m_pBaseDrawingTool = new EllipseDrawingTool(this);
    }
    break;
    case DrawingToolType::Line:
    {
        m_pBaseDrawingTool = new LineDrawingTool(this);
    }
    break;
    default:
        m_pBaseDrawingTool = nullptr;
        break;
    }
    m_eToolType = type;

    UpdateCanvas();
}

Shapes* MyGraphicsView::GetCurrentShapes()
{
    return m_pShapes;
}

double MyGraphicsView::GetScaleFactory()
{
    return transform().m11();
}

void MyGraphicsView::UpdateCanvas()
{
    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->DrawTrace();
    }
}

void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        // 记录初始鼠标位置并进入拖动状态
        m_startPos  = event->pos();
        m_bDragging = true;
        setCursor(Qt::ClosedHandCursor);// 设置为闭合手型光标
        event->accept();// 标记事件已处理
    }

    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->mousePressEvent(event);
    }
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_bDragging)
    {
        // 计算鼠标移动的增量
        QPoint delta = event->pos() - m_startPos.toPoint();

        // 水平滚动条：反向移动增量（因为视图移动方向与鼠标相反）
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());

        // 垂直滚动条：同上
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        // 更新记录的最后鼠标位置
        m_startPos = event->pos();
        event->accept();
    }

    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->mouseMoveEvent(event);
    }

    emit mouseMovePos(event->pos());
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton && m_bDragging)
    {
        m_bDragging = false;
        setCursor(Qt::ArrowCursor);// 恢复默认光标
        event->accept();
    }

    if (m_pBaseDrawingTool)
    {
        m_pBaseDrawingTool->mouseReleaseEvent(event);
    }
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    // 获取当前光标在视图中的坐标
    const QPointF cursorViewPos             = event->position();
    const QPointF cursorScenePosBeforeScale = mapToScene(cursorViewPos.toPoint());

    // 计算缩放因子（根据滚轮方向调整）
    const double dScale = event->angleDelta().y() > 0 ? 1.1 : 1 / 1.1;

    // 计算新缩放因子并应用缩放
    if ((m_dScaleFactor == MAX_ZOOM && dScale == 1.1) || (m_dScaleFactor == MIN_ZOOM && dScale == 1 / 1.1))
    {
        return;
    }
    m_dScaleFactor *= dScale;
    m_dScaleFactor = qBound(MIN_ZOOM, m_dScaleFactor, MAX_ZOOM);
    QTransform transform;
    transform.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transform);

    // 计算缩放后的视图中心调整
    const QPointF cursorScenePos = mapToScene(cursorViewPos.toPoint());
    const QPointF viewCenter     = mapToScene(viewport()->rect().center());
    const QPointF adjustedCenter = viewCenter + (cursorScenePosBeforeScale - cursorScenePos);

    // 更新视图中心
    centerOn(adjustedCenter);

    UpdateCanvas();
}

void MyGraphicsView::resizeEvent(QResizeEvent* event)
{
    // if(m_pScene)
    // {
    //     m_pScene->setSceneRect(0, 0, event->size().width(), event->size().height());
    // }
    return QGraphicsView::resizeEvent(event);
}

void MyGraphicsView::showEvent(QShowEvent* event)
{
    QGraphicsView::showEvent(event);

    if (!m_bInitPosition)
    {
        horizontalScrollBar()->setSliderPosition(0);
        verticalScrollBar()->setSliderPosition(0);
        m_bInitPosition = true;
    }
}
