/*
 * Copyright 2026 pearOS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "breezebusywatcher.h"

#include <QDBusConnection>
#include <QDir>
#include <QFile>
#include <QTimer>

namespace Breeze
{

    namespace
    {

        //* the fields of /proc/<pid>/stat that are needed here
        struct ProcessInfo
        {
            int parent = 0;
            int group = 0;
            int tty = 0;
            int foregroundGroup = 0;
        };

        //* read every process once, so that a whole tree costs a single scan
        QHash<int, ProcessInfo> readProcesses()
        {
            QHash<int, ProcessInfo> processes;

            const QStringList entries(QDir(QStringLiteral("/proc")).entryList(QDir::Dirs | QDir::NoDotAndDotDot));
            for (const QString &entry : entries)
            {
                bool isPid(false);
                const int pid(entry.toInt(&isPid));
                if (!isPid) continue;

                QFile file(QStringLiteral("/proc/") + entry + QStringLiteral("/stat"));
                if (!file.open(QIODevice::ReadOnly)) continue;

                const QByteArray content(file.readAll());

                // the command name is in parentheses and may itself contain spaces,
                // so the fields after it are counted from the last closing one
                const int commEnd(content.lastIndexOf(')'));
                if (commEnd < 0) continue;

                const QList<QByteArray> fields(content.mid(commEnd + 2).split(' '));
                if (fields.size() < 6) continue;

                ProcessInfo info;
                info.parent = fields.at(1).toInt();
                info.group = fields.at(2).toInt();
                info.tty = fields.at(4).toInt();
                info.foregroundGroup = fields.at(5).toInt();
                processes.insert(pid, info);
            }

            return processes;
        }

        /**
         * A terminal is busy when the foreground process group of its tty is not the
         * group of the shell itself, which is exactly how the shell knows it is
         * waiting for a command. Processes without a controlling terminal never
         * qualify, so ordinary applications with helper processes stay idle.
         */
        bool hasForegroundJob(int pid, const QHash<int, ProcessInfo> &processes)
        {
            QHash<int, QList<int>> children;
            for (auto iter = processes.constBegin(); iter != processes.constEnd(); ++iter)
                children[iter.value().parent].append(iter.key());

            QList<int> pending{pid};
            QSet<int> seen{pid};

            while (!pending.isEmpty())
            {
                const int current(pending.takeFirst());

                const auto iter = processes.constFind(current);
                if (iter != processes.constEnd())
                {
                    const ProcessInfo &info(iter.value());
                    if (info.tty != 0 && info.foregroundGroup > 0 && info.foregroundGroup != info.group)
                        return true;
                }

                for (int child : children.value(current))
                {
                    if (seen.contains(child)) continue;
                    seen.insert(child);
                    pending.append(child);
                }
            }

            return false;
        }

    }

    //__________________________________________________________________
    BusyWatcher::BusyWatcher()
        : QObject()
        , m_timer(new QTimer(this))
    {

        QDBusConnection connection(QDBusConnection::sessionBus());
        connection.registerService(QStringLiteral("org.kde.pearos.WindowDecoration"));
        connection.registerObject(QStringLiteral("/Busy"), this,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

        m_timer->setInterval(1000);
        connect(m_timer, &QTimer::timeout, this, &BusyWatcher::poll);

    }

    //__________________________________________________________________
    BusyWatcher *BusyWatcher::self()
    {
        static BusyWatcher *instance = new BusyWatcher();
        return instance;
    }

    //__________________________________________________________________
    void BusyWatcher::setWindowProcesses(const QString &entries)
    {

        m_pids.clear();

        const QStringList records(entries.split(QLatin1Char('\n'), Qt::SkipEmptyParts));
        for (const QString &record : records)
        {
            const QStringList parts(record.split(QLatin1Char('\x1f')));
            if (parts.size() != 2) continue;

            bool isPid(false);
            const int pid(parts.at(1).toInt(&isPid));
            if (!isPid || pid <= 0) continue;

            // keyed by caption: windowClass() from the decoration and resourceClass
            // from the script do not always spell the same thing
            m_pids.insert(parts.at(0), pid);
        }

        // nothing to watch, so nothing to poll for
        if (m_pids.isEmpty())
        {
            m_timer->stop();
            if (!m_busy.isEmpty())
            {
                m_busy.clear();
                Q_EMIT busyChanged();
            }
            return;
        }

        if (!m_timer->isActive()) m_timer->start();
        poll();

    }

    //__________________________________________________________________
    void BusyWatcher::poll()
    {

        const QHash<int, ProcessInfo> processes(readProcesses());

        QSet<QString> busy;
        for (auto iter = m_pids.constBegin(); iter != m_pids.constEnd(); ++iter)
        {
            if (hasForegroundJob(iter.value(), processes))
                busy.insert(iter.key());
        }

        if (busy == m_busy) return;

        m_busy = busy;
        Q_EMIT busyChanged();

    }

    //__________________________________________________________________
    bool BusyWatcher::isBusy(const QString &caption) const
    { return m_busy.contains(caption); }

}
