#ifndef DRAWINGTOOLSBAR_H
#define DRAWINGTOOLSBAR_H

#include "Global.h"
#include "XCanvasIcon.h"
#include <qtfluentwidgets.h>
#include <QToolBar>

class QButtonGroup;
class EditorSession;

class DrawingToolsBar : public QToolBar
{
    Q_OBJECT
  public:
    explicit DrawingToolsBar(EditorSession* session, QWidget* parent = nullptr);
    ~DrawingToolsBar() override;

  private:
    void createToolBar();
    void syncCurrentTool(DrawingToolType type) const;
    void initMainMenu();
    qfw::RoundMenu* createMainMenu(const QPoint& pos);
    void onFileActionsEnabledChanged(bool enabled) const;
    void applyStyle();
    static void setupToolTip(QWidget* button, const QString& text);
    qfw::TransparentToggleToolButton* makeToggleButton(XCanvasIconType iconType);
    qfw::TransparentToolButton* makeActionButton(XCanvasIconType iconType);

  private:
    qfw::TransparentToolButton* m_pMainMenu;
    qfw::TransparentToolButton* m_pImport;
    qfw::TransparentToggleToolButton* m_pSelectTool;
    qfw::TransparentToggleToolButton* m_pDrawingToolLock;
    qfw::TransparentToggleToolButton* m_pText;
    qfw::TransparentToggleToolButton* m_pPolylineTool;
    qfw::TransparentToggleToolButton* m_pCurveTool;
    qfw::TransparentToggleToolButton* m_pRectTool;
    qfw::TransparentToggleToolButton* m_pEllipseTool;
    qfw::TransparentToggleToolButton* m_pPolygonTool;

    QButtonGroup* m_pGroup;
    qfw::RoundMenu* m_pMainRoundMenu = nullptr;
    qfw::Action*  m_openAction   = nullptr;
    qfw::Action*  m_importAction = nullptr;
    qfw::Action*  m_saveAction   = nullptr;
    qfw::Action*  m_saveAsAction = nullptr;
    EditorSession* m_editorSession = nullptr;
};

#endif// DRAWINGTOOLSBAR_H
