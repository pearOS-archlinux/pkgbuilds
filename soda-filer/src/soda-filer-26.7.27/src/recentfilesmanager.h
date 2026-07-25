/*

    Copyright (C) 2026 Filer contributors

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef FILER_RECENTFILESMANAGER_H
#define FILER_RECENTFILESMANAGER_H

#include <QString>
#include <QStringList>

namespace Filer {

// Tracks recently-opened files and keeps a directory of symlinks to them
// (recentDirPath()) in sync, so the existing FmFolder/FolderModel machinery
// can browse "Recents" like any other folder without needing a virtual-folder
// implementation of its own.
class RecentFilesManager {
public:
  static RecentFilesManager& instance();

  // Records path as most-recently-used and refreshes the on-disk directory.
  void addFile(const QString& path);

  void clear();

  const QStringList& files() const {
    return files_;
  }

  // Directory of symlinks to the recent files; this is what the sidebar's
  // "Recents" places item navigates to.
  QString recentDirPath() const;

private:
  RecentFilesManager();

  void load();
  void save();
  void regenerateDir();

  QStringList files_; // most-recently-used first, absolute paths
  QString listFilePath_;

  static const int kMaxEntries = 50;
};

}

#endif // FILER_RECENTFILESMANAGER_H
