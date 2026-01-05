#include "XMenu.h"
#include "SafeDropShadowEffect.h"

XMenu::XMenu(QWidget* parent, bool needOffsetBlurRadius) : QMenu(parent), m_needOffsetBlurRadius(needOffsetBlurRadius)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    initMenuStyle();
}

XMenu::XMenu(const QString& title, QWidget* parent, bool needOffsetBlurRadius) : QMenu(title, parent), m_needOffsetBlurRadius(needOffsetBlurRadius)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    initMenuStyle();
}

XMenu::~XMenu()
{
}

void XMenu::showEvent(QShowEvent* event)
{
    QMenu::showEvent(event);
    if (m_needOffsetBlurRadius)
    {
        move(pos() - QPoint(blurRadius, blurRadius));
    }
}

void XMenu::initMenuStyle()
{
    auto* shadow = new SafeDropShadowEffect(this);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setBlurRadius(blurRadius);
    setGraphicsEffect(shadow);
    setStyleSheet(QString("XMenu { margin: %1px; }").arg(blurRadius));
}
