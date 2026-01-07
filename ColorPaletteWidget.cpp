#include "ColorPaletteWidget.h"
#include "ColorSwatchButton.h"
#include "AppSettings.h"
#include <QHBoxLayout>

ColorPaletteWidget::ColorPaletteWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("ColorPaletteWidget");
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(3);
    layout->setContentsMargins(4, 4, 4, 4);

    QVector<QColor> layerColors = {
        QColor("#000000"), // Outline
        QColor("#C62828"), // Cut Outer
        QColor("#EF6C00"), // Cut Inner
        QColor("#1565C0"), // Engrave Deep
        QColor("#3949AB"), // Engrave Light
        QColor("#6A1B9A"), // Fill
        QColor("#2E7D32"), // Mark
        QColor("#006064"), // Assist
        QColor("#616161")  // Construct
    };

    setColors(layerColors);
    setCurrentColor(layerColors[0]);
}

void ColorPaletteWidget::setColors(const QVector<QColor>& colors)
{
    qDeleteAll(m_buttons);
    m_buttons.clear();

    auto* layout = qobject_cast<QHBoxLayout*>(this->layout());

    for (const QColor& color : colors)
    {
        auto* btn = new ColorSwatchButton(color, this);
        m_buttons.push_back(btn);

        layout->addWidget(btn);

        connect(btn, &ColorSwatchButton::clicked, this,
                [this](const QColor& activeColor)
        {
            setCurrentColor(activeColor);
            AppSettings::instance().setActiveColor(activeColor);
        });
    }
}

void ColorPaletteWidget::setCurrentColor(const QColor& color)
{
    for (auto* btn : m_buttons)
    {
        btn->setSelected(btn->color() == color);
    }
}