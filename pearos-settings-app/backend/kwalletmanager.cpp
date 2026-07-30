#include "kwalletmanager.h"
#include <QProcess>

KWalletManager::KWalletManager(QObject *parent) : QObject(parent) {}

void KWalletManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void KWalletManager::refresh() {
    run("kreadconfig6 --file kwalletrc --group Wallet --key Enabled --default true",
        [this](QString out) { m_enabled = out.trimmed() != "false"; emit changed(); });
    run("kreadconfig6 --file kwalletrc --group Wallet --key 'Close When Idle' --default true",
        [this](QString out) { m_closeWhenIdle = out.trimmed() != "false"; emit changed(); });
    run("kreadconfig6 --file kwalletrc --group Wallet --key 'Idle Timeout' --default 10",
        [this](QString out) { m_idleTimeout = out.isEmpty() ? 10 : out.toInt(); emit changed(); });
    run("kreadconfig6 --file kwalletrc --group Wallet --key 'Launch Manager' --default true",
        [this](QString out) { m_launchManager = out.trimmed() != "false"; emit changed(); });
}

void KWalletManager::set(const QString &key, const QVariant &value) {
    QString kwalletKey;
    QString strVal = value.toBool() ? "true" : "false";

    if (key == "enabled")       { m_enabled       = value.toBool(); kwalletKey = "Enabled"; }
    else if (key == "closeWhenIdle") { m_closeWhenIdle = value.toBool(); kwalletKey = "Close When Idle"; }
    else if (key == "launchManager") { m_launchManager = value.toBool(); kwalletKey = "Launch Manager"; }
    else if (key == "idleTimeout")   { m_idleTimeout = value.toInt(); kwalletKey = "Idle Timeout"; strVal = QString::number(m_idleTimeout); }
    else return;

    emit changed();
    run(QString("kwriteconfig6 --file kwalletrc --group Wallet --key '%1' '%2'").arg(kwalletKey, strVal),
        [](QString) {});
}
