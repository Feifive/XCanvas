#include "SelectionHudBar.h"
#include "HoverMenuHelper.h"
#include "SafeDropShadowEffect.h"
#include <QDoubleSpinBox>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPixmap>
#include <QStyleFactory>
#include <QToolButton>

SelectionHudBar::SelectionHudBar(QWidget* parent) : QWidget(parent)
{
    setObjectName("SelectionHudBar");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 6, 10, 6);
    mainLayout->setSpacing(12);

    // =====================
    // Transform grid (X/Y/W/H/Angle)
    // =====================
    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(5);
    grid->setVerticalSpacing(4);

    auto makeLabel = [](const QString& text)
    {
        auto* l = new QLabel(text);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->setObjectName("HudLabel");
        return l;
    };

    auto makeSpin = []()
    {
        auto* s = new QDoubleSpinBox;
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
    grid->addWidget(m_spinX, 0, 1);
    grid->addWidget(makeLabel("Y"), 0, 2);
    grid->addWidget(m_spinY, 0, 3);

    auto* separator1 = new QFrame;
    separator1->setFixedHeight(40);
    separator1->setFrameShape(QFrame::VLine);
    separator1->setFrameShadow(QFrame::Plain);
    separator1->setObjectName("HudSeparator");
    grid->addWidget(separator1, 0, 4, 2, 1);

    auto* angleIcon = new QLabel;
    angleIcon->setObjectName("HudAngleIcon");
    angleIcon->setAlignment(Qt::AlignCenter);
    const QIcon   icon(":/Resource/Icons/Angle.svg");
    const QPixmap anglePix = icon.pixmap(14, 14);
    angleIcon->setPixmap(anglePix);
    grid->addWidget(angleIcon, 0, 5);
    grid->addWidget(m_spinAngle, 0, 6);

    grid->addWidget(makeLabel("W"), 1, 0);
    grid->addWidget(m_spinW, 1, 1);
    grid->addWidget(makeLabel("H"), 1, 2);
    grid->addWidget(m_spinH, 1, 3);

    auto* separator2 = new QFrame;
    separator2->setFixedHeight(40);
    separator2->setFrameShape(QFrame::VLine);
    separator2->setFrameShadow(QFrame::Plain);
    separator2->setObjectName("HudSeparator");
    grid->addWidget(separator2, 0, 7, 2, 1);

    auto* btnMerge = new QToolButton(parent);
    btnMerge->setIcon(QIcon(":/Resource/Icons/Angle.svg"));
    btnMerge->setFixedSize(40, 40);
    btnMerge->setText(tr("合并"));
    btnMerge->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnMerge->setArrowType(Qt::NoArrow);

    auto* menu = new QMenu(this);
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground, true);
    auto* shadow = new SafeDropShadowEffect(this);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setBlurRadius(20);
    menu->setGraphicsEffect(shadow);
    auto* hoverHelper = new HoverMenuHelper(btnMerge, menu);
    menu->addAction(QIcon(":/Resource/Icons/Angle.svg"), tr("合并图形"));
    menu->addAction(QIcon(":/Resource/Icons/Angle.svg"), tr("排除顶层"));
    menu->addAction(QIcon(":/Resource/Icons/Angle.svg"), tr("图形相交"));
    menu->addAction(QIcon(":/Resource/Icons/Angle.svg"), tr("排除相交"));
    grid->addWidget(btnMerge, 0, 8, 2, 1);

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

        QToolButton {
            border: none;
            border-radius: 5px;
            font-size: 10px;
        }

        QToolButton::menu-indicator {
            width: 0px;
        }

        QToolButton:hover {
            background: #F6F6F9;
        }

        QMenu {
            border-radius: 5px;
            background-color: white;
            padding: 4px;
            margin: 20px;
        }

        QMenu::item {
            border-radius: 5px;
            border: 0px solid transparent;
            background-color: transparent;
            color: black;
            min-height: 30px;
            font-size: 12px;
            padding-left: 12px;
        }

        QMenu::icon {
            left: 8px;
        }

        QMenu::item:selected {
            margin: 2px 0px;
            background-color: #F3F3F5;
        }
    )");
}
