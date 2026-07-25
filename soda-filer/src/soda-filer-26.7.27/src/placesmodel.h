/*

    Copyright (C) 2013  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

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


#ifndef FM_PLACESMODEL_H
#define FM_PLACESMODEL_H

#include "libfmqtglobals.h"
#include "dmgmountregistry.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHash>
#include <QList>
#include <QVector>
#include <QPair>
#include <QString>
#include <QAction>
#include <QColor>
#include <libfm/fm.h>

namespace Fm {

class PlacesModelItem;
class PlacesModelVolumeItem;
class PlacesModelMountItem;
class PlacesModelBookmarkItem;

class LIBFM_QT_API PlacesModel : public QStandardItemModel {
Q_OBJECT
friend class PlacesView;
public:

  // QAction used for popup menus
  class ItemAction : public QAction {
  public:
    ItemAction(const QModelIndex& index, QString text, QObject* parent = 0):
      QAction(text, parent),
      index_(index) {
    }

    QPersistentModelIndex& index() {
      return index_;
    }
  private:
    QPersistentModelIndex index_;
  };

public:
  explicit PlacesModel(QObject* parent = 0);
  virtual ~PlacesModel();

  bool showTrash() {
    return trashItem_ != NULL;
  }
  void setShowTrash(bool show);

  bool showApplications() {
    return showApplications_;
  }
  void setShowApplications(bool show);

  bool showDesktop() {
    return showDesktop_;
  }
  void setShowDesktop(bool show);

public Q_SLOTS:
  void updateIcons();
  void updateTrash();
  // Regenerates the "Tags" section's colored dot pixmaps at the given size
  // -- called by PlacesView::onIconSizeChanged() so the dots follow
  // Settings::sidePaneIconSize() live, same as every other sidebar icon
  // (they used to be a hardcoded 16x16 pixmap, unaffected by icon size).
  void setTagIconSize(int size);

  // Sets the row height of the Favorites/Locations/Tags category headers
  // -- top is the gap above the label (also used by PlacesView's delegate
  // to position the label text), bottom is the gap below it; row height is
  // top + label height + bottom. See Settings::sidePaneCategorySpacing()/
  // sidePaneCategorySpacingBottom(). Live-updatable the same way as
  // setTagIconSize().
  void setCategorySpacing(int top, int bottom);
  // Font size (px) of the category header labels; also affects row height
  // since that's top + label height + bottom.
  void setCategoryFontSize(int size);
  void onDmgMountAdded(const Filer::DmgMount& mount);
  void onDmgMountRemoved(const QString& path);

protected:

  PlacesModelItem* itemFromPath(FmPath* path);
  PlacesModelItem* itemFromPath(QStandardItem* rootItem, FmPath* path);
  PlacesModelVolumeItem* itemFromVolume(GVolume* volume);
  PlacesModelMountItem* itemFromMount(GMount* mount);

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QStringList mimeTypes() const;
  virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
  virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
  Qt::DropActions supportedDropActions() const;

  void createTrashItem();

private:

  static void onVolumeAdded(GVolumeMonitor* monitor, GVolume* volume, PlacesModel* pThis);
  static void onVolumeRemoved(GVolumeMonitor* monitor, GVolume* volume, PlacesModel* pThis);
  static void onVolumeChanged(GVolumeMonitor* monitor, GVolume* volume, PlacesModel* pThis);
  static void onMountAdded(GVolumeMonitor* monitor, GMount* mount, PlacesModel* pThis);
  static void onMountRemoved(GVolumeMonitor* monitor, GMount* mount, PlacesModel* pThis);
  static void onMountChanged(GVolumeMonitor* monitor, GMount* mount, PlacesModel* pThis);

  static void onTrashChanged(GFileMonitor *monitor, GFile *gf, GFile *other, GFileMonitorEvent evt, PlacesModel* pThis);
private:
  GVolumeMonitor* volumeMonitor;
  QList<FmJob*> jobs;
  bool showApplications_;
  bool showDesktop_;

  // Top-level standalone items (no section header)
  PlacesModelItem* recentsItem;
  PlacesModelItem* sharedItem;

  // "Favorites" section
  QStandardItem* placesRoot;
  PlacesModelItem* applicationsItem;
  PlacesModelItem* desktopItem;
  PlacesModelItem* documentsItem;
  PlacesModelItem* downloadsItem;

  // "Locations" section
  QStandardItem* devicesRoot;
  PlacesModelItem* pcloudItem;
  PlacesModelItem* homeItem;
  PlacesModelItem* peardropItem;
  PlacesModelItem* trashItem_;
  GFileMonitor* trashMonitor_;

  // "Tags" section
  QStandardItem* tagsRoot;

  PlacesModelItem* computerItem;
  PlacesModelItem* networkItem;
  QIcon ejectIcon_;

  // DMG volumes registered by filelauncher.cpp (see dmgmountregistry.h),
  // keyed by the extracted folder path so onDmgMountRemoved() can find the
  // row to remove.
  QHash<QString, PlacesModelItem*> dmgVolumeItems_;

  // Sidebar items whose icon is one of the bundled monochrome SVGs (recolored
  // to the active accent color by themedSidebarIcon()) rather than an
  // FmIcon-backed theme icon. PlacesModelItem::updateIcon() only re-resolves
  // FmIcon-backed icons, so these need to be tracked and re-tinted separately
  // whenever the accent color can have changed -- see updateIcons().
  QVector<QPair<PlacesModelItem*, QString>> themedIconItems_;

  // Same as themedIconItems_, but for the "Locations" section (pCloud Drive,
  // home, PearDrop, Trash): those are tinted a fixed light gray instead of
  // the accent color, so they need their own re-tint list in updateIcons().
  QVector<QPair<PlacesModelItem*, QString>> grayIconItems_;

  // "Tags" section color-dot items, tracked so setTagIconSize() can
  // regenerate their pixmap at a new size.
  QVector<QPair<PlacesModelItem*, QColor>> tagIconItems_;
  int tagIconSize_ = 16;
  int categorySpacingTop_ = 17;
  int categorySpacingBottom_ = 8;
  void applyCategoryLayout();
};

}

#endif // FM_PLACESMODEL_H
