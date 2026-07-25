/*
    <one line to give the library's name and an idea of what it does.>
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


#ifndef FM_FILEMENU_H
#define FM_FILEMENU_H

#include "libfmqtglobals.h"
#include <QMenu>
#include <qabstractitemmodel.h>
#include <libfm/fm.h>

class QAction;

struct _FmFileActionItem;

namespace Fm {

class FileLauncher;

class LIBFM_QT_API FileMenu : public QMenu {
Q_OBJECT

public:
  explicit FileMenu(FmFileInfoList* files, FmFileInfo* info, FmPath* cwd, QWidget* parent = 0);
  explicit FileMenu(FmFileInfoList* files, FmFileInfo* info, FmPath* cwd, const QString& title, QWidget* parent = 0);
  ~FileMenu();

  bool confirmDelete() {
    return confirmDelete_;
  }

  void setConfirmDelete(bool confirm) {
    confirmDelete_ = confirm;
  }

  QAction* openAction() {
    return openAction_;
  }

//  QAction* openWithMenuAction() {
//    return openWithMenuAction_;
//  }

  QAction* openWithAction() {
    return openWithAction_;
  }

  QAction* separator1() {
    return separator1_;
  }

  QAction* cutAction() {
    return cutAction_;
  }

  QAction* copyAction() {
    return copyAction_;
  }

  QAction* pasteAction() {
    return pasteAction_;
  }

  QAction* deleteAction() {
    return deleteAction_;
  }

  QAction* unTrashAction() {
    return unTrashAction_;
  }

  QAction* renameAction() {
    return renameAction_;
  }

  QAction* separator2() {
    return separator2_;
  }

  QAction* propertiesAction() {
    return propertiesAction_;
  }

  FmFileInfoList* files() {
    return files_;
  }

  FmFileInfo* firstFile() {
    return info_;
  }

  FmPath* cwd() {
    return cwd_;
  }

  void setFileLauncher(FileLauncher* launcher) {
    fileLauncher_ = launcher;
  }

  FileLauncher* fileLauncher() {
    return fileLauncher_;
  }

  bool sameType() const {
    return sameType_;
  }

  bool sameFilesystem() const {
    return sameFilesystem_;
  }

  bool allVirtual() const {
    return allVirtual_;
  }

  bool allTrash() const {
    return allTrash_;
  }

  bool confirmTrash() const {
    return confirmTrash_;
  }

  void setConfirmTrash(bool value) {
    confirmTrash_ = value;
  }

  // true when the Finder-style single-item layout was built; most legacy
  // accessors (separator1() etc.) return NULL in that case.
  bool finderStyle() const {
    return finderStyle_;
  }

  QAction* openInNewTabAction() {
    return openInNewTabAction_;
  }

Q_SIGNALS:
  void quickLookRequested();
  // emitted after this menu created a file (e.g. Compress) that the view
  // should single-select once it shows up in the model
  void selectFileRequested(QString path);

protected:
  void createMenu(FmFileInfoList* files, FmFileInfo* info, FmPath* cwd);
  void createFinderMenu(FmFileInfo* file, FmMimeType* mimeType, FmPath* path);
  void addCustomActionItem(QMenu* menu, struct _FmFileActionItem* item);
  void openFilesWithApp(GAppInfo* app);

protected Q_SLOTS:
  void onOpenTriggered();
  void onOpenWithTriggered();
  void onShowContentsTriggered();
  void onFilePropertiesTriggered();
  void onEmptyTrashTriggered();
  void onApplicationTriggered();
  void onCustomActionTrigerred();
/*
  void onCompress();
  void onExtract();
  void onExtractHere();
*/
  void onCutTriggered();
  void onCopyTriggered();
  void onPasteTriggered();
  void onRenameTriggered();
  void onDeleteTriggered();
  void onUnTrashTriggered();
  void onCompressTriggered();
  void onDuplicateTriggered();
  void onMakeAliasTriggered();
  void onOpenTerminalTriggered();
  void onSetWallpaperTriggered();
  void onNotImplemented();

private:
  FmFileInfoList* files_;
  FmFileInfo* info_;
  FmPath* cwd_;
  bool confirmDelete_;
  bool confirmTrash_; // Confirm before moving files into "trash can"

  bool sameType_;
  bool sameFilesystem_;
  bool allVirtual_;
  bool allTrash_;

  QAction* openAction_;
  QAction* showContentsAction_;
  // QAction* openWithMenuAction_;
  QAction* openWithAction_;
  QAction* separator1_;
  QAction* cutAction_;
  QAction* copyAction_;
  QAction* pasteAction_;
  QAction* deleteAction_;
  QAction* emptyTrashAction_;
  QAction* unTrashAction_;
  QAction* renameAction_;
  QAction* separator2_;
  QAction* propertiesAction_;
  QAction* openInNewTabAction_;
  bool finderStyle_;

  FileLauncher* fileLauncher_;
};

}

#endif // FM_FILEMENU_H
