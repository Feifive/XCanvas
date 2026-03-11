#ifndef EDITORSESSION_H
#define EDITORSESSION_H

#include "Global.h"
#include <QObject>

class EditorSession : public QObject
{
    Q_OBJECT

public:
    explicit EditorSession(QObject* parent = nullptr);

public slots:
    void requestSwitchTool(DrawingToolType type);
    void requestNewFile();
    void requestImportFile();
    void requestOpenFile();
    void requestSaveFile();
    void requestSaveFileAs();
    void notifyZoomChanged(qreal zoomValue);
    void notifyFinishDrawing();
    void setFileActionsEnabled(bool enabled);

signals:
    void switchTool(DrawingToolType type);
    void newFileRequested();
    void importFileRequested();
    void openFileRequested();
    void saveFileRequested();
    void saveFileAsRequested();
    void zoomChanged(qreal zoomValue);
    void finishDrawing();
    void fileActionsEnabledChanged(bool enabled);
};

#endif// EDITORSESSION_H
