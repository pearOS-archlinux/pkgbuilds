#include "batterymanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <algorithm>

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

        // cycle count
        QString cycleStr = readFile(base + "cycle_count");
        m_cycleCount = cycleStr.isEmpty() ? -1 : cycleStr.toInt();

        // health from sysfs (Good / Overheat / etc.)
        QString sysHealth = readFile(base + "health");
        if (!sysHealth.isEmpty()) {
            m_healthStatus = sysHealth;
        } else if (m_designCapacity > 0 && m_currentCapacity > 0) {
            double pct = 100.0 * m_currentCapacity / m_designCapacity;
            m_healthStatus = pct >= 80 ? "Good" : pct >= 60 ? "Fair" : "Poor";
        }

        run(QString("upower -i $(upower -e | grep -i bat | head -1) 2>/dev/null"
                    " | grep 'time to' | head -1 | awk '{print $NF}'"), [this](QString t) {
            m_timeRemaining = t.trimmed();
            emit statusChanged();
        });
    });

    // low power mode via powerprofilesctl or CPU governor
    run("powerprofilesctl get 2>/dev/null || cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null",
        [this](QString out) {
        QString s = out.trimmed();
        m_lowPowerMode = s.contains("power-saver") || s == "powersave";
    });
}

void BatteryManager::setLowPowerMode(bool enabled) {
    m_lowPowerMode = enabled;
    emit statusChanged();
    QString cmd = enabled
        ? "powerprofilesctl set power-saver 2>/dev/null || "
          "for f in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do echo powersave | pkexec tee \"$f\" >/dev/null 2>&1; done"
        : "powerprofilesctl set balanced 2>/dev/null || "
          "for f in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do echo ondemand | pkexec tee \"$f\" >/dev/null 2>&1; done";
    run(cmd, [](QString) {});
}

void BatteryManager::refreshHistory() {
    run("ls /var/lib/upower/history-charge-BAT*.dat 2>/dev/null | head -1", [this](QString out) {
        QString path = out.trimmed();
        QVariantList result;

        if (!path.isEmpty()) {
            QFile f(path);
            if (f.open(QIODevice::ReadOnly)) {
                for (const QByteArray &rawLine : f.readAll().split('\n')) {
                    QString line = QString::fromUtf8(rawLine).trimmed();
                    if (line.isEmpty()) continue;
                    QStringList parts = line.split('\t');
                    if (parts.size() < 3) continue;

                    qint64 ts  = parts[0].toLongLong();
                    double val = QString(parts[1]).replace(',', '.').toDouble();
                    QString st = parts[2].trimmed();

                    // Skip upower daemon restart markers (value=0, state=unknown)
                    if (val < 0.1 && st == "unknown") continue;

                    QVariantMap entry;
                    entry["ts"]    = ts;
                    entry["pct"]   = val;
                    entry["state"] = st;
                    result.append(entry);
                }
            }
        }

        m_chargeHistory = result;
        emit historyChanged();
    });
}

void BatteryManager::refreshUsage() {
    run("ps aux --sort=-%cpu --no-headers 2>/dev/null", [this](QString out) {
        QMap<QString, double> cpuMap;

        for (const QString &line : out.split('\n')) {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() < 11) continue;
            double cpu = parts[2].toDouble();
            if (cpu < 0.05) continue;

            QString cmd = parts.mid(10).join(' ');
            QString name = cmd.section('/', -1).section(' ', 0, 0);
            // Strip kernel thread brackets
            if (name.startsWith('[') && name.endsWith(']'))
                name = name.mid(1, name.length() - 2);
            if (name.isEmpty()) name = cmd;

            cpuMap[name] += cpu;
        }

        QList<QPair<double, QString>> sorted;
        for (auto it = cpuMap.cbegin(); it != cpuMap.cend(); ++it)
            sorted.append({it.value(), it.key()});
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto &a, const auto &b) { return a.first > b.first; });

        double maxCpu = sorted.isEmpty() ? 1.0 : sorted.first().first;
        QVariantList result;
        for (int i = 0; i < qMin(10, sorted.size()); ++i) {
            if (sorted[i].first < 0.1) break;
            QVariantMap m;
            m["name"] = sorted[i].second;
            m["cpu"]  = QString::number(sorted[i].first, 'f', 1);
            m["rel"]  = sorted[i].first / maxCpu;
            result.append(m);
        }

        m_appUsage = result;
        emit usageChanged();
    });
}
