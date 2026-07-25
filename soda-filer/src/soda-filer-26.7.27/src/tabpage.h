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


#ifndef FM_TABPAGE_H
#define FM_TABPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPixmap>
#include <libfm/fm.h>

class QLabel;
class QListView;

namespace Fm {
class ColumnView;
}
#include "browsehistory.h"
#include "view.h"
#include "path.h"

namespace Fm {
  class FileLauncher;
  class FolderModel;
  class ProxyFolderModel;
  class CachedFolderModel;
};

namespace Filer {

class Settings;
class Launcher;

class ProxyFilter : public Fm::ProxyFolderModelFilter {
public:
  bool filterAcceptsRow(const Fm::ProxyFolderModel* model, FmFileInfo* info) const;
  virtual ~ProxyFilter() {}
  void setVirtHidden(FmFolder* folder);
  QString getFilterStr() {
    return filterStr_;
  }
  void setFilterStr(QString str) {
    filterStr_ = str;
  }

private:
  QString filterStr_;
  QStringList virtHiddenList_;
};

class TabPage : public QWidget {
Q_OBJECT

public:
  enum StatusTextType {
    StatusTextNormal,
    StatusTextSelectedFiles,
    StatusTextFSInfo,
    StatusTextNum
  };

public:
  explicit TabPage(FmPath* path, QWidget* parent = 0);
  virtual ~TabPage();

  void chdir(FmPath* newPath, bool addHistory = true);

  Fm::FolderView::ViewMode viewMode() {
    return folderView_->viewMode();
  }

  void setViewMode(Fm::FolderView::ViewMode mode) {
    setGalleryMode(false); // picking a regular mode always leaves Gallery
    setColumnMode(false);  // ...and Column view
    folderView_->setViewMode(mode);
  }

  // Finder-style Gallery view: large preview + filmstrip, replacing the
  // regular folder view while active. Not a Fm::FolderView::ViewMode --
  // it's a sibling widget sharing the same proxyModel_.
  void setGalleryMode(bool on);
  bool galleryMode() const {
    return galleryWidget_ && galleryWidget_->isVisible();
  }

  // Finder-style Column (Miller) view: cascading per-directory columns,
  // replacing the regular folder view while active. Column navigation is
  // internal to the widget (the tab's own path stays at the root the
  // columns started from).
  void setColumnMode(bool on);
  bool columnMode() const;

  void sort(int col, Qt::SortOrder order = Qt::AscendingOrder) {
    // if(folderModel_)
    //  folderModel_->sort(col, order);
    if(proxyModel_)
      proxyModel_->sort(col, order);
  }

  int sortColumn() {
    return proxyModel_->sortColumn();
  }

  Qt::SortOrder sortOrder() {
    return proxyModel_->sortOrder();
  }

  bool sortFolderFirst() {
    return proxyModel_->folderFirst();
  }
  void setSortFolderFirst(bool value) {
    proxyModel_->setFolderFirst(value);
  }

  bool sortCaseSensitive() {
    return proxyModel_->sortCaseSensitivity();
  }
  void setSortCaseSensitive(bool value) {
    proxyModel_->setSortCaseSensitivity(value ? Qt::CaseSensitive : Qt::CaseInsensitive);
  }

  bool showHidden() {
    return proxyModel_->showHidden();
  }

  void setShowHidden(bool showHidden);

  FmPath* path() {
    return folder_ ? fm_folder_get_path(folder_) : NULL;
  }

  QString pathName();

  FmFolder* folder() {
    return folder_;
  }

  Fm::FolderModel* folderModel() {
    return reinterpret_cast<Fm::FolderModel*>(folderModel_);
  }

  View* folderView() {
    return folderView_;
  }

  Fm::BrowseHistory& browseHistory() {
    return history_;
  }

  FmFileInfoList* selectedFiles() {
    return folderView_->selectedFiles();
  }

  FmPathList* selectedFilePaths() {
    return folderView_->selectedFilePaths();
  }

  void selectAll();

  void invertSelection();

  void reload() {
    if(folder_) {
      proxyFilter_->setVirtHidden(folder_); // reread ".hidden"
      fm_folder_reload(folder_);
    }
  }

  QString title() const {
    return title_;
  }

  QString statusText(StatusTextType type = StatusTextNormal) const {
    return statusText_[type];
  }

  bool canBackward() {
    return history_.canBackward();
  }

  void backward();

  bool canForward() {
    return history_.canForward();
  }

  void forward();

  void jumpToHistory(int index);

  bool canUp();

  void up();

  void updateFromSettings(Settings& settings);

  void setFileLauncher(Fm::FileLauncher* launcher) {
    folderView_->setFileLauncher(launcher);
  }

  Fm::FileLauncher* fileLauncher() {
    return folderView_->fileLauncher();
  }

  QString getFilterStr() {
    if(proxyFilter_)
      return proxyFilter_->getFilterStr();
    return QString();
  }

  void setFilterStr(QString str) {
    if(proxyFilter_)
      proxyFilter_->setFilterStr(str);
  }

  void applyFilter();

Q_SIGNALS:
  void statusChanged(int type, QString statusText);
  void titleChanged(QString title);
  void openDirRequested(FmPath* path, int target);
  void sortFilterChanged();
  void forwardRequested();
  void backwardRequested();

protected Q_SLOTS:
  void onOpenDirRequested(FmPath* path, int target);
  void onModelSortFilterChanged();
  void onSelChanged(int numSel);
  void restoreScrollPos();

private:
  void freeFolder();
  QString formatStatusText();
  void updateGalleryPreview(const QModelIndex& index);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

  static void onFolderStartLoading(FmFolder* _folder, TabPage* pThis);
  static void onFolderFinishLoading(FmFolder* _folder, TabPage* pThis);
  static FmJobErrorAction onFolderError(FmFolder* _folder, GError* err, FmJobErrorSeverity severity, TabPage* pThis);
  static void onFolderFsInfo(FmFolder* _folder, TabPage* pThis);
  static void onFolderRemoved(FmFolder* _folder, TabPage* pThis);
  static void onFolderUnmount(FmFolder* _folder, TabPage* pThis);
  static void onFolderContentChanged(FmFolder* _folder, TabPage* pThis);

private:
  View* folderView_;
  Fm::CachedFolderModel* folderModel_;
  Fm::ProxyFolderModel* proxyModel_;
  ProxyFilter* proxyFilter_;
  QVBoxLayout* verticalLayout;
  QWidget* galleryWidget_;
  QLabel* galleryPreview_;
  QListView* galleryStrip_;
  QPixmap galleryPixmap_; // unscaled preview source, rescaled on resize

  // Compact Quick-Look-style info sidebar, right of the preview/filmstrip --
  // narrower than Column view's info panel (see columnview.cpp), just
  // name/kind/size/modified, no big icon (Gallery already has the large
  // image preview to its left).
  QWidget* galleryInfoPanel_;
  QLabel* galleryInfoName_;
  QLabel* galleryInfoKind_;
  QLabel* galleryInfoSize_;
  QLabel* galleryInfoModified_;
  Fm::ColumnView* columnView_;
  FmFolder* folder_;
  QString title_;
  QString statusText_[StatusTextNum];
  Fm::BrowseHistory history_; // browsing history
  bool overrideCursor_;
};

}

#endif // FM_TABPAGE_H
