#include "mainwindow.h"

#include <QApplication>
#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QIcon>

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
// first or loadScript() on the same name returns -1).
//
// Returns the plugin name actually loaded under (empty on failure) so main()
// can unload it again on clean shutdown -- see aboutToQuit below.
QString loadWindowTrackerKwinScript() {
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
        return QString();
    }

    QDBusInterface scripting(QStringLiteral("org.kde.KWin"), QStringLiteral("/Scripting"),
                              QStringLiteral("org.kde.kwin.Scripting"));
    if (!scripting.isValid()) {
        qWarning() << "org.kde.kwin.Scripting D-Bus interface not available, sidebar wallpaper tint won't track window position.";
        return QString();
    }
    // PID-scoped name instead of a hand-bumped versioned suffix ("-v3"
    // through "-v5" wedged in turn during this project's own debugging
    // sessions, each after relaunching the binary many times back-to-back
    // in one KWin session): KWin was observed to permanently wedge a script
    // name after enough unload/reload cycles of that *same* name -- still
    // reporting isScriptLoaded() true and accepting a fresh loadScript()+
    // run() with no error, but its JS body silently never executing again.
    // A name that's never reused within a KWin session can't accumulate
    // those cycles in the first place, so it can't wedge -- no more manual
    // suffix bumps needed. aboutToQuit below unloads it again on clean exit
    // so relaunches don't pile up orphaned scripts in a long-lived KWin
    // session; the isScriptLoaded()/unloadScript() dance right here still
    // covers the crash/SIGKILL case where a same-PID name recurs after PIDs
    // wrap around.
    const QString pluginName = QStringLiteral("filer-shell-qt5-window-tracker-%1")
        .arg(QCoreApplication::applicationPid());
    QDBusReply<bool> alreadyLoaded = scripting.call(QStringLiteral("isScriptLoaded"), pluginName);
    if (alreadyLoaded.isValid() && alreadyLoaded.value())
        scripting.call(QStringLiteral("unloadScript"), pluginName);

    QDBusReply<int> scriptId = scripting.call(QStringLiteral("loadScript"), scriptPath, pluginName);
    if (!scriptId.isValid() || scriptId.value() < 0) {
        qWarning() << "Failed to load filer-shell-qt5-window-tracker.js:" << scriptId.error().message();
        return QString();
    }
    QDBusInterface(QStringLiteral("org.kde.KWin"), QStringLiteral("/Scripting/Script%1").arg(scriptId.value()),
                    QStringLiteral("org.kde.kwin.Script")).call(QStringLiteral("run"));
    return pluginName;
}

}

int main(int argc, char* argv[]) {
    // Matches the KWin script's resourceClass check
    // (kwin-scripts/filer-shell-qt5-window-tracker.js) -- deliberately
    // distinct from real Filer's own "pinder" app-id so the two never get
    // confused if both happen to be running.
    QGuiApplication::setDesktopFileName(QStringLiteral("filer-shell-qt5"));

    QApplication app(argc, argv);

    // The pearOS variant icon themes (pearOS-<color>[-dark|-light]) declare
    // Inherits=hicolor,breeze in their index.theme, so QIcon::fromTheme()
    // falls through to the standard freedesktop icon names setupWelcomeContent()
    // uses (system-software-install, emblem-favorite, ...). The bare "pearOS"
    // theme itself ships with no index.theme at all -- QIcon::themeName()
    // still resolves to it correctly from kdeglobals regardless of platform-
    // theme integration, but with no Inherits chain every fromTheme() call
    // silently returns a null icon (confirmed via QIcon::hasThemeIcon()
    // returning false for a sentinel name on an affected install). Detect
    // that and fall back to pearOS-dark instead -- a real installed pearOS
    // variant with the missing Inherits chain, so the icon set stays
    // on-brand instead of jumping to generic breeze artwork.
    if (!QIcon::hasThemeIcon(QStringLiteral("system-software-install"))) {
        qWarning() << "Icon theme" << QIcon::themeName()
                   << "has no usable icons (missing index.theme/Inherits?), falling back to pearOS-dark";
        QIcon::setThemeName(QStringLiteral("pearOS-dark"));
    }

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
    const QString trackerPluginName = loadWindowTrackerKwinScript();

    // Unload our own PID-scoped script on clean shutdown so relaunches
    // across a long-lived KWin session don't pile up orphaned scripts --
    // see loadWindowTrackerKwinScript()'s own comment. Best-effort only:
    // skipped entirely on a crash/SIGKILL, same as any other cleanup here.
    if (!trackerPluginName.isEmpty()) {
        QObject::connect(&app, &QCoreApplication::aboutToQuit, [trackerPluginName]() {
            QDBusInterface scripting(QStringLiteral("org.kde.KWin"), QStringLiteral("/Scripting"),
                                      QStringLiteral("org.kde.kwin.Scripting"));
            if (scripting.isValid())
                scripting.call(QStringLiteral("unloadScript"), trackerPluginName);
        });
    }

    return app.exec();
}

#include "main.moc"
