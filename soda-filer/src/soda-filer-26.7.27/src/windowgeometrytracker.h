#ifndef WINDOWGEOMETRYTRACKER_H
#define WINDOWGEOMETRYTRACKER_H

#include <QObject>
#include <QString>

// D-Bus counterpart to src/kwin-scripts/filer-window-tracker.js: on Wayland a
// client can't query its own global screen position, so the KWin script
// watches Filer's windows (app-id "pinder") from the compositor side and
// relays their real geometry here as they move/resize, keying by pid+caption
// since Filer's MainWindow instances all share one process (see
// Application::loadWindowTrackerKwinScript()).
class WindowGeometryTracker : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.filer.WindowTracker")

public:
    explicit WindowGeometryTracker(QObject* parent = nullptr);
    // pid is `int`, not qlonglong: KWin script's callDBus() marshals a JS
    // Number as D-Bus int32, and a signature mismatch here means the call
    // silently fails to dispatch (no error surfaces on either side) --
    // confirmed by a manual qdbus call (which does send int64) working fine
    // while the live KWin-script-triggered path produced no tint change at all.
    Q_SCRIPTABLE void reportWindowGeometry(int pid, const QString& caption, int x, int y, int w, int h);
};

#endif // WINDOWGEOMETRYTRACKER_H
