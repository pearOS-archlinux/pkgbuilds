/*

    Copyright (C) 2012  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

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


#ifndef FM_PLACESVIEW_H
#define FM_PLACESVIEW_H

#include "libfmqtglobals.h"
#include <QTreeView>
#include <libfm/fm.h>

namespace Fm {

class PlacesModel;
class PlacesModelItem;

class LIBFM_QT_API PlacesView : public QTreeView {
Q_OBJECT

public:
  explicit PlacesView(QWidget* parent = 0);
  virtual ~PlacesView();

  void setCurrentPath(FmPath* path);
  FmPath* currentPath() {
    return currentPath_;
  }

  // libfm-gtk compatible alias
  FmPath* getCwd() {
    return currentPath();
  }

  void chdir(FmPath* path) {
    setCurrentPath(path);
  }

  void setCategorySpacing(int top, int bottom);
  void setCategoryFontSize(int size);
  void setCategoryLeftPadding(int padding);
  void setSelectorRightInset(int inset);
  void setIconTextSpacing(int spacing);

  // Re-tints the accent-colored sidebar SVG icons (Recents, Documents, ...)
  // with the current QPalette::Highlight -- see PlacesModel::updateIcons()'s
  // doc comment. Needs an explicit call because nothing else in the repaint/
  // theme-change path reaches into the model to trigger it.
  void updateAccentIcons();

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
  void setIconSize(const QSize &size) {
      // The signal QAbstractItemView::iconSizeChanged is only available after Qt 5.5.
      // To simulate the effect for older Qt versions, we override setIconSize().
      QAbstractItemView::setIconSize(size);
      onIconSizeChanged(size);
  }
#endif

Q_SIGNALS:
  void chdirRequested(int type, FmPath* path);

protected Q_SLOTS:
  void onClicked(const QModelIndex & index);
  void onPressed(const QModelIndex & index);
  void onIconSizeChanged(const QSize & size);

  void onOpenNewTab();
  void onOpenNewWindow();

  void onShowInEnclosingFolder();
  void onRemoveFromSidebar();
  void onGetInfo();
  void onAddToDock();

  void onMountVolume();
  void onUnmountVolume();
  void onEjectVolume();
  void onUnmountMount();

protected:
  void drawBranches ( QPainter * painter, const QRect & rect, const QModelIndex & index ) const {
    // override this method to inhibit drawing of the branch grid lines by Qt.
  }

  virtual void dragMoveEvent(QDragMoveEvent* event);
  virtual void dropEvent(QDropEvent* event);
  virtual void contextMenuEvent(QContextMenuEvent* event);
  virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event = nullptr) const override;

private:
  void onEjectButtonClicked(PlacesModelItem* item);
  void activateRow(int type, const QModelIndex& index);
  void updateSpanning(const QModelIndex& parent = QModelIndex());
  // Hides (QTreeView::setRowHidden) any row whose PlacesModelItem::id() is
  // in Settings::hiddenSidebarItems() -- see onRemoveFromSidebar(). Model
  // rows are never actually removed (some, like PearDrop, are used as
  // insertion anchors elsewhere in PlacesModel), so hiding is done here at
  // the view level instead.
  void applyHiddenSidebarItems(const QModelIndex& parent = QModelIndex());

private:
  PlacesModel* model_;
  FmPath* currentPath_;
  class ItemDelegate* itemDelegate_;
};

}

#endif // FM_PLACESVIEW_H
