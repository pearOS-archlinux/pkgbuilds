#include "systeminfo.h"
#include <QProcess>
#include <QFile>
#include <QRegularExpression>

SystemInfo::SystemInfo(QObject *parent) : QObject(parent) {}

void SystemInfo::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void SystemInfo::refresh() {
    run("cat /etc/os-release 2>/dev/null", [this](QString out) {
        static QRegularExpression nameRe("^PRETTY_NAME=\"(.+)\"", QRegularExpression::MultilineOption);
        static QRegularExpression versionRe("^VERSION=\"(.+)\"", QRegularExpression::MultilineOption);
        auto nm = nameRe.match(out);
        auto vm = versionRe.match(out);
        m_osName    = nm.hasMatch() ? nm.captured(1) : "Linux";
        m_osVersion = vm.hasMatch() ? vm.captured(1) : QString();
    });

    run("uname -r", [this](QString out) { m_kernelVersion = out.trimmed(); });
    run("hostname", [this](QString out) { m_hostName = out.trimmed(); });

    run("grep -m1 'model name' /proc/cpuinfo | cut -d: -f2", [this](QString out) {
        m_cpuModel = out.trimmed();
    });
    run("nproc", [this](QString out) { m_cpuCores = out.trimmed().toInt(); });

    run("lspci 2>/dev/null | grep -i 'vga\\|3d\\|display' | head -1 | sed 's/.*: //'", [this](QString out) {
        m_gpuModel = out.trimmed();
    });

    run("free -b | awk 'NR==2{print $2\" \"$3}'", [this](QString out) {
        QStringList parts = out.trimmed().split(' ');
        m_totalRam = parts.value(0).toLongLong();
        m_usedRam  = parts.value(1).toLongLong();
    });

    run("df -B1 / | awk 'NR==2{print $2\" \"$3}'", [this](QString out) {
        QStringList parts = out.trimmed().split(' ');
        m_totalDisk = parts.value(0).toLongLong();
        m_usedDisk  = parts.value(1).toLongLong();
    });

    // Get filesystem label for root partition (disk name shown in Storage page)
    run("findmnt -n -o SOURCE / 2>/dev/null | head -1", [this](QString out) {
        QString dev = out.trimmed();
        if (dev.isEmpty()) { m_diskName = "Main Storage"; return; }
        run(QString("lsblk -no LABEL %1 2>/dev/null | head -1").arg(dev), [this, dev](QString label) {
            QString l = label.trimmed();
            if (!l.isEmpty()) { m_diskName = l; return; }
            // Fall back to device basename
            m_diskName = dev.section('/', -1);
        });
    });

    run("dmidecode -t system 2>/dev/null | grep 'Serial Number:' | head -1 | cut -d: -f2", [this](QString out) {
        m_serialNumber = out.trimmed();
        emit infoChanged();
    });
}

void SystemInfo::refreshStorage() {
    QString home = qgetenv("HOME");
    // Applications
    run("du -sb /usr/share/applications /opt 2>/dev/null | awk '{sum+=$1} END {printf \"%.3f\", sum/1073741824}'", [this](QString out) {
        m_storageApps = out.trimmed().toDouble();
        emit storageChanged();
    });
    // Documents
    run(QString("du -sb \"%1/Documents\" 2>/dev/null | awk '{printf \"%.3f\", $1/1073741824}'").arg(home), [this](QString out) {
        m_storageDocs = out.trimmed().toDouble();
        emit storageChanged();
    });
    // Photos
    run(QString("du -sb \"%1/Pictures\" 2>/dev/null | awk '{printf \"%.3f\", $1/1073741824}'").arg(home), [this](QString out) {
        m_storagePhotos = out.trimmed().toDouble();
        emit storageChanged();
    });
    // Downloads
    run(QString("du -sb \"%1/Downloads\" 2>/dev/null | awk '{printf \"%.3f\", $1/1073741824}'").arg(home), [this](QString out) {
        m_storageDownloads = out.trimmed().toDouble();
        emit storageChanged();
    });
    // Desktop
    run(QString("du -sb \"%1/Desktop\" 2>/dev/null | awk '{printf \"%.3f\", $1/1073741824}'").arg(home), [this](QString out) {
        m_storageDesktop = out.trimmed().toDouble();
        emit storageChanged();
    });
    // System
    run("du -sb /var /etc /boot /usr/lib /usr/share 2>/dev/null | awk '{sum+=$1} END {printf \"%.3f\", sum/1073741824}'", [this](QString out) {
        m_storageSystem = out.trimmed().toDouble();
        emit storageChanged();
    });
}
