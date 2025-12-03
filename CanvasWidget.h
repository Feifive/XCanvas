#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include "Global.h"
#include <QWidget>

class QGraphicsScene;
class MyGraphicsView;
class RulerWidget;

class CanvasWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit CanvasWidget(QWidget* parent = nullptr);

  private:
    QGraphicsScene* m_pGraphicsScene;
    MyGraphicsView* m_pGraphicsView;
    RulerWidget*    m_pRulerHorizontal;
    RulerWidget*    m_pRulerVertical;
};

#endif// CANVASWIDGET_H
