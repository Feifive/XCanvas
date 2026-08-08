#ifndef EDITORPAGEMANAGER_H
#define EDITORPAGEMANAGER_H

#include <QPointer>
#include <QVector>
#include <QObject>
#include <qtfluentwidgets.h>

class QWidget;
class QStackedWidget;
class MyCanvasView;
class DrawingToolsBar;
class EditorSession;

class EditorPageManager final : public QObject
{
    Q_OBJECT

public:
    EditorPageManager(QWidget* hostWindow, QStackedWidget* pageStack, qfw::TabBar* tabBar, QObject* parent = nullptr);

    void initDefaultPage();
    void openProjectInNewPage();
    bool maybeSaveAllBeforeClose();

public slots:
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onTabAddRequested();
    void onTabMoved(int from, int to);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct EditorPageContext
    {
        QWidget*        page = nullptr;
        DrawingToolsBar* drawingToolsBar = nullptr;
        MyCanvasView*    canvasView = nullptr;
        EditorSession*   session = nullptr;
    };

    EditorPageContext createEditorPage();
    void              addEditorPage(const QString& title = QString());
    void              removeEditorPageAt(int index);
    void              refreshTabToolTips();
    void              updateTabTitleForPage(int index);
    void              updateTabTitleForView(MyCanvasView* canvasView);
    bool              isValidPageIndex(int index) const;

private:
    QWidget*                 m_hostWindow;
    QStackedWidget*          m_pageStack;
    qfw::TabBar*             m_tabBar;
    QVector<EditorPageContext> m_editorPages;
    QVector<QPointer<qfw::TabItem>> m_trackedTabItems;
    int                      m_nextPageNumber;
};

#endif // EDITORPAGEMANAGER_H
