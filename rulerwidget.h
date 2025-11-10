#ifndef RULERWIDGET_H
#define RULERWIDGET_H

#include <QWidget>

class QGraphicsView;

class RulerWidget : public QWidget
{
    Q_OBJECT

public:
    RulerWidget(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~RulerWidget()override;
    void AttachView(QGraphicsView* pView) { m_pAttachView = pView; }
    void setOffset(int value)
    {
        offset = value;
        update();
    }
    void setSlidingLinePos(int pos)
    {
        slidingLinePos = pos + offset;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event)override;

private:
    QColor backgroundColor{Qt::white};//背景色
    QColor textAndLineColor{"#606060"};//文本和刻度颜色
    QColor slidingLineColor{"#D56161"};//游标颜色
    Qt::Orientations orientation;
    int slidingLinePos{0};
    int offset{0};
    QFont font{"PingFang SC",16};
    QGraphicsView* m_pAttachView;
};

#endif // RULERWIDGET_H
