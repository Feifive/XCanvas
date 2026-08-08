#include "ViewRenderController.h"

#include "AppSettings.h"
#include "Canvas/SelectionOutlineStyle.h"
#include "../Canvas/Canvas.h"
#include "../Canvas/ICanvasViewport.h"
#include "MyMath.h"
#include "../Shape/Shape.h"
#include <qtfluentwidgets.h>

#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QtMath>
#include <cmath>

ViewRenderController::ViewRenderController(
    ICanvasViewport* const  view,
    xcanvas::Canvas* const  canvas,
    QSvgRenderer* const     rotateHandle)
    : m_view(view),
      m_canvas(canvas),
      m_rotateHandle(rotateHandle)
{
}

void ViewRenderController::setRotateHandle(QSvgRenderer* const rotateHandle)
{
    m_rotateHandle = rotateHandle;
}

void ViewRenderController::setSuppressedShape(const xcanvas::Shape* shape)
{
    m_suppressedShape = shape;
}

void ViewRenderController::setSelectionHandlesVisible(const bool visible)
{
    m_selectionHandlesVisible = visible;
}

void ViewRenderController::setSelectionDashPhase(const qreal phase)
{
    m_selectionDashPhase = phase;
}

void ViewRenderController::drawBackground(QPainter* painter, const QRectF& rect)
{
    drawCanvas(painter);
    drawGrid(painter);
    drawNormalShapes(painter, rect);
}

void ViewRenderController::drawForeground(QPainter* painter, const QRectF& rect)
{
    drawSelectedShapes(painter, rect);
}

void ViewRenderController::drawShapes(QPainter* painter, const QRectF& visibleRect)
{
    drawNormalShapes(painter, visibleRect);
    drawSelectedShapes(painter, visibleRect);
}

void ViewRenderController::drawNormalShapes(QPainter* painter, const QRectF& visibleRect)
{
    if (!m_canvas || !m_canvas->layerManager())
    {
        return;
    }

    QList<xcanvas::LayerParameter*> layers = m_canvas->layerManager()->getOrderedLayers();
    for (auto* layer : layers)
    {
        if (!layer->visible)
        {
            continue;
        }

        if (layer->mode == xcanvas::ProcessMode::Cut)
        {
            painter->save();

            QPen pen(layer->color);
            pen.setWidth(1);
            pen.setCosmetic(true);
            pen.setStyle(Qt::SolidLine);

            painter->setPen(pen);

            for (const auto shape : layer->shapes)
            {
                if (shape == m_suppressedShape)
                {
                    continue;
                }
                if (visibleRect.intersects(shape->boundingRect()) && !shape->isSelected())
                {
                    shape->draw(painter);
                }
            }

            painter->restore();
        }
        else if (layer->mode == xcanvas::ProcessMode::Scan)
        {
            painter->save();

            painter->setPen(Qt::NoPen);
            painter->setBrush(layer->color);

            QPainterPath path;
            path.setFillRule(Qt::OddEvenFill);
            for (const auto shape : layer->shapes)
            {
                if (shape == m_suppressedShape)
                {
                    continue;
                }
                if (visibleRect.intersects(shape->boundingRect()))
                {
                    path.addPath(shape->path());
                }
            }
            painter->drawPath(path);

            painter->restore();
        }
        else if (layer->mode == xcanvas::ProcessMode::Image)
        {
            for (const auto shape : layer->shapes)
            {
                if (shape == m_suppressedShape)
                {
                    continue;
                }
                shape->draw(painter);
            }
        }
    }
}

void ViewRenderController::drawSelectedShapes(QPainter* painter, const QRectF& visibleRect)
{
    if (!m_canvas || !m_canvas->shapeManager())
    {
        return;
    }

    const QSet<xcanvas::Shape*> selected = m_canvas->shapeManager()->selectedShapes();
    if (selected.isEmpty())
    {
        return;
    }

    painter->save();

    painter->setBrush(Qt::NoBrush);

    for (xcanvas::Shape* shape : selected)
    {
        if (!shape->isVisible())
        {
            m_canvas->shapeManager()->deselectShape(shape);
            continue;
        }
        if (shape == m_suppressedShape)
        {
            continue;
        }
        if (visibleRect.intersects(shape->boundingRect()))
        {
            painter->setPen(xcanvas::SelectionOutlineStyle::pen(shape->color(), m_selectionDashPhase));
            painter->drawPath(shape->path());
        }
    }

    if (!selected.isEmpty() && m_selectionHandlesVisible)
    {
        drawTrace(painter);
    }

    painter->restore();
}

void ViewRenderController::drawGrid(QPainter* const p)
{
    if (!m_view || !m_canvas || AppSettings::instance().gridContrast() == AppSettings::GridContrast::Off)
    {
        return;
    }

    const QRectF canvasSceneRect  = m_canvas->canvasRect();
    const QRectF viewSceneRect    = m_view->mapToWorld(m_view->viewportRect());
    const QRectF visibleSceneRect = canvasSceneRect.intersected(viewSceneRect);

    if (!visibleSceneRect.isValid() || visibleSceneRect.isEmpty())
    {
        return;
    }

    const double scale      = m_view->zoomScale();
    const double step       = gridStep(scale);
    const int    majorCount = 10;

    QColor minorColor;
    QColor majorColor;
    const bool dark = qfw::isDarkTheme();

    switch (AppSettings::instance().gridContrast())
    {
    case AppSettings::GridContrast::Low:
        majorColor = dark ? QColor("#4A4F57") : QColor("#E0E0E0");
        minorColor = dark ? QColor("#3D4148") : QColor("#ECECEC");
        break;
    case AppSettings::GridContrast::Medium:
        majorColor = dark ? QColor("#59606A") : QColor("#D0D0D0");
        minorColor = dark ? QColor("#474D56") : QColor("#E4E4E4");
        break;
    case AppSettings::GridContrast::High:
        majorColor = dark ? QColor("#6E7683") : QColor("#C0C0C0");
        minorColor = dark ? QColor("#565D68") : QColor("#DCDCDC");
        break;
    default:
        break;
    }

    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);

    const QTransform oldWorldTransform = p->worldTransform();
    p->setWorldTransform(QTransform());

    const QRectF canvasViewRect = m_view->mapFromWorld(visibleSceneRect);
    p->setClipRect(canvasViewRect);

    const double s0x = visibleSceneRect.left();
    const double s1x = visibleSceneRect.right();

    qint64 firstIndexX = qFloor((s0x + 1e-12) / step);
    double x0          = firstIndexX * step;

    QVector<QLineF> minorLines;
    QVector<QLineF> majorLines;

    for (double x = x0; x <= s1x + step; x += step, ++firstIndexX)
    {
        QPointF v  = m_view->mapFromWorld(QPointF(x, visibleSceneRect.top()));
        double  vx = std::round(v.x()) - 0.5;

        if ((firstIndexX % majorCount) == 0)
        {
            majorLines.append(QLineF(vx, canvasViewRect.top(), vx, canvasViewRect.bottom()));
        }
        else
        {
            minorLines.append(QLineF(vx, canvasViewRect.top(), vx, canvasViewRect.bottom()));
        }
    }

    const double s0y = visibleSceneRect.top();
    const double s1y = visibleSceneRect.bottom();

    qint64 firstIndexY = qFloor((s0y + 1e-12) / step);
    double y0          = firstIndexY * step;

    for (double y = y0; y <= s1y + step; y += step, ++firstIndexY)
    {
        QPointF v  = m_view->mapFromWorld(QPointF(visibleSceneRect.left(), y));
        double  vy = std::round(v.y()) - 0.5;

        if ((firstIndexY % majorCount) == 0)
        {
            majorLines.append(QLineF(canvasViewRect.left(), vy, canvasViewRect.right(), vy));
        }
        else
        {
            minorLines.append(QLineF(canvasViewRect.left(), vy, canvasViewRect.right(), vy));
        }
    }

    p->setPen(QPen(minorColor, 0));
    p->drawLines(minorLines);

    p->setPen(QPen(majorColor, 0));
    p->drawLines(majorLines);

    p->setWorldTransform(oldWorldTransform);
    p->restore();
}

double ViewRenderController::gridStep(const double scale) const
{
    static const double steps[] = {0.1, 0.5, 1.0, 5.0, 10.0, 50.0, 100.0, 500.0};
    const double        minPixelSpacing = 10.0;

    for (const double step : steps)
    {
        if (step * scale >= minPixelSpacing)
        {
            return step;
        }
    }

    return 500.0;
}

void ViewRenderController::drawTrace(QPainter* painter)
{
    if (!m_view || !m_canvas || !m_canvas->shapeManager() || m_canvas->shapeManager()->isEmpty())
    {
        return;
    }

    const QRectF rect = m_canvas->shapeManager()->selectedBoundingRect();
    if (!rect.isValid())
    {
        return;
    }

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#90909B"));

    const auto [resizeRects, rotateRect] = xcanvas::geometryMath::traceRects(rect, m_view->zoomScale());
    for (const QRectF& r : resizeRects)
    {
        painter->drawRect(r);
    }

    if (m_rotateHandle)
    {
        const QTransform old = painter->worldTransform();
        painter->setWorldTransform(QTransform());
        const QRectF viewRotateRect = m_view->mapFromWorld(rotateRect);
        m_rotateHandle->render(painter, viewRotateRect);
        painter->setWorldTransform(old);
    }

    painter->restore();
}

void ViewRenderController::drawCanvas(QPainter* painter)
{
    if (!m_view || !m_canvas)
    {
        return;
    }

    painter->save();

    const QTransform oldWorldTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    const bool dark = qfw::isDarkTheme();
    painter->fillRect(m_view->hostRect(), workspaceBackgroundColor());
    painter->setWorldTransform(oldWorldTransform);

    painter->setPen(Qt::NoPen);
    painter->setBrush(dark ? QColor("#2A2E36") : QColor("#FFFFFF"));
    painter->drawRect(m_canvas->canvasRect());

    painter->restore();
}

QColor ViewRenderController::workspaceBackgroundColor()
{
    return qfw::isDarkTheme() ? QColor("#1D2025") : QColor("#E7E9ED");
}
