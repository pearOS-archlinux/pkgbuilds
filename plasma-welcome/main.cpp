#include "mainwindow.h"

#include <QApplication>
#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>

namespace {

// D-Bus counterpart to kwin-scripts/filer-shell-qt5-window-tracker.js: on
// Wayland a client can't query its own global screen position, so the KWin
// script watches this shell's window from the compositor side and relays
// its real geometry here as it moves/resizes. Ported from real Filer's
// WindowGeometryTracker (src/windowgeometrytracker.h/.cpp), simplified
// since this shell only ever has the one MainWindow instance -- no
// pid/caption matching against a list of top-level widgets needed.
class WindowGeometryRelay : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.filershellqt5.WindowTracker")

public:
    WindowGeometryRelay(MainWindow* window, QObject* parent)
        : QObject(parent), window_(window) {
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/WindowTracker"), this,
                QDBusConnection::ExportScriptableContents);
    }

    // pid is `int`, not qlonglong -- see windowgeometrytracker.h's own doc
    // comment: KWin script's callDBus() marshals a JS Number as D-Bus
    // int32, and a signature mismatch here means the call silently fails
    // to dispatch on either side.
    Q_SCRIPTABLE void reportWindowGeometry(int pid, const QString& caption, int x, int y, int w, int h) {
        Q_UNUSED(caption);
        if (pid != static_cast<int>(QCoreApplication::applicationPid()))
            return;
        window_->updateSidebarWallpaperTint(QRect(x, y, w, h));
    }

private:
    MainWindow* window_;
};

// Ported from Application::loadWindowTrackerKwinScript()
// (src/application.cpp:969-1020) -- same isScriptLoaded()/unloadScript()
// idempotency dance (KWin keeps scripts loaded across separate process
// launches, so a stale instance from a previous run has to be unloaded
// first or loadScript() on the same name returns -1), same distinct plugin
// name pattern to sidestep the same wedged-script issue documented there if
// it ever recurs.
void loadWindowTrackerKwinScript() {
    QString scriptPath = QCoreApplication::applicationDirPath()
        + QStringLiteral("/kwin-scripts/filer-shell-qt5-window-tracker.js");
    if (!QFile::exists(scriptPath)) {
        // Development fallback: run straight from qt5/build/ without an
        // install step, same pattern as mainwindow.cpp's assetsBasePath_.
        scriptPath = QCoreApplication::applicationDirPath()
            + QStringLiteral("/../kwin-scripts/filer-shell-qt5-window-tracker.js");
    }
    if (!QFile::exists(scriptPath)) {
        scriptPath = QStringLiteral("/usr/share/pearos-welcome/kwin-scripts/filer-shell-qt5-window-tracker.js");
    }
    if (!QFile::exists(scriptPath)) {
        qWarning() << "filer-shell-qt5-window-tracker.js not found, sidebar wallpaper tint won't track window position. Looked at" << scriptPath;
        return;
    }

    QDBusInterface scripting(QStringLiteral("org.kde.KWin"), QStringLiteral("/Scripting"),
                              QStringLiteral("org.kde.kwin.Scripting"));
    if (!scripting.isValid()) {
        qWarning() << "org.kde.kwin.Scripting D-Bus interface not available, sidebar wallpaper tint won't track window position.";
        return;
    }
    // Versioned suffix ("-v4", bumped again -- "-v3" wedged after this
    // project's own debugging session relaunched the binary many times
    // back-to-back): after enough unload/reload cycles in one KWin session,
    // a given plugin name was observed to end up permanently stuck -- still
    // reporting isScriptLoaded() true and accepting a fresh loadScript()+run()
    // with no error, but its JS body silently never executing again. Since
    // KWin's own scripting engine (not this file) owns whatever internal
    // state got stuck, and there's no unwedge API to call, the name itself
    // is the only lever available -- same fix, same reasoning as real
    // Filer's own "-v2" suffix (src/application.cpp:998-1008). Bump this
    // suffix again if it ever recurs -- and avoid repeatedly relaunching
    // this binary back-to-back during development, since that's exactly
    // what triggers it.
    const QString pluginName = QStringLiteral("filer-shell-qt5-window-tracker-v5");
    QDBusReply<bool> alreadyLoaded = scripting.call(QStringLiteral("isScriptLoaded"), pluginName);
    if (alreadyLoaded.isValid() && alreadyLoaded.value())
        scripting.call(QStringLiteral("unloadScript"), pluginName);

    QDBusReply<int> scriptId = scripting.call(QStringLiteral("loadScript"), scriptPath, pluginName);
    if (!scriptId.isValid() || scriptId.value() < 0) {
        qWarning() << "Failed to load filer-shell-qt5-window-tracker.js:" << scriptId.error().message();
        return;
    }
    QDBusInterface(QStringLiteral("org.kde.KWin"), QStringLiteral("/Scripting/Script%1").arg(scriptId.value()),
                    QStringLiteral("org.kde.kwin.Script")).call(QStringLiteral("run"));
}

}

int main(int argc, char* argv[]) {
    // Matches the KWin script's resourceClass check
    // (kwin-scripts/filer-shell-qt5-window-tracker.js) -- deliberately
    // distinct from real Filer's own "pinder" app-id so the two never get
    // confused if both happen to be running.
    QGuiApplication::setDesktopFileName(QStringLiteral("filer-shell-qt5"));

    QApplication app(argc, argv);

    MainWindow window;

    // Sidebar wallpaper tint (see MainWindow::updateSidebarWallpaperTint()):
    // registers the /WindowTracker D-Bus object under its own service name
    // (distinct from real Filer's org.freedesktop.FileManager1, see
    // WindowGeometryRelay's doc comment), then loads the KWin script that
    // feeds it real window geometry from the compositor side -- this
    // project has no X11 support at all, so this is the only geometry
    // source, unconditionally.
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.filershellqt5.Instance"));
    new WindowGeometryRelay(&window, &app);

    // window.show() must happen before the KWin script loads: the script's
    // own workspace.windowList().forEach(trackWindow) at load time is what
    // actually connects frameGeometryChanged for THIS window -- if the
    // window doesn't exist in the compositor's window list yet (i.e. this
    // ran before show()), that pass finds nothing, and the tracker ends up
    // relying solely on workspace.windowAdded firing afterward instead.
    // That path was observed to silently never fire in practice, leaving
    // the sidebar wallpaper tint permanently stuck at its default color --
    // loading after show() means the initial forEach() always finds and
    // wires up the real window directly, no windowAdded round-trip needed.
    window.show();
    loadWindowTrackerKwinScript();

    return app.exec();
}

#include "main.moc"
