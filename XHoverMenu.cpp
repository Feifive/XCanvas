#include "XHoverMenu.h"
#include "SafeDropShadowEffect.h"
#include <QToolButton>
#include <QCoreApplication>

constexpr int blurRadius = 20;

XHoverMenu::XHoverMenu(QToolButton *button, const PopupOrientation orientation, QWidget *parent) : QMenu(parent), m_button(button), m_orientation(orientation) {
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    initMenuStyle();

    if (button) {
        installEventFilter(this);
        m_button->installEventFilter(this);

        m_closeTimer.setSingleShot(true);
        connect(&m_closeTimer, &QTimer::timeout, this, &XHoverMenu::onCloseTimeout);
        connect(this, &QMenu::triggered, this, [this]() { m_button->update(); });
    }
}

XHoverMenu::~XHoverMenu() {
}

void XHoverMenu::setGap(const int gap) {
    m_gap = gap;
}

bool XHoverMenu::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_button)
    {
        if (event->type() == QEvent::Enter)
        {
            if (m_button->isActiveWindow())
            {
                m_closeTimer.stop();
                showMenu();
            }
        }
        else if (event->type() == QEvent::Leave)
        {
            m_closeTimer.start(20);
        }
    }
    else if (obj == this)
    {
        if (event->type() == QEvent::Enter)
        {
            m_closeTimer.stop();
        }

        if (event->type() == QEvent::MouseMove)
        {
            if (!m_button->rect().contains(m_button->mapFromGlobal(QCursor::pos())))
            {
                QEvent leaveEvent = QEvent(QEvent::Leave);
                qApp->sendEvent(m_button, &leaveEvent);
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void XHoverMenu::initMenuStyle() {
    auto* shadow = new SafeDropShadowEffect(this);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setBlurRadius(blurRadius);
    setGraphicsEffect(shadow);
    setStyleSheet(QString("margin: %1px").arg(blurRadius));
}

void XHoverMenu::onCloseTimeout()
{
    if (!m_button || !isVisible()) {
        return;
    }

    const QPoint globalPos = QCursor::pos();

    const QRect menuRect = rect().adjusted(blurRadius,blurRadius,-blurRadius,-blurRadius);

    const QPoint menuPos = mapFromGlobal(globalPos);
    const QPoint btnPos  = m_button->mapFromGlobal(globalPos);

    if (!menuRect.contains(menuPos) && !m_button->rect().contains(btnPos))
    {
        hide();
        m_button->update();
    }
}

void XHoverMenu::showMenu()
{
    if (!m_button) {
        return;
    }

    const QSize btnSize  = m_button->size();
    const QSize menuSize = sizeHint();

    QPoint localPos;

    switch (m_orientation)
    {
        case PopupOrientation::Bottom:
        {
            localPos.setX(btnSize.width() / 2- menuSize.width() / 2);
            localPos.setY(btnSize.height() + m_gap - blurRadius);
            break;
        }
        case PopupOrientation::Top:
        {
            localPos.setX(btnSize.width() / 2- menuSize.width() / 2);
            localPos.setY(-menuSize.height()- m_gap+ blurRadius);
            break;
        }
        default:
            return;
    }

    const QPoint globalPos = m_button->mapToGlobal(localPos);
    popup(globalPos);
    m_button->update();
}
