#include "privacymanager.h"
#include <QProcess>
#include <QFile>

PrivacyManager::PrivacyManager(QObject *parent) : QObject(parent) {}

void PrivacyManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void PrivacyManager::refresh() {
    run("kreadconfig5 --group 'Privacy' --key 'RecentFilesEnabled' --default 'true'", [this](QString out) {
        m_recentFiles = out.trimmed() != "false";
    });
    run("kreadconfig5 --group 'Privacy' --key 'SendDiagnostics' --default 'false'", [this](QString out) {
        m_sendDiagnostics = out.trimmed() == "true";
        emit privacyChanged();
    });
}

void PrivacyManager::setSendDiagnostics(bool enabled) {
    m_sendDiagnostics = enabled;
    run(QString("kwriteconfig5 --group Privacy --key SendDiagnostics %1").arg(enabled ? "true" : "false"),
        [this](QString) { emit privacyChanged(); });
}

void PrivacyManager::setLocationServices(bool enabled) {
    m_locationServices = enabled;
    emit privacyChanged();
}

void PrivacyManager::setRecentFiles(bool enabled) {
    m_recentFiles = enabled;
    run(QString("kwriteconfig5 --group Privacy --key RecentFilesEnabled %1").arg(enabled ? "true" : "false"),
        [this](QString) { emit privacyChanged(); });
}

void PrivacyManager::clearRecentFiles() {
    run("rm -f ~/.local/share/recently-used.xbel 2>/dev/null", [this](QString) {
        run("qdbus org.kde.ActivityManager /ActivityManager/Resources/Scoring "
            "org.kde.ActivityManager.ResourcesScoring.DeleteEarlierStats '' 0 2>/dev/null",
            [](QString) {});
    });
}
