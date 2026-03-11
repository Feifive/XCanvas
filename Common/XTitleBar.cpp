#include "XTitleBar.h"

XTitleBar::XTitleBar(QWidget* parent) : qfw::TitleBar(parent) {
    setFixedHeight(48);

    tabBar_ = new qfw::TabBar(this);
    tabBar_->setAttribute(Qt::WA_TranslucentBackground);
    tabBar_->setStyleSheet("background-color: translucent; border:none;");
    tabBar_->setScrollable(false);
    tabBar_->setMovable(false);
    tabBar_->setTabsClosable(true);
    tabBar_->setAddButtonVisible(true);
    tabBar_->setCloseButtonDisplayMode(qfw::TabCloseButtonDisplayMode::OnHover);
    tabBar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (const QLayoutItem* item = hBoxLayout()->takeAt(0)) {
        delete item;
    }

    hBoxLayout()->setSpacing(0);
    hBoxLayout()->setContentsMargins(0, 0, 0, 0);
    hBoxLayout()->insertWidget(0, tabBar_, 1, Qt::AlignVCenter);
    hBoxLayout()->insertSpacing(1, 40);
}

bool XTitleBar::isInTabBarRegion(const QPoint& pos) const {
    if (!tabBar_ || !tabBar_->geometry().contains(pos)) {
        return false;
    }

    const QPoint localPos = tabBar_->mapFromParent(pos);
    QWidget* child = tabBar_->childAt(localPos);

    while (child) {
        if (qobject_cast<qfw::TabItem*>(child) || qobject_cast<qfw::TabToolButton*>(child)) {
            return true;
        }
        child = child->parentWidget();
    }

    return false;
}

void XTitleBar::mousePressEvent(QMouseEvent* event) {
    if (isInTabBarRegion(event->pos())) {
        QWidget::mousePressEvent(event);
        return;
    }
    qfw::TitleBar::mousePressEvent(event);
}

void XTitleBar::mouseMoveEvent(QMouseEvent* event) {
    if (isInTabBarRegion(event->pos())) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    qfw::TitleBar::mouseMoveEvent(event);
}

void XTitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (isInTabBarRegion(event->pos())) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    qfw::TitleBar::mouseDoubleClickEvent(event);
}
