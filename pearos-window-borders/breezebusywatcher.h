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

#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>

class QTimer;

namespace Breeze
{

    /**
     * Tracks which windows currently run a foreground job.
     *
     * The decoration API exposes no process id, so the pids are pushed in by the
     * companion kwin script over d-bus, keyed by window class and caption. Whether
     * a given process is busy is then decided here, by walking /proc.
     */
    class BusyWatcher : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.pearos.WindowDecoration")

        public:

        static BusyWatcher *self();

        //* true when the window runs something in the foreground of its terminal
        bool isBusy(const QString &caption) const;

        public Q_SLOTS:

        //* called by the kwin script; newline separated "caption\x1fpid" records.
        //* a plain string is used because kwin's callDBus cannot send a string list
        void setWindowProcesses(const QString &entries);

        Q_SIGNALS:

        //* emitted whenever any window changed between busy and idle
        void busyChanged();

        private Q_SLOTS:

        void poll();

        private:

        explicit BusyWatcher();

        //* window key -> pid, as reported by the kwin script
        QHash<QString, int> m_pids;

        //* keys of the windows that are currently busy
        QSet<QString> m_busy;

        QTimer *m_timer;

    };

}
