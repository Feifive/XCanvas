#include "Common/AppLogging.h"
#include "mainwindow.h"

#include <QApplication>
#include <qtfluentwidgets.h>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    AppLogging::initialize();
    Q_INIT_RESOURCE(resource);
    qfw::setTheme(qfw::Theme::Auto);

    MainWindow w;
    w.showMaximized();

    const int exitCode = a.exec();
    AppLogging::shutdown();
    return exitCode;
}
