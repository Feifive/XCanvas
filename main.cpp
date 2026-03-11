#include "mainwindow.h"

#include <QApplication>
#include <qtfluentwidgets.h>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    Q_INIT_RESOURCE(resource);
    qfw::setTheme(qfw::Theme::Auto);

    MainWindow   w;
    w.showMaximized();
    return a.exec();
}
