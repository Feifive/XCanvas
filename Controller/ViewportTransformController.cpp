#include "ViewportTransformController.h"

#include "../Canvas/Canvas.h"
#include "../Canvas/ICanvasViewport.h"
#include "Global.h"

ViewportTransformController::ViewportTransformController(
    ICanvasViewport* const view,
    ICanvasNavigation* const navigation,
    xcanvas::Canvas* const canvas)
    : m_view(view),
      m_navigation(navigation),
      m_canvas(canvas),
      m_scaleFactor(1.0)
{
}

qreal ViewportTransformController::scaleFactor() const
{
    return m_scaleFactor;
}

bool ViewportTransformController::zoomIn(const QPointF& zoomCenterPoint)
{
    if (!m_view || !m_navigation || m_scaleFactor >= MAX_ZOOM)
    {
        return false;
    }

    constexpr qreal dScale = 1.1;

    const QPointF sceneAnchor = m_view->mapToWorld(zoomCenterPoint);

    m_navigation->scaleBy(dScale);
    m_scaleFactor *= dScale;
    m_scaleFactor = qBound(MIN_ZOOM, m_scaleFactor, MAX_ZOOM);

    const QPointF newSceneAnchor = m_view->mapToWorld(zoomCenterPoint);
    const QPointF delta          = newSceneAnchor - sceneAnchor;
    m_navigation->translateWorld(delta);

    return true;
}

bool ViewportTransformController::zoomOut(const QPointF& zoomCenterPoint)
{
    if (!m_view || !m_navigation || m_scaleFactor <= MIN_ZOOM)
    {
        return false;
    }

    constexpr qreal dScale = 1.0 / 1.1;

    const QPointF sceneAnchor = m_view->mapToWorld(zoomCenterPoint);

    m_navigation->scaleBy(dScale);
    m_scaleFactor *= dScale;
    m_scaleFactor = qBound(MIN_ZOOM, m_scaleFactor, MAX_ZOOM);

    const QPointF newSceneAnchor = m_view->mapToWorld(zoomCenterPoint);
    const QPointF delta          = newSceneAnchor - sceneAnchor;
    m_navigation->translateWorld(delta);

    return true;
}

bool ViewportTransformController::zoomTo(const qreal zoomValue)
{
    if (!m_view || !m_navigation || zoomValue <= 0.0)
    {
        return false;
    }

    const qreal targetScale = qBound(MIN_ZOOM, zoomValue, MAX_ZOOM);
    const QPointF viewAnchor = m_view->viewportRect().center();
    const QPointF sceneAnchor = m_view->mapToWorld(viewAnchor);

    const qreal factor = targetScale / m_scaleFactor;
    if (qFuzzyCompare(factor, 1.0))
    {
        return false;
    }

    m_navigation->scaleBy(factor);
    m_scaleFactor = targetScale;

    const QPointF newSceneAnchor = m_view->mapToWorld(viewAnchor);
    const QPointF delta          = newSceneAnchor - sceneAnchor;
    m_navigation->translateWorld(delta);

    return true;
}

bool ViewportTransformController::fitWidth()
{
    if (!m_view || !m_navigation || !m_canvas)
    {
        return false;
    }

    const qreal scale = m_view->viewportRect().width() / m_canvas->canvasRect().width();
    zoomTo(scale);
    m_navigation->centerOnWorld(m_canvas->canvasRect().center());
    return true;
}

bool ViewportTransformController::fitHeight()
{
    if (!m_view || !m_navigation || !m_canvas)
    {
        return false;
    }

    const qreal scale = m_view->viewportRect().height() / m_canvas->canvasRect().height();
    zoomTo(scale);
    m_navigation->centerOnWorld(m_canvas->canvasRect().center());
    return true;
}

bool ViewportTransformController::fitCanvas()
{
    if (!m_view || !m_navigation || !m_canvas)
    {
        return false;
    }

    const qreal scaleW = m_view->viewportRect().width() / m_canvas->canvasRect().width();
    const qreal scaleH = m_view->viewportRect().height() / m_canvas->canvasRect().height();
    const qreal scale  = qMin(scaleW, scaleH);
    zoomTo(scale);
    m_navigation->centerOnWorld(m_canvas->canvasRect().center());
    return true;
}

bool ViewportTransformController::fitShapes()
{
    if (!m_view || !m_navigation || !m_canvas || !m_canvas->shapeManager() || m_canvas->shapeManager()->isEmpty())
    {
        return false;
    }

    const QRectF rect = m_canvas->shapeManager()->boundingRect();
    const qreal  scaleW = m_view->viewportRect().width() / rect.width();
    const qreal  scaleH = m_view->viewportRect().height() / rect.height();
    const qreal  scale  = qMin(scaleW, scaleH);
    zoomTo(scale);
    m_navigation->centerOnWorld(rect.center());
    return true;
}
