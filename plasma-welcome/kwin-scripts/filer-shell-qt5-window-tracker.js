// Loaded by loadWindowTrackerKwinScript() in main.cpp (Wayland only).
//
// A Wayland client can't query its own global screen position, but KWin
// (the compositor) always knows it -- so this script watches this shell's
// own window (app-id "filer-shell-qt5", see QGuiApplication::setDesktopFileName()
// in main.cpp) and relays its real frame geometry back over D-Bus, the same
// way real Filer's src/kwin-scripts/filer-window-tracker.js feeds
// src/windowgeometrytracker.h. Used by MainWindow::updateSidebarWallpaperTint()
// to retint the sidebar from the wallpaper behind it, like macOS's sidebar
// vibrancy.
//
// Distinct service/interface names from real Filer's own
// (org.freedesktop.FileManager1 / org.filer.WindowTracker) so this shell
// never collides with an actual Filer instance running on the same session
// bus.

function reportGeometry(window) {
    if (!window || window.resourceClass !== "filer-shell-qt5") {
        return;
    }
    callDBus("org.filershellqt5.Instance", "/WindowTracker", "org.filershellqt5.WindowTracker", "reportWindowGeometry",
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
