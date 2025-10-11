#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    QApplication::setApplicationName("SerialFlow");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("SerialFlow");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
