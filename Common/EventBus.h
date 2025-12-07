#ifndef EVENT_BUS
#define EVENT_BUS

#include <QObject>
#include "Global.h"

class EventBus : public QObject
{
    Q_OBJECT

public:
    static EventBus& instance();

signals:
    void switchTool(DrawingToolType);
    void finishDrawing();
    void zoomChanged(const qreal zoomValue);

private:
    EventBus() {}

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
};

#endif // !EVENT_BUS




