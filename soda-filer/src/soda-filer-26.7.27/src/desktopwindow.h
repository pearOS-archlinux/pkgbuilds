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


#ifndef PCMANFM_DESKTOPWINDOW_H
#define PCMANFM_DESKTOPWINDOW_H

#include "view.h"
#include "launcher.h"
#include <QHash>
#include <QPoint>
#include <QByteArray>
#include <xcb/xcb.h>

class QMenuBar;
class QMenu;

namespace Fm {
  class FolderModel;
  class ProxyFolderModel;
  class FolderViewListView;
}

namespace Filer {

class DesktopItemDelegate;
class DesktopMainWindow;
class Settings;

class DesktopWindow : public View {
Q_OBJECT
public:
  friend class Application;

  enum WallpaperMode {
    WallpaperNone,
    WallpaperTransparent,
    WallpaperStretch,
    WallpaperFit,
    WallpaperCenter,
    WallpaperTile
  };

  explicit DesktopWindow(int screenNum);
  virtual ~DesktopWindow();

  void setForeground(const QColor& color);
  void setShadow(const QColor& color);
  void setBackground(const QColor& color);
  void setDesktopFolder();
  void setWallpaperFile(QString filename);
  void setWallpaperMode(WallpaperMode mode = WallpaperStretch);

  // void setWallpaperAlpha(qreal alpha);
  void updateWallpaper();
  void updateFromSettings(Settings& settings);

  void queueRelayout(int delay = 0);

  // Pin a (possibly not-yet-listed) item at refName's position plus offset.
  // No-op if refName isn't an item of this desktop.
  void pinNewItemNear(const QByteArray& refName, const QByteArray& newName, const QPoint& offset);

  void setReservedTopArea(int px);

  // Average color of wallpaperPixmap_ within localRect (same logical-pixel,
  // per-screen-local coordinate space this window itself paints in). Used to
  // dynamically tint the sidebar of a Filer window sitting over this screen.
  // Falls back to a neutral gray if the rect misses the wallpaper entirely.
  QColor averageColorForRect(const QRect& localRect) const;

  int screenNum() const {
    return screenNum_;
  }

  void setScreenNum(int num);

protected:
  virtual void prepareFolderMenu(Fm::FolderMenu* menu);
  virtual void prepareFileMenu(Fm::FileMenu* menu);
  virtual void resizeEvent(QResizeEvent* event);
  virtual void paintEvent(QPaintEvent* event);
  virtual void onFileClicked(int type, FmFileInfo* fileInfo);

  void loadItemPositions();
  void saveItemPositions();
  void showOrRefreshQuickLook(bool allowToggleClose);
  void onFileRenamed(const QByteArray& oldName, const QByteArray& newName);

  QSize gridCellSize() const;
  QHash<QByteArray, QPoint> computeGridPositions() const;
  QHash<QByteArray, QPoint> computeGridPositionsSorted(int column, Qt::SortOrder order) const;
  QHash<QByteArray, QPoint> computeGridPositionsForOrder(const QVector<QByteArray>& orderedNames) const;
  QHash<QByteArray, QPoint> capturePositions() const;
  QHash<QByteArray, QPoint> computeArrangedPositions() const;
  QHash<QByteArray, QPoint> computeSnappedPositions() const;
  void applyGridPositions(const QHash<QByteArray, QPoint>& positions);
  void animateRelayout(const QHash<QByteArray, QPoint>& startPositions, const QHash<QByteArray, QPoint>& targetPositions);
  void buildSortCriteriaMenu(QMenu* parent, bool isCleanUpBy);
  void applyGridSnapState();

  QImage loadWallpaperFile(QSize requiredSize);

  virtual bool event(QEvent* event);
  virtual bool eventFilter(QObject * watched, QEvent * event);

  virtual void childDropEvent(QDropEvent* e);
  virtual void closeEvent(QCloseEvent *event);

protected Q_SLOTS:
  void onOpenFolder(QString folder);
  void onOpenFolderAndSelectItems(QString folder, QStringList items);
  void onOpenTrash();
  void onOpenDesktop();
  void onOpenDocuments();
  void onOpenDownloads();
  void onOpenHome();
  void onOpenDirRequested(FmPath* path, int target);
  void onDesktopPreferences();
  void onFilerPreferences();
  void onGoUp();
  void onOpenActivated(); // probono
  void onOpenWithActivated(); // probono
  void onShowContentsActivated(); // probono
  void onNewFolder();
  void onNewBlankFile();
  void onOpenTerminal();
  void onFindFiles();
  void onAbout();
  void onEditBookmarks();

  void onShowHidden(bool hidden);
  void onSortColumn(int column);
  void onArrangeBy(int column);
  void onSortOrder(Qt::SortOrder order);
  void onFolderFirst(bool first);
  void onCaseSensitive(Qt::CaseSensitivity sensitivity);
  void onReload();

  void onStackByChanged(int index);
  void onSortByChanged(int index);
  void onIconSizeChanged(int size);
  void onGridSpacingChanged(int spacing);
  void onTextSizeChanged(int size);
  void onLabelPositionChanged(bool right);
  void onShowItemInfoChanged(bool show);
  void onShowIconPreviewChanged(bool show);

  void updateMenu();

  void onRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  void onRowsInserted(const QModelIndex& parent, int start, int end);
  void onLayoutChanged();
  void onLayoutAboutToChange();
  void onIndexesMoved(const QModelIndexList& indexes);

  void relayoutItems();
  void onStickToCurrentPos(bool toggled);

  void onCleanUp();
  void onCleanUpBy(int column);
  void onUseStacksToggled(bool checked);
  void onShowViewOptions();

  // void updateWorkArea();

  // file operations
  void onCutActivated();
  void onCopyActivated();
  void onPasteActivated();
  void onDuplicateActivated();
  void onEmptyTrashActivated();
  void onRenameActivated();
  void onQuickLookActivated();
  void onQuickLookRefreshOnSelectionChange();
  void onDeleteActivated();
  void onDeleteWithoutTrashActivated();
  void onFilePropertiesActivated();

private:
  Fm::ProxyFolderModel* proxyModel_;
  Fm::FolderModel* model_;
  FmFolder* folder_;
  Fm::FolderViewListView* listView_;

  QColor fgColor_;
  QColor bgColor_;
  QColor shadowColor_;
  QString wallpaperFile_;
  WallpaperMode wallpaperMode_;
  // Rendered by paintEvent() -- rebuilt by updateWallpaper() whenever the
  // wallpaper file/mode/size changes. Qt's stylesheet "border-image" property
  // (previously used here for Stretch/Fit) silently paints nothing on this
  // style/platform combination, leaving the desktop solid black, so the
  // image is drawn directly instead.
  QPixmap wallpaperPixmap_;
  DesktopItemDelegate* delegate_;
  Launcher fileLauncher_;

  int screenNum_;
  QHash<QByteArray, QPoint> customItemPos_;
  QTimer* relayoutTimer_;
  // desktopArrangeColumn_ values:
  //   -1 (ArrangeNone) - free arrangement: icons individually draggable, no
  //       forced grid snap.
  //   -2 (ArrangeGrid) - locked to the grid, but in whatever order the icons
  //       already are (no sort criterion applied).
  //   >= 0 - a Fm::FolderModel::Column* value: locked to the grid, ordered by
  //       that column.
  // Locked (!= ArrangeNone) means the desktop is kept continuously arranged
  // (matches Finder's "Sort By" -- as opposed to the one-shot "Clean Up By").
  static const int ArrangeNone = -1;
  static const int ArrangeGrid = -2;
  int desktopArrangeColumn_;

  int desktopIconSize_;
  int desktopGridSpacing_;
  int desktopTextSize_;
  bool desktopLabelRight_;
  bool desktopShowItemInfo_;
  bool desktopShowIconPreview_;
  DesktopMainWindow* desktopMainWindow_;
};

}

#endif // PCMANFM_DESKTOPWINDOW_H
