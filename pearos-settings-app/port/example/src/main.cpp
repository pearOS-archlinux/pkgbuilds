#include "gui/mainwindow.h"
#include "utils/logger.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setApplicationName("pearOS Notes");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("pearOS");
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    Logger::info("Starting pearOS Notes");
    Logger::info(QString("Qt version: %1").arg(qVersion()));
    
    MainWindow window;
    window.show();
    
    Logger::info("Application window shown");
    
    int result = app.exec();
    
    Logger::info("Application exiting");
    return result;
}
