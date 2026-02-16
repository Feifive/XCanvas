#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QSvgRenderer>
#include <QColor>
#include <QVector>

class BottomFloatingToolBar;
class SelectionHudBar;
class ColorPaletteWidget;
class QThread;
class QObject;
class MessageWidget;

namespace xcanvas
{
class DrawingTool;
class Canvas;
class ToolManager;
class LayerManager;
class Shape;
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
    bool                   maybeSaveBeforeClose();

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
    void onNewDocument();
    void onOpenDocument();
    void onSaveDocument();
    void onSaveDocumentAs();
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
    void   resetToNewDocument();
    bool   maybeSaveBeforeProceed();
    bool   isProjectFilePath(const QString& path) const;
    bool   openDocumentFile(const QString& path);
    bool   saveDocumentFile(const QString& path);
    bool   saveDocumentFileBlocking(const QString& path);
    void   openDocumentFileAsync(const QString& path);
    void   saveDocumentFileAsync(const QString& path, bool updateCurrentPath);
    void   setFileActionsEnabled(bool enabled);
    void   showFileTaskLoading(const QString& text);
    void   closeFileTaskLoading();
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
    void   showCanvasContextMenu(const QPoint& viewPos);
    bool   copySelectedShapes();
    bool   cutSelectedShapes();
    bool   pasteCopiedShapes();
    bool   pasteCopiedShapesAt(const QPointF& scenePos);
    bool   deleteSelectedShapes();
    bool   groupSelectedShapes();
    bool   ungroupSelectedShapes();
    bool   pasteFromClipboard(const QPointF& scenePos);
    bool   hasClipboardPasteContent() const;
    void   clearCopiedShapes();
    void   updateWindowTitle();
    void   applySelectionHudX(double newX);
    void   applySelectionHudY(double newY);
    void   applySelectionHudW(double newW);
    void   applySelectionHudH(double newH);
    void   applySelectionHudAngle(double newAngle);
    void   onKeepAspectRatioToggled(bool enabled);
    QString projectDisplayName() const;

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
    QVector<xcanvas::Shape*>              m_copiedShapes;
    QPoint                                m_rightPressPos;
    bool                                  m_rightDragged;
    int                                   m_pasteSerial;
    QString                               m_currentDocumentPath;
    QThread*                              m_fileIoThread;
    QObject*                              m_fileIoContext;
    bool                                  m_fileTaskRunning;
    MessageWidget*                        m_fileTaskMessage;
    bool                                  m_keepAspectRatio;
    bool                                  m_isDestroying;
};

#endif// MYGRAPHICSVIEW_H
