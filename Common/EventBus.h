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
    void newFileRequested();
    void importFileRequested();
    void openFileRequested();
    void saveFileRequested();
    void saveFileAsRequested();
    void fileActionsEnabledChanged(bool enabled);

private:
    EventBus() {}

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
};

#endif // !EVENT_BUS




