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


#ifndef FM_FOLDERITEMDELEGATE_H
#define FM_FOLDERITEMDELEGATE_H

#include "libfmqtglobals.h"
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QHash>
#include <QPersistentModelIndex>
#include <QPoint>
#include <QPair>
#include <QString>

typedef struct _FmFileInfo FmFileInfo;

namespace Fm {

class LIBFM_QT_API FolderItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  explicit FolderItemDelegate(QAbstractItemView* view, QObject* parent = 0);
  virtual ~FolderItemDelegate();

  void setGridSize(QSize size) {
    gridSize_ = size;
  }

  QSize gridSize() {
    return gridSize_;
  }

  // Per-side grid cell padding (px) -- see Settings::gridSpacingTop() etc.
  // Replaces the flat symmetric padding sizeHint() used to bake in.
  void setGridPadding(int left, int right, int top, int bottom) {
    gridPaddingLeft_ = left;
    gridPaddingRight_ = right;
    gridPaddingTop_ = top;
    gridPaddingBottom_ = bottom;
  }

  // Label styling -- see Settings::iconViewTextSize() etc. size/weight 0
  // means "don't override, keep the item view's own default font"; anchor
  // is "left"/"center"/"right" ("center" for anything else, matching the
  // previous hardcoded behavior).
  void setTextStyle(int size, const QString& anchor, int marginTop, int marginBottom, int weight) {
    textSize_ = size;
    textAnchor_ = anchor;
    textMarginTop_ = marginTop;
    textMarginBottom_ = marginBottom;
    textWeight_ = weight;
  }

  virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

  // Per-index pixel offsets applied while a Clean Up/Clean Up By slide
  // animation is running -- same mechanism as DesktopItemDelegate's.
  void setAnimationOffsets(const QHash<QPersistentModelIndex, QPoint>& offsets) {
      animationOffsets_ = offsets;
  }
  void clearAnimationOffsets() {
      animationOffsets_.clear();
  }

private:
  void drawText(QPainter* painter, QStyleOptionViewItem& opt, const QString& subtext, const QColor& tagColor = QColor()) const;
  static QIcon::Mode iconModeFromState(QStyle::State state);
  QString itemCountText(void* file) const;
  // If `file` is a directory and has a tag, tries a color-named themed icon
  // (e.g. "folder-red") to replace `fallback`; returns `fallback` unchanged
  // for files (only the label's tag dot applies to those) or if the icon
  // theme has no matching colored folder icon.
  static QIcon taggedFolderIcon(const QIcon& fallback, FmFileInfo* file, const QString& tagName);

private:
  QAbstractItemView* view_;
  QIcon symlinkIcon_;
  QSize gridSize_;
  int gridPaddingLeft_ = 2;
  int gridPaddingRight_ = 2;
  int gridPaddingTop_ = 2;
  int gridPaddingBottom_ = 2;
  int textSize_ = 0;
  QString textAnchor_ = QStringLiteral("center");
  int textMarginTop_ = 0;
  int textMarginBottom_ = 0;
  int textWeight_ = 0;
  // path -> (isAppBundleNotLaunchable, lastCheckedMsecsSinceEpoch) -- see
  // the identical cache in DesktopItemDelegate for why this is cached
  // (isAppBundleNotLaunchable() does several filesystem checks per call).
  mutable QHash<QString, QPair<bool, qint64> > notLaunchableCache_;
  // path -> (formatted "N items" subtext, dir mtime at compute time) -- a
  // directory listing per repaint would be far too slow, so entries are
  // recomputed only when the directory's mtime changes (see itemCountText()).
  mutable QHash<QString, QPair<QString, qint64> > itemCountCache_;
  // path -> (hasUncommittedChanges, lastCheckedMsecsSinceEpoch) -- the git
  // emblem used to run a BLOCKING `git status` subprocess on every repaint.
  mutable QHash<QString, QPair<bool, qint64> > gitStatusCache_;
  // path -> (tagName, lastCheckedMsecsSinceEpoch) -- same cache pattern as
  // DesktopItemDelegate to avoid reading xattrs on every repaint.
  mutable QHash<QString, QPair<QString, qint64> > tagCache_;
  QHash<QPersistentModelIndex, QPoint> animationOffsets_;
};

}

#endif // FM_FOLDERITEMDELEGATE_H
