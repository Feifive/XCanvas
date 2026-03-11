#include "SelectionHudBar.h"
#include "XCanvasIcon.h"
#include "XHoverMenu.h"
#include <QAction>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <qtfluentwidgets.h>

SelectionHudBar::SelectionHudBar(QWidget* parent) : QWidget(parent), m_booleanTool(nullptr)
{
    setObjectName("SelectionHudBar");
    setAttribute(Qt::WA_StyledBackground, true);
    applyStyle();

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 6, 10, 6);
    mainLayout->setSpacing(12);

    m_gridLayout = new QGridLayout;
    m_gridLayout->setHorizontalSpacing(5);
    m_gridLayout->setVerticalSpacing(4);

    auto makeLabel = [](const QString& text)
    {
        auto* l = new qfw::BodyLabel(text);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->setObjectName("HudLabel");
        return l;
    };

    auto makeSpin = []()
    {
        auto* s = new qfw::DoubleSpinBox;
        s->setFixedSize(60, 25);
        s->setDecimals(2);
        s->setRange(-1e9, 1e9);
        s->setAlignment(Qt::AlignLeft);
        s->setButtonSymbols(QAbstractSpinBox::NoButtons);
        s->setSymbolVisible(false);
        s->setObjectName("HudSpin");
        return s;
    };

    m_spinX     = makeSpin();
    m_spinY     = makeSpin();
    m_spinW     = makeSpin();
    m_spinH     = makeSpin();
    m_spinAngle = makeSpin();
    m_spinW->setRange(0.01, 1e9);
    m_spinH->setRange(0.01, 1e9);
    m_spinAngle->setSuffix("°");
    m_spinAngle->setRange(0.0, 360.0);

    connect(m_spinX, &qfw::DoubleSpinBox::editingFinished, this, [this]() { emit xEdited(m_spinX->value()); });
    connect(m_spinY, &qfw::DoubleSpinBox::editingFinished, this, [this]() { emit yEdited(m_spinY->value()); });
    connect(m_spinW, &qfw::DoubleSpinBox::editingFinished, this, [this]() { emit wEdited(m_spinW->value()); });
    connect(m_spinH, &qfw::DoubleSpinBox::editingFinished, this, [this]() { emit hEdited(m_spinH->value()); });
    connect(m_spinAngle, &qfw::DoubleSpinBox::editingFinished, this, [this]() { emit angleEdited(m_spinAngle->value()); });

    m_keepAspectRatioButton = new qfw::TransparentToolButton(this);
    m_keepAspectRatioButton->setObjectName("HudKeepAspectButton");
    m_keepAspectRatioButton->setFixedSize(25, 25);
    m_keepAspectRatioButton->setCheckable(true);
    m_keepAspectRatioButton->setChecked(true);
    m_keepAspectRatioButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_keepAspectRatioButton->setIcon(XCanvasIcon(XCanvasIconType::KeepAspectRatio));
    m_keepAspectRatioButton->setToolTip(tr("约束宽高比例"));
    connect(m_keepAspectRatioButton, &qfw::TransparentToolButton::toggled, this, [this](bool checked) {
        emit keepAspectRatioToggled(checked);
        m_keepAspectRatioButton->setIcon(checked ? XCanvasIcon(XCanvasIconType::KeepAspectRatio)
                                                 : XCanvasIcon(XCanvasIconType::AspectRatioClose));
    });

    m_gridLayout->addWidget(makeLabel("X"), 0, 0);
    m_gridLayout->addWidget(m_spinX, 0, 1);
    m_gridLayout->addWidget(makeLabel("Y"), 0, 3);
    m_gridLayout->addWidget(m_spinY, 0, 4);

    auto* angleIcon = new qfw::TransparentToolButton(this);
    angleIcon->setObjectName("HudAngleIcon");
    angleIcon->setFixedSize(15, 15);
    angleIcon->setToolButtonStyle(Qt::ToolButtonIconOnly);
    angleIcon->setIcon(XCanvasIcon(XCanvasIconType::Angle));
    angleIcon->setFocusPolicy(Qt::NoFocus);
    angleIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_gridLayout->addWidget(angleIcon, 0, 6);
    m_gridLayout->addWidget(m_spinAngle, 0, 7);

    auto* separator1 = new QFrame;
    separator1->setFixedHeight(40);
    separator1->setFrameShape(QFrame::VLine);
    separator1->setFrameShadow(QFrame::Plain);
    separator1->setObjectName("HudSeparator");
    m_gridLayout->addWidget(separator1, 0, 5, 2, 1);

    m_gridLayout->addWidget(makeLabel("W"), 1, 0);
    m_gridLayout->addWidget(m_spinW, 1, 1);
    m_gridLayout->addWidget(m_keepAspectRatioButton, 1, 2, 1, 1, Qt::AlignCenter);
    m_gridLayout->addWidget(makeLabel("H"), 1, 3);
    m_gridLayout->addWidget(m_spinH, 1, 4);

    auto* separator2 = new QFrame;
    separator2->setFixedHeight(40);
    separator2->setFrameShape(QFrame::VLine);
    separator2->setFrameShadow(QFrame::Plain);
    separator2->setObjectName("HudSeparator");
    m_gridLayout->addWidget(separator2, 0, 8, 2, 1);

    auto* alignTool = new qfw::TransparentToolButton(this);
    alignTool->setIcon(XCanvasIcon(XCanvasIconType::AlignLeft));
    alignTool->setFixedSize(40, 40);
    alignTool->setToolButtonStyle(Qt::ToolButtonIconOnly);
    alignTool->setArrowType(Qt::NoArrow);
    m_gridLayout->addWidget(alignTool, 0,  9, 2, 1);
    auto* alignMenu = new XHoverMenu(alignTool, XHoverMenu::PopupOrientation::Bottom, this);
    alignMenu->setObjectName("menu_iconMenu");
    alignMenu->setGap(15);
    auto addMenuAction = [this](XHoverMenu* menu,
                                const XCanvasIconType iconType,
                                const QString& text,
                                void (SelectionHudBar::*slot)())
    {
        auto* action = new qfw::Action(XCanvasIcon(iconType), text, menu);
        menu->addAction(action);
        connect(action, &QAction::triggered, this, slot);
    };
    addMenuAction(alignMenu, XCanvasIconType::AlignLeft, tr("左对齐"), &SelectionHudBar::alignLeft);
    addMenuAction(alignMenu, XCanvasIconType::HorizontalAlignCenter, tr("水平居中"), &SelectionHudBar::alignHorizontalCenter);
    addMenuAction(alignMenu, XCanvasIconType::AlignRight, tr("右对齐"), &SelectionHudBar::alignRight);
    addMenuAction(alignMenu, XCanvasIconType::AlignTop, tr("顶部对齐"), &SelectionHudBar::alignTop);
    addMenuAction(alignMenu, XCanvasIconType::VerticalAlignCenter, tr("垂直居中"), &SelectionHudBar::alignVerticalCenter);
    addMenuAction(alignMenu, XCanvasIconType::AlignBottom, tr("底部对齐"), &SelectionHudBar::alignBottom);
    addMenuAction(alignMenu, XCanvasIconType::AlignCenter, tr("中心对齐"), &SelectionHudBar::alignCenter);

    auto* mirrorTool = new qfw::TransparentToolButton(this);
    mirrorTool->setIcon(XCanvasIcon(XCanvasIconType::MirrorHorizontally));
    mirrorTool->setFixedSize(40, 40);
    mirrorTool->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mirrorTool->setArrowType(Qt::NoArrow);
    m_gridLayout->addWidget(mirrorTool, 0,  10, 2, 1);
    auto* mirrorMenu = new XHoverMenu(mirrorTool, XHoverMenu::PopupOrientation::Bottom, this);
    mirrorMenu->setObjectName("menu_iconMenu");
    mirrorMenu->setGap(15);
    addMenuAction(mirrorMenu, XCanvasIconType::MirrorHorizontally, tr("水平镜像"), &SelectionHudBar::mirrorHorizontal);
    addMenuAction(mirrorMenu, XCanvasIconType::MirrorVertically, tr("垂直镜像"), &SelectionHudBar::mirrorVertical);

    m_booleanTool = new qfw::TransparentToolButton(this);
    m_booleanTool->setIcon(XCanvasIcon(XCanvasIconType::Union));
    m_booleanTool->setFixedSize(40, 40);
    m_booleanTool->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_booleanTool->setArrowType(Qt::NoArrow);
    m_gridLayout->addWidget(m_booleanTool, 0, 11, 2, 1);
    auto* mergeMenu = new XHoverMenu(m_booleanTool, XHoverMenu::PopupOrientation::Bottom, this);
    mergeMenu->setObjectName("menu_iconMenu");
    mergeMenu->setGap(15);
    addMenuAction(mergeMenu, XCanvasIconType::Union, tr("图形合并"), &SelectionHudBar::booleanUnion);
    addMenuAction(mergeMenu, XCanvasIconType::Intersection, tr("图形相交"), &SelectionHudBar::booleanIntersection);
    addMenuAction(mergeMenu, XCanvasIconType::SubtractA, tr("图形相减"), &SelectionHudBar::booleanSubtractAB);
    addMenuAction(mergeMenu, XCanvasIconType::SubtractB, tr("图形相减"), &SelectionHudBar::booleanSubtractBA);

    mainLayout->addLayout(m_gridLayout);
    mainLayout->addStretch();

    connect(&qfw::QConfig::instance(), &qfw::QConfig::themeChanged, this,
            [this](qfw::Theme)
            {
                applyStyle();
                update();
            });
}

SelectionHudBar::~SelectionHudBar() = default;

bool SelectionHudBar::keepAspectRatio() const
{
    return m_keepAspectRatioButton && m_keepAspectRatioButton->isChecked();
}

void SelectionHudBar::setSummary(const SelectionSummary &summary) {
    hideGridColumn(11, !(summary.onlyVector() && summary.vectorCount == 2));
    adjustSize();
}

void SelectionHudBar::hideGridColumn(int col, bool hide) {
    if (!m_gridLayout) {
        return;
    }

    for (int row = 0; row < m_gridLayout->rowCount(); ++row)
    {
        const QLayoutItem* item = m_gridLayout->itemAtPosition(row, col);
        if (!item) {
            continue;
        }

        if (QWidget* widget = item->widget())
        {
            widget->setVisible(!hide);
        }
    }

    if (hide)
    {
        m_gridLayout->setColumnMinimumWidth(col, 0);
        m_gridLayout->setColumnStretch(col, 0);
    }
}

void SelectionHudBar::applyStyle() {
    const bool dark = qfw::isDarkTheme();
    setStyleSheet(QStringLiteral(R"(
        QWidget#SelectionHudBar QDoubleSpinBox {
            font-size: 12px;
        }

        QWidget#SelectionHudBar {
            background-color: %1;
            border-radius: 10px;
            border: 1px solid %2;
        }

        #HudLabel {
            color: %3;
            font-size: 12px;
        }

        QFrame#HudSeparator {
            color: %2;
        }
    )").arg(dark ? QStringLiteral("#232830")
                 : QStringLiteral("#FFFFFF"),
            dark ? QStringLiteral("#3A404B")
                 : QStringLiteral("#E6E6E6"),
            dark ? QStringLiteral("#8F97A3")
                 : QStringLiteral("#848B95")));
}
