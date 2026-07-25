#include "scrollbarmanager.h"
#include <QProcess>

ScrollbarManager::ScrollbarManager(QObject *parent) : QObject(parent) {}

void ScrollbarManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void ScrollbarManager::writeGtkKey(const QString &key, const QString &value) const {
    QProcess::startDetached("kwriteconfig6", {"--file", "gtk-3.0/settings.ini", "--group", "Settings", "--key", key, value});
    QProcess::startDetached("kwriteconfig6", {"--file", "gtk-4.0/settings.ini", "--group", "Settings", "--key", key, value});
}

void ScrollbarManager::refresh() {
    run("kreadconfig6 --file gtk-3.0/settings.ini --group Settings --key gtk-overlay-scrolling --default true",
        [this](QString out) {
            m_alwaysVisible = out.trimmed() == "false";
            emit changed();
        });
    run("kreadconfig6 --file gtk-3.0/settings.ini --group Settings --key gtk-primary-button-warps-slider --default true",
        [this](QString out) {
            m_clickToJump = out.trimmed() == "true";
            emit changed();
        });
}

void ScrollbarManager::setAlwaysVisible(bool v) {
    m_alwaysVisible = v;
    emit changed();
    // overlay-scrolling is the inverse of "always visible" (overlay = auto-hide)
    writeGtkKey("gtk-overlay-scrolling", v ? "false" : "true");
    // gsettings backs the same property for GTK apps that read the schema
    // directly (dconf) instead of settings.ini, and broadcasts live.
    run(QString("gsettings set org.gnome.desktop.interface overlay-scrolling %1")
            .arg(v ? "false" : "true"), [](QString) {});
}

void ScrollbarManager::setClickToJump(bool v) {
    m_clickToJump = v;
    emit changed();
    writeGtkKey("gtk-primary-button-warps-slider", v ? "true" : "false");
}
