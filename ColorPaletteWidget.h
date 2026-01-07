#ifndef COLORPALETTEWIDGET_H
#define COLORPALETTEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>

class ColorSwatchButton;

class ColorPaletteWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPaletteWidget(QWidget* parent = nullptr);

    void setColors(const QVector<QColor>& colors);
    void setCurrentColor(const QColor& color);

private:
    QVector<ColorSwatchButton*> m_buttons;
};

#endif //COLORPALETTEWIDGET_H
