#ifndef MYCANVASVIEW_H
#define MYCANVASVIEW_H

#include "Canvas/CanvasView.h"
#include <QSvgRenderer>
#include <QColor>

class QString;
class QHideEvent;
class QShowEvent;
class QTimer;
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
class TextEditController;
class ShapeInteractionController;
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

class MyCanvasView : public xcanvas::CanvasView
{
    Q_OBJECT
  public:
    explicit MyCanvasView(EditorSession* session, QWidget* parent = nullptr);
    ~MyCanvasView() override;

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
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void inputMethodEvent(QInputMethodEvent* event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void wheelEvent(QWheelEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;
    void drawWorld(QPainter& painter, const QRectF& visibleWorldRect) override;
    QColor rulerBaseColor() const override;
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
    void scheduleInitialFit();
    void syncSelectionOutlineAnimation();

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

  private:
    xcanvas::Canvas*       m_canvas{nullptr};
    BottomFloatingToolBar* m_bottomFloatingToolBar{nullptr};
    SelectionHudBar*       m_selectionHudBar{nullptr};
    ColorPaletteWidget*    m_colorPaletteWidget{nullptr};
    std::unique_ptr<SelectionHudController> m_selectionHudController;
    std::unique_ptr<SelectionUiCoordinator> m_selectionUiCoordinator;
    std::unique_ptr<ViewLayoutController> m_viewLayoutController;
    std::unique_ptr<ViewportTransformController> m_viewportTransformController;
    std::unique_ptr<ViewRenderController> m_viewRenderController;
    std::unique_ptr<xcanvas::ToolManager> m_toolMgr;
    QSvgRenderer                          m_rotateHandle;
    std::unique_ptr<AsyncFileTaskRunner>  m_fileTaskRunner;
    bool                                  m_isDestroying{false};
    QTimer*                               m_initialFitTimer{nullptr};
    bool                                  m_initialFitPending{true};
    QTimer*                               m_selectionOutlineTimer{nullptr};
    qreal                                 m_selectionDashPhase{0.0};
    std::unique_ptr<ClipboardCommandService> m_clipboardCommandService;
    std::unique_ptr<DocumentSessionController> m_documentSessionController;
    std::unique_ptr<ViewInteractionController> m_viewInteractionController;
    std::unique_ptr<DocumentIoController> m_documentIoController;
    std::unique_ptr<TextEditController> m_textEditController;
    std::unique_ptr<ShapeInteractionController> m_shapeInteractionController;
    EditorSession*                           m_editorSession{nullptr};
};

#endif// MYCANVASVIEW_H
