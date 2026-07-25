// Loaded by Application::loadWindowTrackerKwinScript() (Wayland only).
//
// A Wayland client can't query its own global screen position, but KWin
// (the compositor) always knows it -- so this script watches Filer's own
// windows (app-id "pinder", see QGuiApplication::setDesktopFileName() in
// application.cpp) and relays their real frame geometry back to Filer over
// D-Bus, keyed by pid+caption (src/windowgeometrytracker.h matches on that).
// Filer uses this to retint its sidebar from the wallpaper behind it, like
// macOS's sidebar vibrancy.

function reportGeometry(window) {
    if (!window || window.resourceClass !== "pinder") {
        return;
    }
    callDBus("org.freedesktop.FileManager1", "/WindowTracker", "org.filer.WindowTracker", "reportWindowGeometry",
        window.pid, window.caption,
        Math.round(window.frameGeometry.x), Math.round(window.frameGeometry.y),
        Math.round(window.frameGeometry.width), Math.round(window.frameGeometry.height));
}

function trackWindow(window) {
    reportGeometry(window);
    window.frameGeometryChanged.connect(function() { reportGeometry(window); });
}

workspace.windowList().forEach(trackWindow);
workspace.windowAdded.connect(trackWindow);
