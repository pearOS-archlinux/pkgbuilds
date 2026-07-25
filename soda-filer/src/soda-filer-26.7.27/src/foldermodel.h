/*

    Copyright (C) 2012  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef FM_FOLDERMODEL_H
#define FM_FOLDERMODEL_H

#include "libfmqtglobals.h"
#include <QAbstractListModel>
#include <QIcon>
#include <QImage>
#include <libfm/fm.h>
#include <QList>
#include <QVector>
#include <QLinkedList>
#include <QPair>
#include "foldermodelitem.h"

namespace Fm {

class LIBFM_QT_API FolderModel : public QAbstractListModel {
Q_OBJECT
public:

  enum Role {
    FileInfoRole = Qt::UserRole
  };

  enum ColumnId {
    ColumnFileName,
    ColumnFileType,
    ColumnFileSize,
    ColumnFileMTime,
    ColumnFileCTime,
    ColumnFileTags,
    ColumnFileOwner,
    NumOfColumns
  };

public:
  FolderModel();
  virtual ~FolderModel();

  FmFolder* folder() {
    return folder_;
  }

  FmFolder* computerFolder() {
    return computerFolder_;
  }

  void setFolder(FmFolder* new_folder, bool add_devices = false);

  FmPath* path() {
    return folder_ ? fm_folder_get_path(folder_) : NULL;
  }

  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount (const QModelIndex & parent) const;
  QVariant data(const QModelIndex & index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
  QModelIndex parent( const QModelIndex & index ) const;
  // void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  Qt::ItemFlags flags(const QModelIndex & index) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  virtual QStringList mimeTypes() const;
  virtual QMimeData* mimeData(const QModelIndexList & indexes) const;
  virtual Qt::DropActions supportedDropActions() const;
  virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

  FmFileInfo* fileInfoFromIndex(const QModelIndex& index) const;
  FolderModelItem* itemFromIndex(const QModelIndex& index) const;
  QImage thumbnailFromIndex(const QModelIndex& index, int size);

  void cacheThumbnails(int size);
  void releaseThumbnails(int size);

  void wantToSelect(QStringList files, bool add, void *view);

Q_SIGNALS:
  void thumbnailLoaded(const QModelIndex& index, int size);
  // Emitted synchronously from setData() right when a rename is issued --
  // i.e. well before the filesystem watcher's resulting remove+insert
  // events arrive asynchronously. Lets listeners (e.g. the desktop's
  // position-pinning map, keyed by file name) migrate any old-name-keyed
  // state to the new name before that remove+insert cycle would otherwise
  // wipe it out.
  void fileRenamed(const QByteArray& oldName, const QByteArray& newName);

public Q_SLOTS:
  void updateIcons();

protected:
  static void onStartLoading(FmFolder* folder, gpointer user_data);
  static void onFinishLoading(FmFolder* folder, gpointer user_data);
  static void onFilesAdded(FmFolder* folder, GSList* files, gpointer user_data);
  static void onFilesChanged(FmFolder* folder, GSList* files, gpointer user_data);
  static void onFilesRemoved(FmFolder* folder, GSList* files, gpointer user_data);
  static void onThumbnailLoaded(FmThumbnailLoader *res, gpointer user_data);

  void onFinishedLoading();
  void insertFiles(int row, FmFileInfoList* files);
  void removeAll();
  QList<FolderModelItem>::iterator findItemByPath(FmPath* path, int* row);
  QList<FolderModelItem>::iterator findItemByName(const char* name, int* row);
  QList<FolderModelItem>::iterator findItemByFileInfo(FmFileInfo* info, int* row);
  bool consumePendingRename(const QByteArray& newName, QByteArray* oldNameOut);

private:
  FmFolder* folder_;
  FmFolder* computerFolder_; // the items added for drives on the desktop
  // FIXME: should we use a hash table here so item lookup becomes much faster?
  QList<FolderModelItem> items;

  // Renames issued through setData() are applied to the row immediately
  // (see setData()) instead of waiting for the filesystem watcher's async
  // remove(oldName)+add(newName) notifications, which otherwise arrive as
  // two separate model changes with a visible gap between them (the item
  // disappears, then reappears -- perceived as blinking). This maps
  // oldName -> newName for renames whose async notifications haven't
  // arrived yet, so onFilesRemoved/onFilesAdded can recognize and swallow
  // them instead of duplicating the row change. Entries are also cleared
  // by a safety timeout in setData() in case the notifications never
  // arrive (matching the old name).
  QHash<QByteArray, QByteArray> pendingRenames_;

  // record what size of thumbnails we should cache in an array of <size, refCount> pairs.
  QVector<QPair<int, int> > thumbnailRefCounts;
  QLinkedList<FmThumbnailLoader*> thumbnailResults;

  // for "ShowItems"
  QStringList filesToSelect;
  bool filesToSelectAdd;
  void *filesToSelectView;
  QString makeFilenameSafe(const QString &input);
};

}

#endif // FM_FOLDERMODEL_H
