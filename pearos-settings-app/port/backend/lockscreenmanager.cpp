#include "lockscreenmanager.h"
#include <QProcess>
#include <QRegularExpression>

LockScreenManager::LockScreenManager(QObject *parent) : QObject(parent) {}

void LockScreenManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void LockScreenManager::refresh() {
    run("kreadconfig5 --file kscreenlockerrc --group Daemon --key Autolock 2>/dev/null", [this](QString out) {
        m_autoLock = out.trimmed() != "false";
    });
    run("kreadconfig5 --file kscreenlockerrc --group Daemon --key Timeout 2>/dev/null", [this](QString out) {
        bool ok; int v = out.trimmed().toInt(&ok);
        m_lockTimeout = ok ? v : 5;
    });
    run("kreadconfig5 --file kscreenlockerrc --group Greeter --key RequirePassword --default true 2>/dev/null",
        [this](QString out) {
            m_requirePassword = out.trimmed() != "false";
            emit settingsChanged();
        });
}

void LockScreenManager::setAutoLock(bool enabled) {
    m_autoLock = enabled;
    run(QString("kwriteconfig5 --file kscreenlockerrc --group Daemon --key Autolock %1").arg(enabled ? "true" : "false"),
        [this](QString) { emit settingsChanged(); });
}

void LockScreenManager::setLockTimeout(int minutes) {
    m_lockTimeout = minutes;
    run(QString("kwriteconfig5 --file kscreenlockerrc --group Daemon --key Timeout %1").arg(minutes),
        [this](QString) { emit settingsChanged(); });
}

void LockScreenManager::setRequirePassword(bool enabled) {
    m_requirePassword = enabled;
    run(QString("kwriteconfig5 --file kscreenlockerrc --group Greeter --key RequirePassword %1")
            .arg(enabled ? "true" : "false"),
        [this](QString) { emit settingsChanged(); });
}

void LockScreenManager::lockNow() {
    run("loginctl lock-session 2>/dev/null || "
        "qdbus org.freedesktop.ScreenSaver /ScreenSaver Lock 2>/dev/null", [](QString) {});
}
