#include "dmgmountregistry.h"
#include <QDir>
#include <QFile>

namespace Filer {

DmgMountRegistry& DmgMountRegistry::instance() {
  static DmgMountRegistry inst;
  return inst;
}

void DmgMountRegistry::addMount(const QString& name, const QString& path, const QString& tmpRoot,
                                const QString& desktopLinkPath) {
  for(const DmgMount& m : mounts_) {
    if(m.path == path)
      return; // already tracked
  }
  DmgMount mount;
  mount.name = name;
  mount.path = path;
  mount.tmpRoot = tmpRoot;
  mount.desktopLinkPath = desktopLinkPath;
  mounts_.append(mount);
  Q_EMIT mountAdded(mount);
}

const DmgMount* DmgMountRegistry::mountForPath(const QString& path) const {
  for(const DmgMount& m : mounts_) {
    if(m.path == path)
      return &m;
  }
  return nullptr;
}

const DmgMount* DmgMountRegistry::mountForDesktopLink(const QString& desktopLinkPath) const {
  for(const DmgMount& m : mounts_) {
    if(m.desktopLinkPath == desktopLinkPath)
      return &m;
  }
  return nullptr;
}

void DmgMountRegistry::eject(const QString& path) {
  for(int i = 0; i < mounts_.size(); ++i) {
    if(mounts_.at(i).path == path) {
      QString tmpRoot = mounts_.at(i).tmpRoot;
      QString desktopLinkPath = mounts_.at(i).desktopLinkPath;
      mounts_.removeAt(i);
      QDir(tmpRoot).removeRecursively();
      if(!desktopLinkPath.isEmpty())
        QFile::remove(desktopLinkPath);
      Q_EMIT mountRemoved(path);
      return;
    }
  }
}

} // namespace Filer
