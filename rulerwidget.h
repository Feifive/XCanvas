#ifndef RULERWIDGET_H
#define RULERWIDGET_H

#include <QWidget>

class MyGraphicsView;

class RulerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RulerWidget(Qt::Orientation orientation, QWidget* parent = nullptr);
    void attachView(MyGraphicsView* view);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void   drawHorizontal(QPainter& painter);
    void   drawVertical(QPainter& painter);
    double tickStep(double scale) const;

private:
    MyGraphicsView*  m_view = nullptr;
    Qt::Orientation m_orientation;

    QColor m_backgroundColor { "#E7E9ED" };   // 背景色
    QColor m_textAndLineColor { "#606060" };        // 文本和刻度颜色
    QFont  m_font;
};

#endif // RULERWIDGET_H
