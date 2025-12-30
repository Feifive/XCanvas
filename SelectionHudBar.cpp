#include "SelectionHudBar.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPixmap>

SelectionHudBar::SelectionHudBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("SelectionHudBar");
    setAttribute(Qt::WA_StyledBackground, true);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 6, 10, 6);
    mainLayout->setSpacing(12);

    // =====================
    // Transform grid (X/Y/W/H/Angle)
    // =====================
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(5);
    grid->setVerticalSpacing(4);

    auto makeLabel = [](const QString &text) {
        auto *l = new QLabel(text);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->setObjectName("HudLabel");
        return l;
    };

    auto makeSpin = []() {
        auto *s = new QDoubleSpinBox;
        s->setFixedSize(50, 25);
        s->setDecimals(2);
        s->setRange(-1e9, 1e9);
        s->setAlignment(Qt::AlignLeft);
        s->setButtonSymbols(QAbstractSpinBox::NoButtons);
        s->setObjectName("HudSpin");
        return s;
    };

    m_spinX     = makeSpin();
    m_spinY     = makeSpin();
    m_spinW     = makeSpin();
    m_spinH     = makeSpin();
    m_spinAngle = makeSpin();
    m_spinAngle->setSuffix("°");
    m_spinAngle->setRange(-360.0, 360.0);

    grid->addWidget(makeLabel("X"), 0, 0);
    grid->addWidget(m_spinX,         0, 1);
    grid->addWidget(makeLabel("Y"),  0, 2);
    grid->addWidget(m_spinY,         0, 3);

    auto *angleSeparator = new QFrame;
    angleSeparator->setFixedHeight(40);
    angleSeparator->setFrameShape(QFrame::VLine);
    angleSeparator->setFrameShadow(QFrame::Plain);
    angleSeparator->setObjectName("HudSeparator");

    grid->addWidget(angleSeparator, 0, 4, 2, 1);

    auto *angleIcon = new QLabel;
    angleIcon->setObjectName("HudAngleIcon");
    angleIcon->setAlignment(Qt::AlignCenter);
    const QIcon icon(":/Resource/Icons/Angle.svg");
    const QPixmap anglePix = icon.pixmap(16, 16);
    angleIcon->setPixmap(anglePix);
    grid->addWidget(angleIcon, 0, 5);
    grid->addWidget(m_spinAngle,    0, 6);

    grid->addWidget(makeLabel("W"),  1, 0);
    grid->addWidget(m_spinW,         1, 1);
    grid->addWidget(makeLabel("H"),  1, 2);
    grid->addWidget(m_spinH,         1, 3);

    mainLayout->addLayout(grid);
    mainLayout->addStretch();

    applyStyleSheet();
}

SelectionHudBar::~SelectionHudBar() = default;

void SelectionHudBar::applyStyleSheet()
{
    setStyleSheet(R"(
        #SelectionHudBar {
            background-color: #FFFFFF;
            border-radius: 10px;
            border: 1px solid #E6E6E6;
        }

        QLabel#HudLabel {
            color: #848B95;
            font-size: 12px;
        }

        QDoubleSpinBox#HudSpin {
            background: transparent;
            border: none;
            color: #111111;
            font-size: 12px;
            border-radius: 4px;
        }

        QDoubleSpinBox#HudSpin:hover {
            background-color: #F0F0F5;
        }

        QDoubleSpinBox#HudSpin:focus {
            background-color: #E7E8F0;
        }

        QFrame#HudSeparator {
            color: #D9DCE4;
        }
    )");
}
