#include "CanvasRenderer.h"
#include "ShapeManager.h"
#include "Canvas.h"
#include "Shape.h"
#include <QPainter>

namespace xcanvas {
    void CanvasRenderer::drawBackground(QPainter* painter, const Canvas* canvas, const QRectF& rect) {
        if (!canvas) {
            return;
        }
        drawCanvas(painter, canvas);
        drawGrid(painter, canvas);
        drawNormalShapes(painter, canvas, rect);
    }

    void CanvasRenderer::drawForeground(QPainter* painter, const Canvas* canvas, const QRectF& rect) {
        drawSelectedShapes(painter, canvas, rect);
    }

    void CanvasRenderer::drawNormalShapes(QPainter *painter, const Canvas *canvas, const QRectF &visibleRect) {
        painter->save();

        // 遍历所有形状，只绘制未选中的
        for (xcanvas::Shape* shape : canvas->shapeManager()->shapes())
        {
            if (shape->isSelected())
            {
                continue;
            }

            // 视口裁剪：只绘制可见的形状
            if (!visibleRect.intersects(shape->boundingRect()))
            {
                continue;
            }

            // 绘制形状
            shape->draw(painter);
        }

        painter->restore();
    }

    void CanvasRenderer::drawSelectedShapes(QPainter *painter, const Canvas *canvas, const QRectF &visibleRect) {
        painter->save();

        // 最后绘制选中的形状（显示在最上层）
        QVector<xcanvas::Shape*> selected = canvas->shapeManager()->selectedShapes();

        for (xcanvas::Shape* shape : selected)
        {
            if (!visibleRect.intersects(shape->boundingRect()))
            {
                continue;
            }

            shape->draw(painter);
        }

        // 绘制trace
        if (!selected.isEmpty())
        {
            drawTrace(painter);
        }

        painter->restore();
    }

    void CanvasRenderer::drawGrid(QPainter *painter, const Canvas *canvas) {
    }

    void CanvasRenderer::drawTrace(QPainter *painter, const Canvas *canvas) {
        if (!canvas && !canvas->shapeManager() || canvas->shapeManager()->isEmpty())
        {
            return;
        }

        QRectF rect = m_canvas->shapeManager()->selectedBoundingRect();
        if (!rect.isValid())
        {
            return;
        }

        double dScale      = zoomValue();
        double dLineLength = 6 / dScale;

        painter->save();

        QPen pen(Qt::black);
        pen.setWidthF(2 / dScale);
        pen.setCosmetic(false);
        painter->setPen(pen);
        painter->setBrush(Qt::black);

        QRectF rects[9];
        traceRects(rect, rects);

        for (int i = 0; i < 9; ++i)
        {
            if (i == ERECT_CENTER)
            {
                continue;
            }
            painter->drawRect(rects[i]);
        }

        QPointF center = rect.center();

        painter->drawLine(QPointF(center.x() - dLineLength, center.y() - dLineLength), QPointF(center.x() + dLineLength, center.y() + dLineLength));

        painter->drawLine(QPointF(center.x() + dLineLength, center.y() - dLineLength), QPointF(center.x() - dLineLength, center.y() + dLineLength));

        painter->restore();
    }

    void CanvasRenderer::drawCanvas(QPainter *painter, const Canvas *canvas) {
        painter->save();

        const QTransform oldWorldTransform = painter->worldTransform();
        painter->setWorldTransform(QTransform());
        painter->fillRect(rect(), QColor("#E7E9ED"));
        painter->setWorldTransform(oldWorldTransform);

        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawRect(m_canvas->canvasRect());

        painter->restore();
    }
} // xcanvas