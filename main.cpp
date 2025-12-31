#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
#ifdef Q_OS_WIN
    QApplication::setStyle(QStyleFactory::create("Windows"));
#endif
    QApplication a(argc, argv);
    MainWindow   w;
    w.showMaximized();
    return a.exec();
}
