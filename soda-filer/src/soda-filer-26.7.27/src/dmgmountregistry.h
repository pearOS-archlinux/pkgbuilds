/*
    Tracks .dmg files that filelauncher.cpp has extracted into a temp
    directory (see the ".dmg" branch there) so the sidebar and the desktop
    can present that extraction as a mounted volume with a real "Eject"
    action, instead of it just being an ordinary temp folder nobody cleans
    up. This is not a real block-device/GVolume mount -- see
    filelauncher.cpp for why (7z extraction, not hdiutil/udisks).
*/

#ifndef FILER_DMGMOUNTREGISTRY_H
#define FILER_DMGMOUNTREGISTRY_H

#include <QObject>
#include <QString>
#include <QVector>

namespace Filer {

struct DmgMount {
  QString name;           // display name, e.g. the bundled .app's basename without ".app"
  QString path;           // the extracted folder shown as the volume's contents
  QString tmpRoot;        // the outer /tmp/pearos_dmg_mounts/<dmgName> dir removed on eject
  QString desktopLinkPath; // ~/Desktop/<name> symlink standing in for the mounted volume icon
};

class DmgMountRegistry : public QObject {
  Q_OBJECT
public:
  static DmgMountRegistry& instance();

  void addMount(const QString& name, const QString& path, const QString& tmpRoot,
                const QString& desktopLinkPath = QString());
  void eject(const QString& path); // deletes tmpRoot, emits mountRemoved
  const QVector<DmgMount>& mounts() const {
    return mounts_;
  }
  const DmgMount* mountForPath(const QString& path) const;
  const DmgMount* mountForDesktopLink(const QString& desktopLinkPath) const;

Q_SIGNALS:
  void mountAdded(const Filer::DmgMount& mount);
  void mountRemoved(const QString& path);

private:
  explicit DmgMountRegistry(QObject* parent = nullptr): QObject(parent) {}

  QVector<DmgMount> mounts_;
};

} // namespace Filer

#endif // FILER_DMGMOUNTREGISTRY_H
