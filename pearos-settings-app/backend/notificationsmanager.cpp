#include "notificationsmanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>

const QString NotificationsManager::CONFIG_FILE = QStringLiteral("plasmanotifyrc");

NotificationsManager::NotificationsManager(QObject *parent) : QObject(parent) {}

void NotificationsManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

QString NotificationsManager::readKey(const QString &group, const QString &key, const QString &defaultVal) const {
    return readKey2(group, {}, key, defaultVal);
}

QString NotificationsManager::readKey2(const QString &group, const QString &subGroup, const QString &key, const QString &defaultVal) const {
    QStringList args = {"--file", CONFIG_FILE, "--group", group};
    if (!subGroup.isEmpty()) args << "--group" << subGroup;
    args << "--key" << key;
    QProcess proc;
    proc.start("kreadconfig6", args);
    proc.waitForFinished(2000);
    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    return out.isEmpty() ? defaultVal : out;
}

// KConfig's own file-change watcher (used by the generated Settings/DoNotDisturbSettings
// classes in libnotificationmanager) picks up kwriteconfig6 writes live — no dbus poke needed,
// same as AppearanceManager's plain kwriteconfig6 calls for kdeglobals.
void NotificationsManager::writeKey(const QString &group, const QString &subGroup, const QString &key, const QString &value) const {
    QStringList args = {"--file", CONFIG_FILE, "--group", group};
    if (!subGroup.isEmpty()) args << "--group" << subGroup;
    args << "--key" << key << value;
    QProcess::startDetached("kwriteconfig6", args);
}

void NotificationsManager::refresh() {
    m_lowPriorityPopups  = readKey("Notifications", "LowPriorityPopups", "true") == "true";
    m_lowPriorityHistory = readKey("Notifications", "LowPriorityHistory", "false") == "true";
    m_popupPosition      = readKey("Notifications", "PopupPosition", "0").toInt();
    m_popupTimeout        = readKey("Notifications", "PopupTimeout", "5000").toInt();
    m_showPopupTimeout   = readKey("Notifications", "ShowPopupTimeout", "true") == "true";

    // Discover per-app entries directly from the ini — kreadconfig6 has no "list groups" verb.
    run(QString("awk -F'[][]' '/^\\[Applications\\]\\[/{print $4}' \"$HOME/.config/%1\" 2>/dev/null | sort -u")
            .arg(CONFIG_FILE),
        [this](QString out) {
            QVariantList apps;
            for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
                QString id = line.trimmed();
                if (id.isEmpty()) continue;
                QVariantMap entry;
                entry["id"]            = id;
                entry["label"]         = id;
                entry["showPopups"]    = readKey2("Applications", id, "ShowPopups", "true") == "true";
                entry["showInHistory"] = readKey2("Applications", id, "ShowInHistory", "true") == "true";
                apps.append(entry);
            }
            m_appList = apps;
            emit changed();
        });

    emit changed();
}

void NotificationsManager::setLowPriorityPopups(bool v) {
    m_lowPriorityPopups = v; emit changed();
    writeKey("Notifications", {}, "LowPriorityPopups", v ? "true" : "false");
}

void NotificationsManager::setLowPriorityHistory(bool v) {
    m_lowPriorityHistory = v; emit changed();
    writeKey("Notifications", {}, "LowPriorityHistory", v ? "true" : "false");
}

void NotificationsManager::setPopupPosition(int v) {
    m_popupPosition = v; emit changed();
    writeKey("Notifications", {}, "PopupPosition", QString::number(v));
}

void NotificationsManager::setPopupTimeout(int ms) {
    m_popupTimeout = ms; emit changed();
    writeKey("Notifications", {}, "PopupTimeout", QString::number(ms));
}

void NotificationsManager::setShowPopupTimeout(bool v) {
    m_showPopupTimeout = v; emit changed();
    writeKey("Notifications", {}, "ShowPopupTimeout", v ? "true" : "false");
}

void NotificationsManager::setAppShowPopups(const QString &id, bool v) {
    for (auto &entry : m_appList) {
        QVariantMap m = entry.toMap();
        if (m["id"].toString() == id) { m["showPopups"] = v; entry = m; break; }
    }
    emit changed();
    writeKey("Applications", id, "ShowPopups", v ? "true" : "false");
}

void NotificationsManager::setAppShowInHistory(const QString &id, bool v) {
    for (auto &entry : m_appList) {
        QVariantMap m = entry.toMap();
        if (m["id"].toString() == id) { m["showInHistory"] = v; entry = m; break; }
    }
    emit changed();
    writeKey("Applications", id, "ShowInHistory", v ? "true" : "false");
}
