#pragma once

#include <qtfluentwidgets.h>

class XTitleBar : public qfw::TitleBar {
public:
    explicit XTitleBar(QWidget* parent = nullptr);

    qfw::TabBar* tabBar() const { return tabBar_; }

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    bool isInTabBarRegion(const QPoint& pos) const;

    qfw::TabBar* tabBar_ = nullptr;
};
