#ifndef BOTTOMFLOATINGTOOLBAR_H
#define BOTTOMFLOATINGTOOLBAR_H

#include <QWidget>

class XHoverMenu;
class QTimer;
class EditorSession;
namespace qfw
{
class TransparentToolButton;
}

class BottomFloatingToolBar final : public QWidget
{
    Q_OBJECT
  public:
    explicit BottomFloatingToolBar(EditorSession* session, QWidget* parent = nullptr);
    ~BottomFloatingToolBar() override;
    void setCanUndo(bool canUndo) const;
    void setCanRedo(bool canRedo) const;

  signals:
    void zoomIn();
    void zoomOut();
    void zoomTo(qreal zoomValue);
    void fitWidth();
    void fitHeight();
    void fitCanvas();
    void fitShapes();
    void redo();
    void undo();

  private slots:
    void onZoomChanged(qreal zoomValue);

  private:
    void init();
    void initMenu();
    void applyStyle();

    qfw::TransparentToolButton* m_pUndo;
    qfw::TransparentToolButton* m_pRedo;
    qfw::TransparentToolButton* m_pZoomIn;
    qfw::TransparentToolButton* m_pZoomOut;
    qfw::TransparentToolButton* m_pZoomTool;
    XHoverMenu*  m_pZoomMenu;
    EditorSession* m_editorSession = nullptr;
};

#endif//BOTTOMFLOATINGTOOLBAR_H
