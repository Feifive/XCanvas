#ifndef DRAWINGTOOLSBAR_H
#define DRAWINGTOOLSBAR_H

#include "Global.h"
#include <QToolBar>

class QToolButton;
class QButtonGroup;
class QAction;

class DrawingToolsBar : public QToolBar
{
    Q_OBJECT
  public:
    explicit DrawingToolsBar(QWidget* parent = nullptr);
    ~DrawingToolsBar() override;

  private:
    void onFinishDrawing() const;
    void initMainMenu();
    void onFileActionsEnabledChanged(bool enabled) const;

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
    QAction*      m_openAction   = nullptr;
    QAction*      m_importAction = nullptr;
    QAction*      m_saveAction   = nullptr;
    QAction*      m_saveAsAction = nullptr;
};

#endif// DRAWINGTOOLSBAR_H
