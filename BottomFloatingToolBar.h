#ifndef BOTTOMFLOATINGTOOLBAR_H
#define BOTTOMFLOATINGTOOLBAR_H

#include <QWidget>

class QToolButton;
class QMenu;
class QTimer;

class BottomFloatingToolBar: public QWidget
{
    Q_OBJECT
public:
    explicit BottomFloatingToolBar(QWidget* parent = nullptr);
    ~BottomFloatingToolBar();
	void setCanUndo(bool canUndo);
	void setCanRedo(bool canRedo);

signals:
    void zoomIn();
    void zoomOut();
    void zoomTo(const qreal zoomValue);
    void fitWidth();
    void fitHeight();
    void fitCanvas();
    void fitShapes();
    void redo();
    void undo();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
	void onZoomChanged(const qreal zoomValue);
	void onCloseTimerTimeout();

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
    QTimer*      m_pCloseTimer;
};



#endif //BOTTOMFLOATINGTOOLBAR_H
