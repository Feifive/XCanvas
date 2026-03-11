#include "EditorSession.h"

EditorSession::EditorSession(QObject* parent) : QObject(parent)
{
}

void EditorSession::requestSwitchTool(const DrawingToolType type)
{
    emit switchTool(type);
}

void EditorSession::requestNewFile()
{
    emit newFileRequested();
}

void EditorSession::requestImportFile()
{
    emit importFileRequested();
}

void EditorSession::requestOpenFile()
{
    emit openFileRequested();
}

void EditorSession::requestSaveFile()
{
    emit saveFileRequested();
}

void EditorSession::requestSaveFileAs()
{
    emit saveFileAsRequested();
}

void EditorSession::notifyZoomChanged(const qreal zoomValue)
{
    emit zoomChanged(zoomValue);
}

void EditorSession::notifyFinishDrawing()
{
    emit finishDrawing();
}

void EditorSession::setFileActionsEnabled(const bool enabled)
{
    emit fileActionsEnabledChanged(enabled);
}
