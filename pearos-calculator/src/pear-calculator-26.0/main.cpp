#include <QApplication>
#include <QTimer>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Calculator"));
    app.setApplicationDisplayName(QStringLiteral("Calculator"));
    app.setDesktopFileName(QStringLiteral("calculator.desktop"));
    MainWindow w;
    w.setWindowTitle(QStringLiteral("Calculator"));
    w.show();
    QTimer::singleShot(0, &w, [&w]() { w.setWindowTitle(QStringLiteral("Calculator")); });
    return app.exec();
}
