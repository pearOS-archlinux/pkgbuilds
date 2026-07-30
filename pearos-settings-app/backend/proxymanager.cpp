#include "proxymanager.h"
#include <QProcess>

ProxyManager::ProxyManager(QObject *parent) : QObject(parent) {}

void ProxyManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void ProxyManager::refresh() {
    run("kreadconfig6 --file kioslaverc --group 'Proxy Settings' --key ProxyType --default 0",
        [this](QString out) { m_enabled = out.trimmed() == "1"; emit changed(); });

    run("kreadconfig6 --file kioslaverc --group 'Proxy Settings' --key httpProxy --default ''",
        [this](QString out) {
        // Stored as "http://host port"
        QString v = out.trimmed();
        if (v.startsWith("http://")) v = v.mid(7);
        int sp = v.lastIndexOf(' ');
        if (sp > 0) {
            m_host = v.left(sp);
            m_port = v.mid(sp + 1).toInt();
        } else if (!v.isEmpty()) {
            m_host = v;
        }
        emit changed();
    });
}

void ProxyManager::apply() {
    QString value = m_enabled ? QString("http://%1 %2").arg(m_host).arg(m_port) : QString();
    run(QString("kwriteconfig6 --file kioslaverc --group 'Proxy Settings' --key ProxyType %1 && "
                "kwriteconfig6 --file kioslaverc --group 'Proxy Settings' --key httpProxy '%2' && "
                "kwriteconfig6 --file kioslaverc --group 'Proxy Settings' --key httpsProxy '%2' && "
                "kwriteconfig6 --file kioslaverc --group 'Proxy Settings' --key ftpProxy '%2'")
            .arg(m_enabled ? "1" : "0", value),
        [](QString) {});
}

void ProxyManager::setEnabled(bool enabled) {
    m_enabled = enabled;
    emit changed();
    apply();
}

void ProxyManager::setProxy(const QString &host, int port) {
    m_host = host;
    m_port = port;
    emit changed();
    if (m_enabled) apply();
}
