#include "ColorPaletteWidget.h"
#include "ColorSwatchButton.h"
#include "AppSettings.h"
#include <qtfluentwidgets.h>
#include <QHBoxLayout>

ColorPaletteWidget::ColorPaletteWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("ColorPaletteWidget");
    applyStyle();
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(3);
    layout->setContentsMargins(4, 4, 4, 4);

    QVector<QColor> layerColors = {
        QColor("#000000"),
        QColor("#C62828"),
        QColor("#EF6C00"),
        QColor("#1565C0"),
        QColor("#3949AB"),
        QColor("#6A1B9A"),
        QColor("#2E7D32"),
        QColor("#006064"),
        QColor("#616161")
    };

    setColors(layerColors);
    setCurrentColor(layerColors[0]);

    connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
            [this](qfw::Theme)
            {
                applyStyle();
                for (auto* btn : m_buttons)
                {
                    if (btn)
                    {
                        btn->update();
                    }
                }
            });
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
            emit colorSelected(activeColor);
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

void ColorPaletteWidget::applyStyle() {
    const bool dark = qfw::isDarkTheme();
    setStyleSheet(QStringLiteral(R"(
        QWidget#ColorPaletteWidget {
            background-color: %1;
            border-radius: 4px;
            border: 1px solid %2;
        }
    )").arg(dark ? QStringLiteral("#232830")
                 : QStringLiteral("#FFFFFF"),
            dark ? QStringLiteral("#3A404B")
                 : QStringLiteral("#E7E9ED")));
}
