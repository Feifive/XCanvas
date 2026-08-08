#include "XHoverMenu.h"
#include <QCoreApplication>
#include <QToolButton>

XHoverMenu::XHoverMenu(QToolButton* button, const PopupOrientation orientation, QWidget* parent)
    : qfw::RoundMenu(QString(), parent), m_button(button), m_orientation(orientation)
{
    if (button)
    {
        installEventFilter(this);
        m_button->installEventFilter(this);

        m_closeTimer.setSingleShot(true);
        connect(&m_closeTimer, &QTimer::timeout, this, &XHoverMenu::onCloseTimeout);
        connect(this, &QMenu::triggered, this, [this]() { m_button->update(); });
    }
}

XHoverMenu::~XHoverMenu()
{
}

void XHoverMenu::setGap(const int gap)
{
    m_gap = gap;
}

bool XHoverMenu::eventFilter(QObject* obj, QEvent* event)
{
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

void XHoverMenu::onCloseTimeout()
{
    if (!m_button || !isVisible())
    {
        return;
    }

    const QPoint globalPos = QCursor::pos();
    const QRect menuRect = rect();

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
    if (!m_button)
    {
        return;
    }

    // Match qfw::RoundMenu animation manager offsets so final popup position stays correct.
    constexpr int kDropDownOffsetY = 4;
    constexpr int kPullUpOffsetY   = 10;

    const QSize btnSize   = m_button->size();
    const QSize menuSize  = sizeHint();
    const int leftMargin  = layout() ? layout()->contentsMargins().left() : 0;
    const int centeredPos = btnSize.width() / 2 - menuSize.width() / 2 + leftMargin;

    QPoint localPos;
    qfw::MenuAnimationType aniType = qfw::MenuAnimationType::DropDown;

    switch (m_orientation)
    {
    case PopupOrientation::Bottom:
    {
        localPos.setX(centeredPos);
        localPos.setY(btnSize.height() + m_gap + kDropDownOffsetY);
        aniType = qfw::MenuAnimationType::DropDown;
        break;
    }
    case PopupOrientation::Top:
    {
        localPos.setX(centeredPos);
        localPos.setY(-m_gap - kPullUpOffsetY);
        aniType = qfw::MenuAnimationType::PullUp;
        break;
    }
    default:
        return;
    }

    const QPoint globalPos = m_button->mapToGlobal(localPos);
    execAt(globalPos, true, aniType);
    m_button->update();
}
