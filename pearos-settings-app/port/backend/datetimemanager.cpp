#include "datetimemanager.h"
#include <QProcess>
#include <QRegularExpression>

DateTimeManager::DateTimeManager(QObject *parent) : QObject(parent) {}

void DateTimeManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void DateTimeManager::refresh() {
    run("timedatectl show 2>/dev/null", [this](QString out) {
        static QRegularExpression tzRe("^Timezone=(.+)", QRegularExpression::MultilineOption);
        static QRegularExpression ntpRe("^NTP=(.+)", QRegularExpression::MultilineOption);
        auto tm = tzRe.match(out); m_timezone = tm.hasMatch() ? tm.captured(1).trimmed() : "UTC";
        auto nm = ntpRe.match(out); m_autoTime = !nm.hasMatch() || nm.captured(1).trimmed() == "yes";
    });

    run("kreadconfig5 --group 'Locale' --key 'TimeFormat' 2>/dev/null || echo '%H:%M'", [this](QString out) {
        m_use24h = out.trimmed().contains("%H");
    });

    run("timedatectl list-timezones 2>/dev/null", [this](QString out) {
        m_timezones.clear();
        for (const QString &tz : out.split('\n')) {
            if (!tz.trimmed().isEmpty()) m_timezones.append(tz.trimmed());
        }
        emit settingsChanged();
    });
}

void DateTimeManager::setAutoTime(bool enabled) {
    m_autoTime = enabled;
    run(QString("pkexec timedatectl set-ntp %1").arg(enabled ? "true" : "false"),
        [this](QString) { emit settingsChanged(); });
}

void DateTimeManager::setTimezone(const QString &tz) {
    m_timezone = tz;
    auto *proc = new QProcess(this);
    proc->start("pkexec", {"timedatectl", "set-timezone", tz});
    connect(proc, &QProcess::finished, this, [this, proc](int, QProcess::ExitStatus) {
        emit settingsChanged(); proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
}

void DateTimeManager::setUse24h(bool enabled) {
    m_use24h = enabled;
    QString fmt = enabled ? "%H:%M:%S" : "%I:%M:%S %p";
    run(QString("kwriteconfig5 --group Locale --key TimeFormat '%1'").arg(fmt),
        [this](QString) { emit settingsChanged(); });
}

void DateTimeManager::setDateTime(const QString &isoString) {
    auto *proc = new QProcess(this);
    proc->start("pkexec", {"timedatectl", "set-time", isoString});
    connect(proc, &QProcess::finished, this, [this, proc](int, QProcess::ExitStatus) {
        emit settingsChanged(); proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
}
