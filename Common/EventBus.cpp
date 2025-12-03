#include "EventBus.h"
#include <QMutex>
#include <QMutexLocker>

EventBus& EventBus::instance()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    static EventBus instance;
    return instance;
}
