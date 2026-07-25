#include "menubarmanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>

const QString MenuBarManager::CONFIG_FILE = QStringLiteral("filer-topbar-appletsrc");

// Known systray plugin ids → friendly label + icon (freedesktop icon names)
static const struct { const char *id; const char *label; const char *icon; } kKnownTrayItems[] = {
    {"org.kde.plasma.networkmanagement", "Wi-Fi & Network",     "network-wireless"},
    {"org.kde.plasma.bluetooth",         "Bluetooth",           "preferences-system-bluetooth"},
    {"org.kde.plasma.volume",            "Volume",              "audio-volume-high"},
    {"org.kde.kscreen",                  "Displays",            "video-display"},
    {"org.kde.plasma.printmanager",      "Printers",            "printer"},
    {"org.kde.plasma.cameraindicator",   "Camera",              "camera-web"},
    {"org.kde.plasma.clipboard",         "Clipboard",           "edit-paste"},
    {"org.kde.plasma.manage-inputmethod","Input Method",        "input-keyboard"},
    {"org.kde.plasma.notifications",     "Notifications",       "preferences-desktop-notification"},
    {"org.kde.kdeconnect",               "KDE Connect",         "smartphone"},
    {"org.kde.plasma.mediacontroller",   "Media Controller",    "multimedia-player"},
    {"org.kde.plasma.devicenotifier",    "Removable Devices",   "drive-removable-media"},
};

MenuBarManager::MenuBarManager(QObject *parent) : QObject(parent) {}

void MenuBarManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

// Locate the menu-bar panel containment (hosts xyz.pearos.pearmenu) and its
// nested systray applet, by scanning the ini file — ids aren't stable across installs.
void MenuBarManager::findIds() {
    if (!m_containmentId.isEmpty()) return;

    QFile f(QDir::homePath() + "/.config/" + CONFIG_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    static QRegularExpression containmentRe(R"(^\[Containments\]\[(\d+)\]$)");
    static QRegularExpression appletRe(R"(^\[Containments\]\[(\d+)\]\[Applets\]\[(\d+)\]$)");

    QString curContainment;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (auto m = containmentRe.match(line); m.hasMatch()) {
            curContainment = m.captured(1);
        } else if (auto m = appletRe.match(line); m.hasMatch()) {
            curContainment = m.captured(1);
        } else if (line.trimmed() == "plugin=xyz.pearos.pearmenu") {
            m_containmentId = curContainment;
        }
    }
    f.close();

    if (m_containmentId.isEmpty()) return;

    // Re-scan cleanly for the systray applet id within that containment
    QFile f2(QDir::homePath() + "/.config/" + CONFIG_FILE);
    if (!f2.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    static QRegularExpression thisContainmentAppletRe(
        QString(R"(^\[Containments\]\[%1\]\[Applets\]\[(\d+)\]$)").arg(m_containmentId));
    QTextStream in2(&f2);
    QString pendingApplet;
    while (!in2.atEnd()) {
        QString line = in2.readLine();
        if (auto m = thisContainmentAppletRe.match(line); m.hasMatch()) {
            pendingApplet = m.captured(1);
        } else if (!pendingApplet.isEmpty() && line.trimmed() == "plugin=org.kde.plasma.systemtray") {
            m_systrayAppletId = pendingApplet;
            break;
        } else if (line.startsWith('[')) {
            pendingApplet.clear();
        }
    }
}

QString MenuBarManager::readKey(const QString &groupPath, const QString &key, const QString &defaultVal) const {
    QStringList args;
    args << "--file" << CONFIG_FILE;
    for (const QString &g : groupPath.split('/'))
        args << "--group" << g;
    args << "--key" << key;
    QProcess proc;
    proc.start("kreadconfig6", args);
    proc.waitForFinished(2000);
    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    return out.isEmpty() ? defaultVal : out;
}

QStringList MenuBarManager::readList(const QString &groupPath, const QString &key) const {
    QString raw = readKey(groupPath, key);
    QStringList out;
    for (const QString &s : raw.split(',', Qt::SkipEmptyParts))
        out << s.trimmed();
    return out;
}

// kwriteconfig6 chokes on numeric group names (containment/applet ids),
// so patch the ini file directly — same approach DockManager uses.
void MenuBarManager::writeKey(const QString &groupPath, const QString &key, const QString &value) const {
    QString cfgPath = QDir::homePath() + "/.config/" + CONFIG_FILE;
    QFile f(cfgPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QString targetSection = "[" + groupPath.split('/').join("][") + "]";
    QStringList lines = QString::fromUtf8(f.readAll()).split('\n');
    f.close();

    bool inSection = false;
    bool keyWritten = false;
    int sectionLine = -1;

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i].trimmed();
        if (line.startsWith('[')) {
            if (inSection && !keyWritten) {
                lines.insert(i, key + "=" + value);
                keyWritten = true;
                break;
            }
            inSection = (line == targetSection);
            if (inSection) sectionLine = i;
        } else if (inSection && (line.startsWith(key + "=") || line.startsWith(key + " ="))) {
            lines[i] = key + "=" + value;
            keyWritten = true;
            break;
        }
    }

    if (!keyWritten) {
        if (sectionLine < 0) {
            lines << "";
            lines << targetSection;
        }
        lines << (key + "=" + value);
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return;
    QTextStream out(&f);
    out << lines.join('\n');
}

// Apply live via Plasma's scripting API — same mechanism System Settings itself
// uses, no plasmashell restart needed.
void MenuBarManager::notifyMenuBar() const {
    if (m_systrayAppletId.isEmpty()) return;
    QString script = QString(
        "var allPanels = panels();"
        "for (var i = 0; i < allPanels.length; i++) {"
        "  var ws = allPanels[i].widgets();"
        "  for (var j = 0; j < ws.length; j++) {"
        "    if (ws[j].type === 'org.kde.plasma.systemtray') {"
        "      ws[j].reloadConfig();"
        "    }"
        "  }"
        "}");
    QProcess::startDetached("qdbus6", {
        "org.kde.plasmashell", "/PlasmaShell",
        "org.kde.PlasmaShell.evaluateScript", script
    });
}

void MenuBarManager::refresh() {
    findIds();
    if (m_containmentId.isEmpty()) { emit changed(); return; }

    int vis = readKey(m_containmentId + "/General", "panelVisibility", "0").toInt();
    m_autoHide = (vis != 0);

    QStringList hidden = m_systrayAppletId.isEmpty() ? QStringList()
        : readList(m_containmentId + "/Applets/" + m_systrayAppletId + "/General", "hiddenItems");

    m_trayItems.clear();
    for (const auto &item : kKnownTrayItems) {
        QVariantMap entry;
        entry["id"]      = QString(item.id);
        entry["label"]   = QString(item.label);
        entry["icon"]    = QString(item.icon);
        entry["visible"] = !hidden.contains(QString(item.id));
        m_trayItems.append(entry);
    }

    emit changed();
}

void MenuBarManager::setAutoHide(bool v) {
    m_autoHide = v;
    emit changed();
    if (m_containmentId.isEmpty()) return;
    writeKey(m_containmentId + "/General", "panelVisibility", v ? "1" : "0");

    // Live-apply without restart via Plasma scripting: find the panel by containment id
    QString script = QString(
        "var allPanels = panels();"
        "for (var i = 0; i < allPanels.length; i++) {"
        "  if (allPanels[i].id == %1) {"
        "    allPanels[i].hiding = '%2';"
        "  }"
        "}").arg(m_containmentId, v ? "autohide" : "none");
    QProcess::startDetached("qdbus6", {
        "org.kde.plasmashell", "/PlasmaShell",
        "org.kde.PlasmaShell.evaluateScript", script
    });
}

void MenuBarManager::setTrayItemVisible(const QString &id, bool visible) {
    for (auto &v : m_trayItems) {
        QVariantMap m = v.toMap();
        if (m["id"].toString() == id) {
            m["visible"] = visible;
            v = m;
            break;
        }
    }
    emit changed();

    if (m_systrayAppletId.isEmpty()) return;
    QString groupPath = m_containmentId + "/Applets/" + m_systrayAppletId + "/General";
    QStringList hidden = readList(groupPath, "hiddenItems");
    QStringList shown  = readList(groupPath, "shownItems");

    hidden.removeAll(id);
    shown.removeAll(id);
    if (visible) shown << id;
    else         hidden << id;

    writeKey(groupPath, "hiddenItems", hidden.join(','));
    writeKey(groupPath, "shownItems",  shown.join(','));
    notifyMenuBar();
}
