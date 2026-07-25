#include "screentimemanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <algorithm>

ScreenTimeManager::ScreenTimeManager(QObject *parent) : QObject(parent) {}

void ScreenTimeManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

QString ScreenTimeManager::dataFilePath() {
    QString dir = QDir::homePath() + "/.local/share/pearos-settings";
    QDir().mkpath(dir);
    return dir + "/screentime.json";
}

QString ScreenTimeManager::collectorScriptPath() {
    QString dir = QDir::homePath() + "/.local/share/pearos-settings";
    QDir().mkpath(dir);
    return dir + "/screentime-collector.py";
}

QString ScreenTimeManager::kwinScriptPath() {
    QString dir = QDir::homePath() + "/.local/share/pearos-settings";
    QDir().mkpath(dir);
    return dir + "/screentime-kwin.js";
}

// KWin itself (not X11/xdotool — works identically under Wayland and X11)
// reports every window activation here. resourceClass is KWin's normalized
// app id: WM_CLASS on X11, xdg_toplevel app_id on Wayland — same property
// either way, so this is backend-agnostic.
static const char *kKwinScript = R"JS(
function report(win) {
    var cls = (win && win.resourceClass) ? win.resourceClass : "";
    callDBus("org.pearos.ScreenTime", "/Collector", "org.pearos.ScreenTime.Collector", "reportActiveWindow", cls);
}
workspace.windowActivated.connect(report);
if (workspace.activeWindow) report(workspace.activeWindow);
)JS";

// Event-driven collector: KWin calls reportActiveWindow() on every window
// switch, and we bill the elapsed time to whichever app was active since the
// last report. A periodic flush covers the case where the user stays on one
// window for a long time (no activation event to trigger a commit).
static const char *kCollectorScript = R"PY(#!/usr/bin/env python3
import dbus, dbus.service, subprocess, json, os, time
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

DATA_FILE = "%1"
FLUSH_INTERVAL = 15

def load():
    try:
        with open(DATA_FILE) as f:
            return json.load(f)
    except Exception:
        return {}

def save(data):
    tmp = DATA_FILE + ".tmp"
    with open(tmp, "w") as f:
        json.dump(data, f)
    os.replace(tmp, DATA_FILE)

def is_locked():
    try:
        out = subprocess.run(
            ["qdbus6", "org.freedesktop.ScreenSaver", "/ScreenSaver",
             "org.freedesktop.ScreenSaver.GetActive"],
            capture_output=True, text=True, timeout=2)
        return out.stdout.strip() == "true"
    except Exception:
        return False

class Collector(dbus.service.Object):
    def __init__(self, bus):
        dbus.service.Object.__init__(self, bus, "/Collector")
        self.current = ""
        self.last_ts = time.time()

    def _flush(self, now):
        if self.current and not is_locked():
            elapsed = int(now - self.last_ts)
            if elapsed > 0:
                data = load()
                today = time.strftime("%Y-%m-%d")
                day = data.setdefault(today, {})
                day[self.current] = day.get(self.current, 0) + elapsed
                save(data)
        self.last_ts = now

    @dbus.service.method("org.pearos.ScreenTime.Collector", in_signature="s")
    def reportActiveWindow(self, class_name):
        now = time.time()
        self._flush(now)
        self.current = str(class_name) if class_name else ""

    def periodic_flush(self):
        self._flush(time.time())
        return True

def main():
    DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    bus_name = dbus.service.BusName("org.pearos.ScreenTime", bus)  # noqa: F841 (must stay alive)
    collector = Collector(bus)
    GLib.timeout_add_seconds(FLUSH_INTERVAL, collector.periodic_flush)
    GLib.MainLoop().run()

if __name__ == "__main__":
    main()
)PY";

void ScreenTimeManager::installCollectorScript() {
    QString content = QString::fromUtf8(kCollectorScript).arg(dataFilePath());
    QFile f(collectorScriptPath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        f.write(content.toUtf8());
        f.close();
        QFile::setPermissions(collectorScriptPath(),
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
            QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther);
    }
}

void ScreenTimeManager::installKwinScript() {
    QFile f(kwinScriptPath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        f.write(QByteArray(kKwinScript));
    }
}

void ScreenTimeManager::installSystemdUnit() {
    QString dir = QDir::homePath() + "/.config/systemd/user";
    QDir().mkpath(dir);

    QFile f(dir + "/pearos-screentime.service");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream(&f) <<
            "[Unit]\nDescription=PearOS Screen Time collector\n\n"
            "[Service]\nType=simple\nExecStart=/usr/bin/python3 \"" << collectorScriptPath() << "\"\nRestart=always\nRestartSec=5\n\n"
            "[Install]\nWantedBy=graphical-session.target\n";
    }

    // (Re)load the KWin script and start it, then bring up the collector
    // service, then confirm both are actually live before reporting success.
    QString kwinPath = kwinScriptPath();
    run(QString(
            "qdbus6 org.kde.KWin /Scripting org.kde.kwin.Scripting.unloadScript pearos-screentime >/dev/null 2>&1; "
            "qdbus6 org.kde.KWin /Scripting org.kde.kwin.Scripting.loadScript '%1' pearos-screentime >/dev/null 2>&1; "
            "qdbus6 org.kde.KWin /Scripting org.kde.kwin.Scripting.start >/dev/null 2>&1; "
            "systemctl --user daemon-reload && systemctl --user enable --now pearos-screentime.service >/dev/null 2>&1; "
            "sleep 0.3; "
            "echo -n \"svc=\"; systemctl --user is-active pearos-screentime.service; "
            "echo -n \" kwin=\"; qdbus6 org.kde.KWin /Scripting org.kde.kwin.Scripting.isScriptLoaded pearos-screentime"
        ).arg(kwinPath),
        [this](QString out) {
            bool svcActive  = out.contains("svc=active");
            bool kwinLoaded = out.contains("kwin=true");
            m_trackingActive = svcActive && kwinLoaded;
            emit changed();
        });
}

void ScreenTimeManager::ensureTrackerRunning() {
    installCollectorScript();
    installKwinScript();
    installSystemdUnit();
}

void ScreenTimeManager::refresh() {
    QFile f(dataFilePath());
    QJsonObject root;
    if (f.open(QIODevice::ReadOnly)) {
        root = QJsonDocument::fromJson(f.readAll()).object();
    }

    QDate today = QDate::currentDate();
    QString todayKey = today.toString("yyyy-MM-dd");

    QJsonObject todayObj = root.value(todayKey).toObject();
    QList<QPair<int, QString>> sorted;
    int total = 0;
    for (auto it = todayObj.begin(); it != todayObj.end(); ++it) {
        int secs = it.value().toInt();
        total += secs;
        sorted.append({secs, it.key()});
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) { return a.first > b.first; });

    int maxSecs = sorted.isEmpty() ? 1 : sorted.first().first;
    QVariantList apps;
    for (const auto &[secs, name] : sorted) {
        QVariantMap m;
        m["id"] = name;
        m["label"] = name;
        m["seconds"] = secs;
        m["rel"] = double(secs) / maxSecs;
        apps.append(m);
    }
    m_todayApps = apps;
    m_todayTotalSecs = total;

    QVariantList week;
    for (int i = 6; i >= 0; --i) {
        QDate d = today.addDays(-i);
        QJsonObject dayObj = root.value(d.toString("yyyy-MM-dd")).toObject();
        int daySecs = 0;
        for (auto it = dayObj.begin(); it != dayObj.end(); ++it)
            daySecs += it.value().toInt();
        QVariantMap m;
        m["date"] = d.toString("yyyy-MM-dd");
        m["label"] = d.toString("ddd");
        m["seconds"] = daySecs;
        week.append(m);
    }
    m_weekTotals = week;

    emit changed();

    run("echo -n \"svc=\"; systemctl --user is-active pearos-screentime.service; "
        "echo -n \" kwin=\"; qdbus6 org.kde.KWin /Scripting org.kde.kwin.Scripting.isScriptLoaded pearos-screentime",
        [this](QString out) {
            m_trackingActive = out.contains("svc=active") && out.contains("kwin=true");
            emit changed();
        });
}
