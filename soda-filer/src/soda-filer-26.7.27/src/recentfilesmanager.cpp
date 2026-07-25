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

#include "recentfilesmanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <ctime>
#include <fcntl.h>
#include <sys/stat.h>

namespace Filer {

RecentFilesManager& RecentFilesManager::instance() {
  static RecentFilesManager inst;
  return inst;
}

RecentFilesManager::RecentFilesManager() {
  QString configDir = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
  if(configDir.isEmpty())
    configDir = QDir::homePath() + "/.config";
  configDir += "/filer";
  QDir().mkpath(configDir);
  listFilePath_ = configDir + "/recently-used";

  load();
  regenerateDir();
}

QString RecentFilesManager::recentDirPath() const {
  QString cacheDir = QString::fromLocal8Bit(qgetenv("XDG_CACHE_HOME"));
  if(cacheDir.isEmpty())
    cacheDir = QDir::homePath() + "/.cache";
  return cacheDir + "/filer/recent";
}

void RecentFilesManager::load() {
  files_.clear();
  QFile f(listFilePath_);
  if(f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&f);
    while(!in.atEnd()) {
      QString line = in.readLine();
      if(!line.isEmpty() && QFileInfo::exists(line))
        files_.append(line);
    }
  }
}

void RecentFilesManager::save() {
  QFile f(listFilePath_);
  if(f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    QTextStream out(&f);
    for(const QString& path : files_)
      out << path << "\n";
  }
}

void RecentFilesManager::addFile(const QString& path) {
  if(path.isEmpty() || !QFileInfo::exists(path))
    return;
  files_.removeAll(path);
  files_.prepend(path);
  while(files_.size() > kMaxEntries)
    files_.removeLast();
  save();
  regenerateDir();
}

void RecentFilesManager::clear() {
  files_.clear();
  save();
  regenerateDir();
}

void RecentFilesManager::regenerateDir() {
  QString dirPath = recentDirPath();
  QDir dir(dirPath);
  if(dir.exists())
    dir.removeRecursively();
  QDir().mkpath(dirPath);

  // Newest entries get the newest mtime stamped on the symlink itself (not
  // its target), so sorting the folder view by "Date Modified" reflects
  // most-recently-used order.
  time_t now = time(nullptr);
  int index = 0;
  for(const QString& path : files_) {
    QFileInfo info(path);
    if(!info.exists())
      continue;

    QString linkName = info.fileName();
    QString linkPath = dir.filePath(linkName);
    int suffix = 2;
    while(QFileInfo(linkPath).exists() || QFileInfo::exists(linkPath)) {
      QString suffixedName = info.completeBaseName() + QString(" %1").arg(suffix);
      if(!info.suffix().isEmpty())
        suffixedName += "." + info.suffix();
      linkPath = dir.filePath(suffixedName);
      ++suffix;
    }

    if(QFile::link(path, linkPath)) {
      struct timespec times[2];
      times[0].tv_sec = now - index; // atime
      times[0].tv_nsec = 0;
      times[1] = times[0]; // mtime
      utimensat(AT_FDCWD, linkPath.toLocal8Bit().constData(), times, AT_SYMLINK_NOFOLLOW);
    }
    ++index;
  }
}

}
