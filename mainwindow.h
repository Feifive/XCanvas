#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qtfluentwidgets.h>

class QWidget;
class QStackedWidget;
class QCloseEvent;
class EditorPageManager;

class MainWindow : public qfw::FluentWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void init();
    void initMultiPageLayout();
    void fixTitleBarGeometry();

private:
    QStackedWidget* m_pPageStack;
    qfw::TabBar*    m_pTabBar;
    EditorPageManager* m_editorPageManager;
};
#endif // MAINWINDOW_H
