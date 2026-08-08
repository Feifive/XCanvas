#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include "CanvasCamera.h"
#include "ICanvasViewport.h"
#include "RulerRenderer.h"

#include <QAbstractScrollArea>

class QPainter;

namespace xcanvas
{

class CanvasView : public QAbstractScrollArea, public ICanvasViewport, public ICanvasNavigation
{
    Q_OBJECT
  public:
    explicit CanvasView(QWidget* parent = nullptr);

    QWidget* viewportWidget() const override;
    QWidget* hostWidget() const override;
    QRect viewportRect() const override;
    QRect hostRect() const override;
    QRectF worldRect() const override;
    qreal zoomScale() const override;
    QPointF mapToWorld(const QPointF& viewPoint) const override;
    QPointF mapFromWorld(const QPointF& worldPoint) const override;
    QRectF mapToWorld(const QRect& viewRect) const override;
    QRectF mapFromWorld(const QRectF& worldRect) const override;
    QRectF visibleWorldRect() const;
    QPoint mapFromGlobalToView(const QPoint& globalPoint) const override;
    QPoint mapFromViewToGlobal(const QPoint& viewPoint) const override;
    void setInputMethodEnabled(bool enabled) override;
    void setViewCursor(const QCursor& cursor) override;
    Qt::CursorShape viewCursorShape() const override;
    void focusViewport() override;
    void requestUpdate() const override;

    void scaleBy(qreal factor) override;
    void translateWorld(const QPointF& delta) override;
    void centerOnWorld(const QPointF& point) override;
    void scrollByViewDelta(const QPoint& delta) override;
    void scrollAxisBy(bool horizontal, int delta) override;

    const CanvasCamera& camera() const;
    void setWorldRect(const QRectF& rect);
    bool setZoomScale(qreal scale);
    bool zoomAt(const QPointF& viewAnchor, qreal targetScale);

    const RulerRenderer& rulerRenderer() const;
    RulerRenderer::HitArea rulerHitArea(const QPoint& viewPoint) const;
    bool isInRuler(const QPoint& viewPoint) const;

  signals:
    void cameraChanged();
    void viewportGeometryChanged(const QRect& viewportRect);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    virtual void drawViewportBackground(QPainter& painter, const QRect& viewportRect);
    virtual void drawWorld(QPainter& painter, const QRectF& visibleWorldRect);
    virtual void drawViewOverlay(QPainter& painter, const QRect& viewportRect);
    virtual QColor rulerBaseColor() const;

    void focusForPointerPress();
    bool filterRulerMousePress(QMouseEvent* event);
    bool filterRulerMouseMove(QMouseEvent* event);
    bool filterRulerMouseRelease(QMouseEvent* event);

  private:
    void applyCameraChange(bool changed);
    bool updateScrollBars();
    void updateCameraFromScrollBars();

    CanvasCamera m_camera;
    RulerRenderer m_rulerRenderer;
    QRectF m_worldRect{0.0, 0.0, 20000.0, 20000.0};
    bool m_syncingScrollBars{false};
    bool m_rulerPointerGrab{false};
};

}// namespace xcanvas

#endif// CANVASVIEW_H
