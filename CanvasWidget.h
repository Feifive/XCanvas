#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>

class QGraphicsScene;
class MyGraphicsView;
class RulerWidget;
namespace xcanvas
{
class LayerManager;
}

class CanvasWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    xcanvas::LayerManager* layerManager();

  private:
    QGraphicsScene* m_pGraphicsScene;
    MyGraphicsView* m_pGraphicsView;
    RulerWidget*    m_pRulerHorizontal;
    RulerWidget*    m_pRulerVertical;
};

#endif// CANVASWIDGET_H
