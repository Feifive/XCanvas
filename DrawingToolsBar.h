#ifndef DRAWINGTOOLSBAR_H
#define DRAWINGTOOLSBAR_H

#include <QWidget>
#include "Global.h"

class DrawingToolsBar : public QWidget
{
    Q_OBJECT
public:
    explicit DrawingToolsBar(QWidget *parent = nullptr);
    ~DrawingToolsBar();

signals:
    void DrawingToolRequest(DrawingToolType);
};

#endif // DRAWINGTOOLSBAR_H
