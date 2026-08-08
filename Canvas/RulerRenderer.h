#ifndef RULERRENDERER_H
#define RULERRENDERER_H

#include <QColor>
#include <QFont>
#include <QPalette>
#include <QRect>
#include <QSizeF>
#include <QString>

class QPainter;

namespace xcanvas
{

class CanvasCamera;

class RulerRenderer final
{
  public:
    enum class HitArea
    {
        None,
        Horizontal,
        Vertical,
        Corner
    };

    explicit RulerRenderer(int thickness = 24);

    int thickness() const;
    void setThickness(int thickness);

    QColor backgroundColor(const QColor& baseColor) const;
    QRect horizontalRect(const QRect& viewportRect) const;
    QRect verticalRect(const QRect& viewportRect) const;
    QRect cornerRect(const QRect& viewportRect) const;
    HitArea hitTest(const QPoint& viewPoint, const QRect& viewportRect) const;
    static QString labelText(qreal worldCoordinate, qreal originCoordinate);

    void draw(QPainter* painter,
              const QRect& viewportRect,
              const QSizeF& viewportSize,
              const CanvasCamera& camera,
              const QPointF& worldOrigin,
              const QColor& baseColor,
              const QPalette& palette) const;

  private:
    qreal tickStep(qreal scale) const;
    void drawHorizontal(QPainter* painter,
                        const QRect& rect,
                        const QSizeF& viewportSize,
                        const CanvasCamera& camera,
                        qreal originX) const;
    void drawVertical(QPainter* painter,
                      const QRect& rect,
                      const QSizeF& viewportSize,
                      const CanvasCamera& camera,
                      qreal originY) const;

    int m_thickness;
    QFont m_font;
};

}// namespace xcanvas

#endif// RULERRENDERER_H
