#include <iostream>
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("artistimagefilers");
    app.setOrganizationName("crystalvalente");
    MainWindow window;
    window.show();
    return app.exec();
}