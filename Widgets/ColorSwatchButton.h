#ifndef COLORSWATCHBUTTON_H
#define COLORSWATCHBUTTON_H

#include <QWidget>
#include <QColor>

class ColorSwatchButton final : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSwatchButton(const QColor& color, QWidget* parent = nullptr);

    QColor color() const { return m_color; }
    void   setSelected(bool sel);

signals:
    void clicked(const QColor& color);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    QColor m_color;
    bool   m_hovered  = false;
    bool   m_selected = false;
};

#endif //COLORSWATCHBUTTON_H
