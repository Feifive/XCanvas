#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QSvgRenderer>
#include <QColor>

class QString;
class BottomFloatingToolBar;
class SelectionHudBar;
class ColorPaletteWidget;
class SelectionHudController;
class ClipboardCommandService;
class ViewportTransformController;
class ViewRenderController;
class ViewInteractionController;
class ViewLayoutController;
class DocumentSessionController;
class AsyncFileTaskRunner;
class DocumentIoController;
class SelectionUiCoordinator;
class EditorSession;

namespace xcanvas
{
class DrawingTool;
class Canvas;
class ToolManager;
class LayerManager;
class Shape;
class ShapeText;
}// namespace xcanvas

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
  public:
    explicit MyGraphicsView(EditorSession* session, QWidget* parent = nullptr);
    ~MyGraphicsView() override;

    double                 zoomValue() const;
    void                   requestFullUpdate() const;
    void                   updateSelectionHud() const;
    xcanvas::LayerManager* layerManager() const;
    bool                   maybeSaveBeforeClose() const;
    QString                currentDocumentPath() const;
    bool                   openDocumentFile(const QString& path) const;

  signals:
    void mouseMovePos(QPointF pos);
    void documentDisplayNameChanged(const QString& name) const;

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
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
    void onColorSelected(const QColor& color) const;

  private:
    void initView();
    void initCore();
    void initWidgets();
    void initControllers();
    void initConnections();
    void initStartup();
    void applyStyle();

    void   importFile();
    void   resetToNewDocument() const;
    bool   maybeSaveBeforeProceed() const;
    bool   isProjectFilePath(const QString& path) const;
    bool   saveDocumentFileBlocking(const QString& path) const;
    void   openDocumentFileAsync(const QString& path) const;
    void   saveDocumentFileAsync(const QString& path, bool updateCurrentPath) const;
    void   importFiles(const QStringList& filePaths) const;
    void   importFiles(const QStringList& filePaths, const QPointF& targetCenter) const;
    void   updateBottomFloatingToolBarPos() const;
    void   updateSelectionHudBarPos() const;
    void   zoomIn(const QPointF& zoomCenterPoint) const;
    void   zoomOut(const QPointF& zoomCenterPoint) const;
    void   zoomTo(qreal zoomValue) const;
    void   fitWidth() const;
    void   fitHeight() const;
    void   fitCanvas() const;
    void   fitShapes() const;
    void   updateWindowTitle() const;
    QString projectDisplayName() const;
    xcanvas::Shape* findTopShapeAtScenePos(const QPointF& scenePos) const;
    void            beginInlineTextEdit(xcanvas::ShapeText* shape);
    void            finishInlineTextEdit(bool commit);

  private:
    xcanvas::Canvas*       m_canvas;
    BottomFloatingToolBar* m_bottomFloatingToolBar;
    SelectionHudBar*       m_selectionHudBar;
    ColorPaletteWidget*    m_colorPaletteWidget;
    std::unique_ptr<SelectionHudController> m_selectionHudController;
    std::unique_ptr<SelectionUiCoordinator> m_selectionUiCoordinator;
    std::unique_ptr<ViewLayoutController> m_viewLayoutController;
    std::unique_ptr<ViewportTransformController> m_viewportTransformController;
    std::unique_ptr<ViewRenderController> m_viewRenderController;
    std::unique_ptr<xcanvas::ToolManager> m_toolMgr;
    QSvgRenderer                          m_rotateHandle;
    std::unique_ptr<AsyncFileTaskRunner>  m_fileTaskRunner;
    bool                                  m_isDestroying;
    std::unique_ptr<ClipboardCommandService> m_clipboardCommandService;
    std::unique_ptr<DocumentSessionController> m_documentSessionController;
    std::unique_ptr<ViewInteractionController> m_viewInteractionController;
    std::unique_ptr<DocumentIoController> m_documentIoController;
    class QGraphicsTextItem*                 m_inlineTextEditor;
    xcanvas::ShapeText*                      m_inlineEditingShape;
    QString                                  m_inlineOriginalText;
    EditorSession*                           m_editorSession;
};

#endif// MYGRAPHICSVIEW_H
