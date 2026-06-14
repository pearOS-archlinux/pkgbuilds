#include "startupmanager.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

StartupManager::StartupManager(QObject *parent) : QObject(parent) {}

void StartupManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void StartupManager::refresh() {
    m_apps.clear();
    QString autostartDir = QDir::homePath() + "/.config/autostart";
    QDir dir(autostartDir);
    for (const QFileInfo &fi : dir.entryInfoList({"*.desktop"}, QDir::Files)) {
        QSettings ini(fi.absoluteFilePath(), QSettings::IniFormat);
        ini.beginGroup("Desktop Entry");
        QVariantMap app;
        app["name"]    = ini.value("Name", fi.baseName()).toString();
        app["exec"]    = ini.value("Exec").toString();
        app["comment"] = ini.value("Comment").toString();
        app["file"]    = fi.absoluteFilePath();
        app["enabled"] = ini.value("Hidden", "false").toString().toLower() != "true"
                      && ini.value("X-GNOME-Autostart-enabled", "true").toString().toLower() != "false";
        ini.endGroup();
        m_apps.append(app);
    }
    emit appsChanged();
}

void StartupManager::setEnabled(const QString &name, bool enabled) {
    for (const QVariant &v : m_apps) {
        QVariantMap app = v.toMap();
        if (app["name"] == name) {
            QSettings ini(app["file"].toString(), QSettings::IniFormat);
            ini.beginGroup("Desktop Entry");
            ini.setValue("X-GNOME-Autostart-enabled", enabled ? "true" : "false");
            ini.endGroup();
            break;
        }
    }
    refresh();
}

void StartupManager::removeApp(const QString &name) {
    for (const QVariant &v : m_apps) {
        QVariantMap app = v.toMap();
        if (app["name"] == name) {
            QFile::remove(app["file"].toString());
            break;
        }
    }
    refresh();
}
