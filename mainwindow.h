#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MyGraphicsView;
class DrawingToolsBar;
class CanvasWidget;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void init();

private:
    Ui::MainWindow *ui;
    DrawingToolsBar* m_pDrawingToolsBar;
    CanvasWidget*    m_pCanvasWidget;
};
#endif // MAINWINDOW_H
