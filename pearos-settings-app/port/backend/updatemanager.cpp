#include "updatemanager.h"
#include <QProcess>

UpdateManager::UpdateManager(QObject *parent) : QObject(parent) {}

void UpdateManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void UpdateManager::runStreaming(const QString &cmd) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc]() {
        m_output += QString::fromUtf8(proc->readAllStandardOutput());
        emit outputChanged();
    });
    connect(proc, &QProcess::finished, this, [this, proc](int, QProcess::ExitStatus) {
        m_updating = false;
        emit stateChanged();
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, this, [this, proc](QProcess::ProcessError) {
        m_updating = false;
        emit stateChanged();
        proc->deleteLater();
    });
    proc->start("bash", {"-c", cmd});
}

void UpdateManager::checkUpdates() {
    if (m_checking) return;
    m_checking = true;
    m_updates.clear();
    emit stateChanged();

    run("checkupdates 2>/dev/null || pacman -Qu 2>/dev/null", [this](QString out) {
        m_checking = false;
        for (const QString &line : out.split('\n')) {
            QString l = line.trimmed();
            if (l.isEmpty()) continue;
            QStringList parts = l.split(' ', Qt::SkipEmptyParts);
            QVariantMap pkg;
            pkg["name"]    = parts.value(0);
            pkg["current"] = parts.value(1);
            pkg["latest"]  = parts.value(3);
            m_updates.append(pkg);
        }
        emit stateChanged();
    });
}

void UpdateManager::applyUpdates() {
    if (m_updating) return;
    m_updating = true;
    m_output.clear();
    emit stateChanged();
    runStreaming("pkexec pacman -Syu --noconfirm 2>&1");
}
