#include "windowgeometrytracker.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDBusConnection>
#include <QWidget>

WindowGeometryTracker::WindowGeometryTracker(QObject* parent) :
    QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/WindowTracker"), this,
            QDBusConnection::ExportScriptableContents);
}

void WindowGeometryTracker::reportWindowGeometry(int pid, const QString& caption, int x, int y, int w, int h)
{
    if(pid != static_cast<int>(QCoreApplication::applicationPid()))
        return;

    const QRect geometry(x, y, w, h);
    const QWidgetList topLevels = qApp->topLevelWidgets();
    for(QWidget* widget : topLevels) {
        Filer::MainWindow* mainWindow = qobject_cast<Filer::MainWindow*>(widget);
        if(mainWindow && mainWindow->windowTitle() == caption) {
            mainWindow->updateSidebarWallpaperTint(geometry);
        }
    }
}
