#include "printersmanager.h"
#include <QProcess>
#include <QRegularExpression>
#include <QVariantMap>

PrintersManager::PrintersManager(QObject *parent) : QObject(parent) {}

void PrintersManager::run(const QString &cmd, std::function<void(QString, int)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int code, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()), code);
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, this, [proc, cb](QProcess::ProcessError) {
        cb(QString(), -1);
        proc->deleteLater();
    });
    proc->start("bash", {"-c", cmd});
}

void PrintersManager::refresh() {
    run("lpstat -p -d 2>&1", [this](QString out, int) {
        QVariantList printers;
        QString defaultPrinter;

        static QRegularExpression idleRe(R"(^printer\s+(\S+)\s+is\s+([^.]+)\.)");
        static QRegularExpression disabledRe(R"(^printer\s+(\S+)\s+disabled\s+since)");
        static QRegularExpression defaultRe(R"(^system default destination:\s*(\S+))");

        QMap<QString, QVariantMap> byName;
        QStringList order;

        for (const QString &line : out.split('\n')) {
            QString l = line.trimmed();
            if (auto m = idleRe.match(l); m.hasMatch()) {
                QString name = m.captured(1);
                QVariantMap entry;
                entry["name"] = name;
                entry["status"] = m.captured(2).trimmed();
                entry["enabled"] = true;
                byName[name] = entry;
                order << name;
            } else if (auto m = disabledRe.match(l); m.hasMatch()) {
                QString name = m.captured(1);
                QVariantMap entry;
                entry["name"] = name;
                entry["status"] = "disabled";
                entry["enabled"] = false;
                byName[name] = entry;
                order << name;
            } else if (auto m = defaultRe.match(l); m.hasMatch()) {
                defaultPrinter = m.captured(1);
            }
        }

        for (const QString &name : order) {
            QVariantMap entry = byName[name];
            entry["isDefault"] = (name == defaultPrinter);
            printers.append(entry);
        }

        m_printers = printers;
        m_defaultPrinter = defaultPrinter;
        emit changed();
    });
}

void PrintersManager::refreshJobs() {
    run("lpstat -o 2>&1", [this](QString out, int) {
        QVariantList jobs;
        // Format: "<printer>-<jobid> <user> <size> <date...>"
        static QRegularExpression jobRe(R"(^(\S+)-(\d+)\s+(\S+)\s+(\d+)\s+(.*)$)");
        for (const QString &line : out.split('\n')) {
            QString l = line.trimmed();
            auto m = jobRe.match(l);
            if (!m.hasMatch()) continue;
            QVariantMap entry;
            entry["id"]      = m.captured(1) + "-" + m.captured(2);
            entry["printer"] = m.captured(1);
            entry["user"]    = m.captured(3);
            entry["date"]    = m.captured(5).trimmed();
            jobs.append(entry);
        }
        m_jobs = jobs;
        emit jobsChanged();
    });
}

void PrintersManager::setDefaultPrinter(const QString &name) {
    run(QString("pkexec lpadmin -d '%1' 2>&1").arg(name), [this, name](QString out, int code) {
        bool ok = (code == 0);
        if (ok) { m_defaultPrinter = name; emit changed(); }
        emit actionResult(ok, ok ? QString() : out.trimmed());
        refresh();
    });
}

void PrintersManager::setPrinterEnabled(const QString &name, bool enabled) {
    QString cmd = enabled ? "cupsenable" : "cupsdisable";
    run(QString("pkexec %1 '%2' 2>&1").arg(cmd, name), [this](QString out, int code) {
        bool ok = (code == 0);
        emit actionResult(ok, ok ? QString() : out.trimmed());
        refresh();
    });
}

void PrintersManager::removePrinter(const QString &name) {
    run(QString("pkexec lpadmin -x '%1' 2>&1").arg(name), [this](QString out, int code) {
        bool ok = (code == 0);
        emit actionResult(ok, ok ? QString() : out.trimmed());
        refresh();
    });
}

void PrintersManager::addPrinterByUri(const QString &name, const QString &uri) {
    QString safeName = name; safeName.remove(QRegularExpression("[^A-Za-z0-9_-]"));
    if (safeName.isEmpty()) { emit actionResult(false, "Invalid printer name"); return; }
    // -m everywhere: driverless IPP Everywhere — works for the large majority
    // of network printers/scanners made in the last ~decade without a PPD.
    run(QString("pkexec lpadmin -p '%1' -E -v '%2' -m everywhere 2>&1").arg(safeName, uri),
        [this](QString out, int code) {
            bool ok = (code == 0);
            emit actionResult(ok, ok ? QString() : out.trimmed());
            refresh();
        });
}

void PrintersManager::cancelJob(const QString &jobId) {
    run(QString("cancel '%1' 2>&1").arg(jobId), [this](QString out, int code) {
        bool ok = (code == 0);
        emit actionResult(ok, ok ? QString() : out.trimmed());
        refreshJobs();
    });
}
