#include "batterymanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QRegularExpression>

BatteryManager::BatteryManager(QObject *parent) : QObject(parent) {}

void BatteryManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void BatteryManager::refresh() {
    // find battery in /sys/class/power_supply
    run("ls /sys/class/power_supply/ 2>/dev/null", [this](QString out) {
        QString bat;
        for (const QString &name : out.split('\n')) {
            QString n = name.trimmed();
            if (n.startsWith("BAT") || n.startsWith("bat")) { bat = n; break; }
        }
        if (bat.isEmpty()) { m_present = false; emit statusChanged(); return; }
        m_present = true;

        QString base = "/sys/class/power_supply/" + bat + "/";
        auto readFile = [](const QString &path) -> QString {
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) return {};
            return QString::fromUtf8(f.readAll()).trimmed();
        };

        QString status = readFile(base + "status");
        m_charging = status == "Charging" || status == "Full";

        QString capStr = readFile(base + "capacity");
        m_percentage = capStr.isEmpty() ? 0 : capStr.toInt();

        QString designStr = readFile(base + "energy_full_design");
        QString fullStr   = readFile(base + "energy_full");
        m_designCapacity   = designStr.isEmpty() ? 0 : designStr.toInt() / 1000;
        m_currentCapacity  = fullStr.isEmpty()   ? 0 : fullStr.toInt()   / 1000;

        run(QString("upower -i $(upower -e | grep -i bat | head -1) 2>/dev/null"
                    " | grep 'time to' | head -1 | awk '{print $NF}'"), [this](QString t) {
            m_timeRemaining = t.trimmed();
            emit statusChanged();
        });
    });
}
