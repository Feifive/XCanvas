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


private:
	void onSwitchTool(DrawingToolType toolType);

private:
    QToolButton* m_pMainMenu;
    QToolButton* m_pSelectTool;
    QToolButton* m_pImport;
    QToolButton* m_pText;
    QToolButton* m_pPolylineTool;
    QToolButton* m_pCurveTool;
    QToolButton* m_pRectTool;
    QToolButton* m_pEllipseTool;
    QToolButton* m_pPolygonTool;

    QButtonGroup* m_pGroup;
};

#endif// DRAWINGTOOLSBAR_H
