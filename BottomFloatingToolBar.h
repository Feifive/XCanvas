#ifndef BOTTOMFLOATINGTOOLBAR_H
#define BOTTOMFLOATINGTOOLBAR_H

#include <QWidget>

class QToolButton;
class QMenu;
class QTimer;

class BottomFloatingToolBar final : public QWidget
{
    Q_OBJECT
  public:
    explicit BottomFloatingToolBar(QWidget* parent = nullptr);
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
    void styleSheet();

    QToolButton* m_pUndo;
    QToolButton* m_pRedo;
    QToolButton* m_pZoomIn;
    QToolButton* m_pZoomOut;
    QToolButton* m_pZoomTool;
    QMenu*       m_pZoomMenu;
};

#endif//BOTTOMFLOATINGTOOLBAR_H
