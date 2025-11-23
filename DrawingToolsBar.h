#ifndef DRAWINGTOOLSBAR_H
#define DRAWINGTOOLSBAR_H

#include "Global.h"
#include <QToolBar>

class QToolButton;
class QButtonGroup;

class DrawingToolsBar : public QToolBar
{
    Q_OBJECT
  public:
    explicit DrawingToolsBar(QWidget* parent = nullptr);
    ~DrawingToolsBar();
    void OnToolFinished();

  private:
    void onImport();

  signals:
    void DrawingToolRequest(DrawingToolType);
    void Imported(QString filePath);

private:
    QToolButton* m_pMainMenu;
    QToolButton* m_pSelectTool;
    QToolButton* m_pImport;
    QToolButton* m_pRectTool;
    QToolButton* m_pPolylineTool;
    QToolButton* m_pEllipseTool;

    QButtonGroup* m_pGroup;
};

#endif// DRAWINGTOOLSBAR_H
