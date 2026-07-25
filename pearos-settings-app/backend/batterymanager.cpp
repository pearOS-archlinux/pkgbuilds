#include "batterymanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QMap>
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

void BatteryManager::findDevicePath() {
    // Re-resolve every refresh — cheap, and survives battery_BAT0 disappearing/
    // reappearing (e.g. hot-swap, suspend/resume edge cases on some laptops).
    QProcess p;
    p.start("bash", {"-c", "upower -e 2>/dev/null | grep -i battery | head -1"});
    p.waitForFinished(2000);
    m_devicePath = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
}

void BatteryManager::refresh() {
    findDevicePath();
    if (m_devicePath.isEmpty()) {
        m_present = false;
        emit statusChanged();
        return;
    }
    m_present = true;

    run(QString("qdbus6 --system org.freedesktop.UPower '%1' "
                "org.freedesktop.DBus.Properties.GetAll org.freedesktop.UPower.Device 2>/dev/null")
            .arg(m_devicePath),
        [this](QString out) {
            QMap<QString, QString> props;
            for (const QString &line : out.split('\n')) {
                int sep = line.indexOf(':');
                if (sep < 0) continue;
                props[line.left(sep).trimmed()] = line.mid(sep + 1).trimmed();
            }
            if (props.isEmpty()) { emit statusChanged(); return; }

            // UPower State enum: 1=Charging, 2=Discharging, 3=Empty, 4=FullyCharged,
            // 5=PendingCharge, 6=PendingDischarge
            int state = props.value("State", "0").toInt();
            m_charging = (state == 1 || state == 4);

            m_percentage = qRound(props.value("Percentage", "0").toDouble());

            double energyFull       = props.value("EnergyFull", "0").toDouble();
            double energyFullDesign = props.value("EnergyFullDesign", "0").toDouble();
            m_currentCapacity = qRound(energyFull * 1000);        // Wh -> mWh
            m_designCapacity  = qRound(energyFullDesign * 1000);

            if (m_designCapacity > 0 && m_currentCapacity > 0) {
                m_healthPercent = qMin(100, qRound(100.0 * m_currentCapacity / m_designCapacity));
                m_healthStatus  = m_healthPercent >= 80 ? "Good" : m_healthPercent >= 60 ? "Fair" : "Poor";
            } else {
                m_healthPercent = 0;
            }

            bool ok;
            int cycles = props.value("ChargeCycles", "-1").toInt(&ok);
            m_cycleCount = (ok && cycles >= 0) ? cycles : -1;

            m_chargeThresholdSupported = props.value("ChargeThresholdSupported") == "true";
            m_optimizedChargingEnabled = props.value("ChargeThresholdEnabled") == "true";

            qint64 timeToEmpty = props.value("TimeToEmpty", "0").toLongLong();
            qint64 timeToFull  = props.value("TimeToFull", "0").toLongLong();
            qint64 secs = m_charging ? timeToFull : timeToEmpty;
            if (secs > 0) {
                int h = secs / 3600, mnt = (secs % 3600) / 60;
                m_timeRemaining = h > 0 ? QString("%1:%2").arg(h).arg(mnt, 2, 10, QChar('0'))
                                         : QString("%1 min").arg(mnt);
            } else {
                m_timeRemaining.clear();
            }

            emit statusChanged();
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

void BatteryManager::setOptimizedChargingEnabled(bool enabled) {
    if (!m_chargeThresholdSupported || m_devicePath.isEmpty()) return;
    m_optimizedChargingEnabled = enabled;
    emit statusChanged();
    // UPower owns the hardware-specific sysfs knob (charge_control_end_threshold,
    // charge_stop_threshold, etc. depending on driver) behind this one generic
    // call, and already holds the polkit rule needed to write it.
    run(QString("qdbus6 --system org.freedesktop.UPower '%1' "
                "org.freedesktop.UPower.Device.EnableChargeThreshold %2 2>/dev/null")
            .arg(m_devicePath, enabled ? "true" : "false"),
        [this](QString) { refresh(); });
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
