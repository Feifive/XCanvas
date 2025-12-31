#include "HoverMenuHelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QMenu>
#include <QToolButton>

HoverMenuHelper::HoverMenuHelper(QToolButton* button, QMenu* menu, PopupOrientation orientation) : QObject(button), m_button(button), m_menu(menu), m_gap(0), m_orientation(orientation)
{
    m_button->installEventFilter(this);
    m_menu->installEventFilter(this);

    m_closeTimer.setSingleShot(true);
    connect(&m_closeTimer, &QTimer::timeout, this, &HoverMenuHelper::onCloseTimeout);
    connect(m_menu, &QMenu::triggered, this, [this]() { m_button->update(); });
}

void HoverMenuHelper::setGap(int gap)
{
    m_gap = gap;
}

bool HoverMenuHelper::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_button)
    {
        if (event->type() == QEvent::Enter)
        {
            if (m_menu)
            {
                if (m_button->isActiveWindow())
                {
                    m_closeTimer.stop();
                    showMenu();
                }
            }
        }
        else if (event->type() == QEvent::Leave)
        {
            m_closeTimer.start(20);
        }
    }
    else if (obj == m_menu)
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

void HoverMenuHelper::onCloseTimeout()
{
    if (m_menu && m_button && m_menu->isVisible())
    {
        if (!m_menu->rect().contains(m_menu->mapFromGlobal(QCursor::pos())) && !m_button->rect().contains(m_button->mapFromGlobal(QCursor::pos())))
        {
            m_menu->hide();
            m_button->update();
        }
    }
}

void HoverMenuHelper::showMenu()
{
    if (!m_button && !m_menu)
    {
        return;
    }

    const QSize btnSize  = m_button->size();
    const QSize menuSize = m_menu->sizeHint();

    QPoint localPos;

    switch (m_orientation)
    {
    case PopupOrientation::Bottom:
        localPos.setX(btnSize.width() / 2 - menuSize.width() / 2);
        localPos.setY(btnSize.height() + m_gap);
        break;
    case PopupOrientation::Top:
        localPos.setX(btnSize.width() / 2 - menuSize.width() / 2);
        localPos.setY(-menuSize.height() - m_gap);
        break;
    default:
        break;
    }

    QPoint globalPos = m_button->mapToGlobal(localPos);
    m_menu->popup(globalPos);
    m_button->update();
}
