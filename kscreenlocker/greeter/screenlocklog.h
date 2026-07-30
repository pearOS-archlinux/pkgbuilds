/*
    pearOS screenlock diagnostic logging.
    Appends unlock-flow events to /usr/share/extras/logs/screenlock.log so
    "Unlocking failed" reports can be diagnosed without needing to attach
    a debugger or enable qCDebug categories on affected user machines.
*/
#pragma once

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

inline void screenlockLog(const QString &msg)
{
    static const QString logDir = QStringLiteral("/usr/share/extras/logs");
    static const QString logPath = logDir + QStringLiteral("/screenlock.log");

    QDir().mkpath(logDir);

    QFile f(logPath);
    if (!f.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&f);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " [" << qEnvironmentVariable("USER") << "] " << msg << '\n';
}
