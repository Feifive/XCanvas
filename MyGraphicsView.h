#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QSvgRenderer>
#include <QColor>

class BottomFloatingToolBar;
class SelectionHudBar;
class ColorPaletteWidget;

namespace xcanvas
{
class DrawingTool;
class Canvas;
class ToolManager;
class LayerManager;
}// namespace xcanvas

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
  public:
    explicit MyGraphicsView(QWidget* parent = nullptr);
    ~MyGraphicsView() override;

    double                 zoomValue();
    void                   requestFullUpdate();
    void                   updateSelectionHud();
    xcanvas::LayerManager* layerManager();

  signals:
    void mouseMovePos(QPointF pos);

  protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

  private slots:
    void onZoomIn();
    void onZoomOut();
    void onUndo();
    void onRedo();
    void onSelectionChanged();
    void onColorSelected(const QColor& color);

  private:
    void drawShapes(QPainter* painter, const QRectF& visibleRect);
    void drawNormalShapes(QPainter* painter, const QRectF& visibleRect);
    void drawSelectedShapes(QPainter* painter, const QRectF& visibleRect);
    void drawGrid(QPainter* painter);
    void drawTrace(QPainter* painter);
    void drawCanvas(QPainter* painter);

  private:
    double gridStep(double scale) const;
    void   importFile();
    void   importFiles(const QStringList& filePaths);
    void   importFiles(const QStringList& filePaths, const QPointF& targetCenter);
    void   updateBottomFloatingToolBarPos();
    void   updateSelectionHudBarPos();
    void   zoomIn(const QPointF& zoomCenterPoint);
    void   zoomOut(const QPointF& zoomCenterPoint);
    void   zoomTo(qreal zoomValue);
    void   fitWidth();
    void   fitHeight();
    void   fitCanvas();
    void   fitShapes();

  private:
    xcanvas::Canvas*       m_canvas;
    QPointF                m_startPos;
    bool                   m_bDragging;
    double                 m_dScaleFactor;
    BottomFloatingToolBar* m_bottomFloatingToolBar;
    SelectionHudBar*       m_selectionHudBar;
    ColorPaletteWidget*    m_colorPaletteWidget;
    std::unique_ptr<xcanvas::ToolManager> m_toolMgr;
    QSvgRenderer                          m_rotateHandle;
};

#endif// MYGRAPHICSVIEW_H
