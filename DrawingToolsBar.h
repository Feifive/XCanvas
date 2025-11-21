#ifndef DRAWINGTOOLSBAR_H
#define DRAWINGTOOLSBAR_H

#include "Global.h"
#include <QWidget>

class DrawingToolsBar : public QWidget
{
    Q_OBJECT
  public:
    explicit DrawingToolsBar(QWidget* parent = nullptr);
    ~DrawingToolsBar();

  private:
    void onImport();

  signals:
    void DrawingToolRequest(DrawingToolType);
    void Imported(QString filePath);
};

#endif// DRAWINGTOOLSBAR_H
