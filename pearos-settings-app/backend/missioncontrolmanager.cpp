#include "missioncontrolmanager.h"
#include <QProcess>

MissionControlManager::MissionControlManager(QObject *parent) : QObject(parent) {}

void MissionControlManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void MissionControlManager::reconfigure() {
    run("qdbus6 org.kde.KWin /KWin reconfigure 2>/dev/null", [](QString) {});
}

void MissionControlManager::refresh() {
    run("kreadconfig6 --file kwinrc --group Desktops --key Number --default 1", [this](QString out) {
        m_desktopCount = out.toInt(); if (m_desktopCount < 1) m_desktopCount = 1;
        emit changed();
    });
    run("kreadconfig6 --file kwinrc --group Desktops --key Rows --default 1", [this](QString out) {
        m_desktopRows = out.toInt(); if (m_desktopRows < 1) m_desktopRows = 1;
        emit changed();
    });
    run("kreadconfig6 --file kwinrc --group Windows --key RollOverDesktops --default false", [this](QString out) {
        m_wrapNavigation = out.trimmed() == "true";
        emit changed();
    });
    run("kreadconfig6 --file kwinrc --group Windows --key PerOutputVirtualDesktops --default false", [this](QString out) {
        m_separateSpacesPerDisplay = out.trimmed() == "true";
        emit changed();
    });
}

void MissionControlManager::setDesktopCount(int n) {
    if (n < 1) n = 1;
    m_desktopCount = n;
    emit changed();
    run(QString("kwriteconfig6 --file kwinrc --group Desktops --key Number %1").arg(n),
        [this](QString) { reconfigure(); });
}

void MissionControlManager::setDesktopRows(int rows) {
    if (rows < 1) rows = 1;
    m_desktopRows = rows;
    emit changed();
    run(QString("kwriteconfig6 --file kwinrc --group Desktops --key Rows %1").arg(rows),
        [this](QString) { reconfigure(); });
}

void MissionControlManager::setWrapNavigation(bool wrap) {
    m_wrapNavigation = wrap;
    emit changed();
    run(QString("kwriteconfig6 --file kwinrc --group Windows --key RollOverDesktops %1")
            .arg(wrap ? "true" : "false"),
        [this](QString) { reconfigure(); });
}

void MissionControlManager::setSeparateSpacesPerDisplay(bool sep) {
    m_separateSpacesPerDisplay = sep;
    emit changed();
    run(QString("kwriteconfig6 --file kwinrc --group Windows --key PerOutputVirtualDesktops %1")
            .arg(sep ? "true" : "false"),
        [this](QString) { reconfigure(); });
}
