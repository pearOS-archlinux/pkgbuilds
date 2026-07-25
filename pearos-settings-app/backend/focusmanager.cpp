#include "focusmanager.h"
#include <QProcess>
#include <QDateTime>

const QString FocusManager::CONFIG_FILE = QStringLiteral("plasmanotifyrc");

FocusManager::FocusManager(QObject *parent) : QObject(parent) {}

QString FocusManager::readKey(const QString &group, const QString &key, const QString &defaultVal) const {
    QProcess proc;
    proc.start("kreadconfig6", {"--file", CONFIG_FILE, "--group", group, "--key", key});
    proc.waitForFinished(2000);
    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    return out.isEmpty() ? defaultVal : out;
}

// Same KConfig live-reload mechanism the notifications applet's generated
// Settings/DoNotDisturbSettings classes rely on — no dbus poke required.
void FocusManager::writeKey(const QString &group, const QString &key, const QString &value) const {
    QProcess::startDetached("kwriteconfig6", {"--file", CONFIG_FILE, "--group", group, "--key", key, value});
}

void FocusManager::refresh() {
    QString until = readKey("DoNotDisturb", "Until");
    m_doNotDisturb = !until.isEmpty() && QDateTime::fromString(until, Qt::ISODate) > QDateTime::currentDateTime();

    m_allowCriticalInDnd       = readKey("Notifications", "CriticalInDndMode", "true") == "true";
    m_muteSoundsInDnd          = readKey("DoNotDisturb", "NotificationSoundsMuted", "false") == "true";
    m_inhibitWhenFullscreen    = readKey("DoNotDisturb", "WhenFullscreen", "true") == "true";
    m_inhibitWhenScreenSharing = readKey("DoNotDisturb", "WhenScreenSharing", "true") == "true";
    m_inhibitWhenMirrored      = readKey("DoNotDisturb", "WhenScreensMirrored", "true") == "true";

    emit changed();
}

void FocusManager::setDoNotDisturb(bool v) {
    m_doNotDisturb = v;
    emit changed();
    if (v) {
        // "Until turned off" is represented the same way the notifications
        // applet does it: inhibited until roughly a year from now.
        QString until = QDateTime::currentDateTime().addYears(1).toString(Qt::ISODate);
        writeKey("DoNotDisturb", "Until", until);
    } else {
        writeKey("DoNotDisturb", "Until", "");
    }
}

void FocusManager::setAllowCriticalInDnd(bool v) {
    m_allowCriticalInDnd = v; emit changed();
    writeKey("Notifications", "CriticalInDndMode", v ? "true" : "false");
}

void FocusManager::setMuteSoundsInDnd(bool v) {
    m_muteSoundsInDnd = v; emit changed();
    writeKey("DoNotDisturb", "NotificationSoundsMuted", v ? "true" : "false");
}

void FocusManager::setInhibitWhenFullscreen(bool v) {
    m_inhibitWhenFullscreen = v; emit changed();
    writeKey("DoNotDisturb", "WhenFullscreen", v ? "true" : "false");
}

void FocusManager::setInhibitWhenScreenSharing(bool v) {
    m_inhibitWhenScreenSharing = v; emit changed();
    writeKey("DoNotDisturb", "WhenScreenSharing", v ? "true" : "false");
}

void FocusManager::setInhibitWhenMirrored(bool v) {
    m_inhibitWhenMirrored = v; emit changed();
    writeKey("DoNotDisturb", "WhenScreensMirrored", v ? "true" : "false");
}
