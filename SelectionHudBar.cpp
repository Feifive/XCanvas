#include "SelectionHudBar.h"
#include "SafeDropShadowEffect.h"
#include "XHoverMenu.h"
#include <QDoubleSpinBox>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPixmap>
#include <QToolButton>

SelectionHudBar::SelectionHudBar(QWidget* parent) : QWidget(parent), m_booleanTool(nullptr)
{
    setObjectName("SelectionHudBar");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 6, 10, 6);
    mainLayout->setSpacing(12);

    m_gridLayout = new QGridLayout;
    m_gridLayout->setHorizontalSpacing(5);
    m_gridLayout->setVerticalSpacing(4);

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
    m_spinW->setRange(0.01, 1e9);
    m_spinH->setRange(0.01, 1e9);
    m_spinAngle->setSuffix("°");
    m_spinAngle->setRange(-360.0, 360.0);

    connect(m_spinX, &QDoubleSpinBox::editingFinished, this, [this]() { emit xEdited(m_spinX->value()); });
    connect(m_spinY, &QDoubleSpinBox::editingFinished, this, [this]() { emit yEdited(m_spinY->value()); });
    connect(m_spinW, &QDoubleSpinBox::editingFinished, this, [this]() { emit wEdited(m_spinW->value()); });
    connect(m_spinH, &QDoubleSpinBox::editingFinished, this, [this]() { emit hEdited(m_spinH->value()); });

    m_keepAspectRatioButton = new QToolButton;
    m_keepAspectRatioButton->setObjectName("HudKeepAspectButton");
    m_keepAspectRatioButton->setFixedSize(25, 25);
    m_keepAspectRatioButton->setCheckable(true);
    m_keepAspectRatioButton->setChecked(true);
    m_keepAspectRatioButton->setIcon(QIcon(":/Resource/Icons/KeepAspectRatio.svg"));
    m_keepAspectRatioButton->setToolTip(tr("约束宽高比例"));
    connect(m_keepAspectRatioButton, &QToolButton::toggled, this, [this](bool checked) {
        emit keepAspectRatioToggled(checked);
        m_keepAspectRatioButton->setIcon(checked ? QIcon(":/Resource/Icons/KeepAspectRatio.svg") : QIcon(":/Resource/Icons/AspectRatioClose.svg"));
    });

    m_gridLayout->addWidget(makeLabel("X"), 0, 0);
    m_gridLayout->addWidget(m_spinX, 0, 1);
    m_gridLayout->addWidget(makeLabel("Y"), 0, 3);
    m_gridLayout->addWidget(m_spinY, 0, 4);

    auto* angleIcon = new QLabel;
    angleIcon->setObjectName("HudAngleIcon");
    angleIcon->setAlignment(Qt::AlignCenter);
    const QIcon   icon(":/Resource/Icons/Angle.svg");
    const QPixmap anglePix = icon.pixmap(14, 14);
    angleIcon->setPixmap(anglePix);
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

    auto* alignTool = new QToolButton(parent);
    alignTool->setIcon(QIcon(":/Resource/Icons/AlignLeft.svg"));
    alignTool->setFixedSize(40, 40);
    alignTool->setText(tr("对齐"));
    alignTool->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    alignTool->setArrowType(Qt::NoArrow);
    m_gridLayout->addWidget(alignTool, 0,  9, 2, 1);
    auto* alignMenu = new XHoverMenu(alignTool, XHoverMenu::PopupOrientation::Bottom, this);
    alignMenu->setObjectName("menu_iconMenu");
    alignMenu->setGap(15);
    alignMenu->addAction(QIcon(":/Resource/Icons/AlignLeft.svg"), tr("左对齐"), this, &SelectionHudBar::alignLeft);
    alignMenu->addAction(QIcon(":/Resource/Icons/HorizontalAlignCenter.svg"), tr("水平居中"), this, &SelectionHudBar::alignHorizontalCenter);
    alignMenu->addAction(QIcon(":/Resource/Icons/AlignRight.svg"), tr("右对齐"), this, &SelectionHudBar::alignRight);
    alignMenu->addAction(QIcon(":/Resource/Icons/AlignTop.svg"), tr("顶部对齐"), this, &SelectionHudBar::alignTop);
    alignMenu->addAction(QIcon(":/Resource/Icons/VerticalAlignCenter.svg"), tr("垂直居中"), this, &SelectionHudBar::alignVerticalCenter);
    alignMenu->addAction(QIcon(":/Resource/Icons/AlignBottom.svg"), tr("底部对齐"), this, &SelectionHudBar::alignBottom);
    alignMenu->addAction(QIcon(":/Resource/Icons/AlignCenter.svg"), tr("中心对齐"), this, &SelectionHudBar::alignCenter);

    auto* mirrorTool = new QToolButton(parent);
    mirrorTool->setIcon(QIcon(":/Resource/Icons/MirrorHorizontally.svg"));
    mirrorTool->setFixedSize(40, 40);
    mirrorTool->setText(tr("镜像"));
    mirrorTool->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mirrorTool->setArrowType(Qt::NoArrow);
    m_gridLayout->addWidget(mirrorTool, 0,  10, 2, 1);
    auto* mirrorMenu = new XHoverMenu(mirrorTool, XHoverMenu::PopupOrientation::Bottom, this);
    mirrorMenu->setObjectName("menu_iconMenu");
    mirrorMenu->setGap(15);
    mirrorMenu->addAction(QIcon(":/Resource/Icons/MirrorHorizontally.svg"), tr("水平镜像"), this, &SelectionHudBar::mirrorHorizontal);
    mirrorMenu->addAction(QIcon(":/Resource/Icons/MirrorVertically.svg"), tr("垂直镜像"), this, &SelectionHudBar::mirrorVertical);

    m_booleanTool = new QToolButton(parent);
    m_booleanTool->setIcon(QIcon(":/Resource/Icons/Union.svg"));
    m_booleanTool->setFixedSize(40, 40);
    m_booleanTool->setText(tr("合并"));
    m_booleanTool->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_booleanTool->setArrowType(Qt::NoArrow);
    m_gridLayout->addWidget(m_booleanTool, 0, 11, 2, 1);
    auto* mergeMenu = new XHoverMenu(m_booleanTool, XHoverMenu::PopupOrientation::Bottom, this);
    mergeMenu->setObjectName("menu_iconMenu");
    mergeMenu->setGap(15);
    mergeMenu->addAction(QIcon(":/Resource/Icons/Union.svg"), tr("图形合并"), this, &SelectionHudBar::booleanUnion);
    mergeMenu->addAction(QIcon(":/Resource/Icons/Intersection.svg"), tr("图形相交"), this, &SelectionHudBar::booleanIntersection);
    mergeMenu->addAction(QIcon(":/Resource/Icons/SubtractA.svg"), tr("图形相减"), this, &SelectionHudBar::booleanSubtractAB);
    mergeMenu->addAction(QIcon(":/Resource/Icons/SubtractB.svg"), tr("图形相减"), this, &SelectionHudBar::booleanSubtractBA);

    mainLayout->addLayout(m_gridLayout);
    mainLayout->addStretch();
}

SelectionHudBar::~SelectionHudBar() = default;

bool SelectionHudBar::keepAspectRatio() const
{
    return m_keepAspectRatioButton && m_keepAspectRatioButton->isChecked();
}

void SelectionHudBar::setSummary(const SelectionSummary &summary) {
    hideGridColumn(10, !(summary.onlyVector() && summary.vectorCount == 2));
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
