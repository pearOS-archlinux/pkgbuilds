#include "gui/mainwindow.h"
#include "utils/logger.h"
#include "utils/cache_helper.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CacheHelper::ensureCacheRoot();
    
    // Set application metadata
    app.setApplicationName("AppStore");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Arch Linux GUI");
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    Logger::info("Starting AppStore");
    Logger::info(QString("Qt version: %1").arg(qVersion()));
    
    MainWindow window;
    window.show();
    
    Logger::info("Application window shown");
    
    int result = app.exec();
    
    Logger::info("Application exiting");
    return result;
}
