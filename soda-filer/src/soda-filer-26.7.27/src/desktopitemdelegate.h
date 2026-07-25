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


#ifndef PCMANFM_DESKTOPITEMDELEGATE_H
#define PCMANFM_DESKTOPITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QColor>
#include <QHash>
#include <QString>
#include <QPair>

class QListView;
class QTextOption;
class QTextLayout;

namespace Filer {

class DesktopItemDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
  explicit DesktopItemDelegate(QListView* view, QObject* parent = 0);
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  virtual ~DesktopItemDelegate();

  void setShadowColor(const QColor& shadowColor) {
    shadowColor_ = shadowColor;
  }
  const QColor& shadowColor() const {
    return shadowColor_;
  }

  void setTextColor(const QColor& textColor) {
    textColor_ = textColor;
  }
  const QColor& textColor() const {
    return textColor_;
  }

  void setGridSpacing(int spacing) { gridSpacing_ = spacing; }
  void setTextSize(int size) { textSize_ = size; }
  void setLabelPosition(bool right) { labelRight_ = right; }
  void setShowItemInfo(bool show) { showItemInfo_ = show; }
  void setShowIconPreview(bool show) { showIconPreview_ = show; }

public Q_SLOTS:
  // Drops all cached icon/thumbnail pixmaps and repaints. Needed because
  // iconPixmapCache_ is keyed by (path, iconMode, size) -- none of which
  // change when an async-loaded thumbnail replaces the generic icon
  // originally cached under the same key, so without this the old pixmap
  // just kept getting reused. Connected to the model's dataChanged() in
  // DesktopWindow.
  void clearIconCache();

private:
  QListView* view_;
  QIcon symlinkIcon_;
  QColor shadowColor_;
  QColor textColor_;
  // path -> (hasChanges, lastCheckedMsecsSinceEpoch). `git status -s` is
  // spawned as a subprocess and waited on synchronously inside paint();
  // without caching, every repaint of every .git-containing folder icon
  // (e.g. during a Clean Up animation, which repaints continuously) would
  // block on a fresh subprocess spawn, causing severe visible stutter.
  mutable QHash<QString, QPair<int, qint64> > gitStatusCache_;

  // path -> (isAppBundleNotLaunchable, lastCheckedMsecsSinceEpoch). Same
  // reasoning as gitStatusCache_ -- isAppBundleNotLaunchable() does several
  // QFile::exists()/QDir::canonicalPath() calls, too much to redo on every
  // repaint of every .app icon.
  mutable QHash<QString, QPair<bool, qint64> > notLaunchableCache_;

  int gridSpacing_ = 120;
  int textSize_ = 12;
  bool labelRight_ = false;
  bool showItemInfo_ = false;
  bool showIconPreview_ = false;

  mutable QHash<QString, QPair<QString, qint64>> emblemCache_;
  mutable QHash<QString, QPair<QString, qint64> > tagCache_;
  mutable QHash<QString, QPair<QSizeF, QPixmap>> textPixmapCache_;
  mutable QHash<QString, QPixmap> iconPixmapCache_;

public:
  void setAnimationOffsets(const QHash<QPersistentModelIndex, QPoint>& offsets) {
      animationOffsets_ = offsets;
  }
  void clearAnimationOffsets() {
      animationOffsets_.clear();
  }

private:
  QHash<QPersistentModelIndex, QPoint> animationOffsets_;
};

}

#endif // PCMANFM_DESKTOPITEMDELEGATE_H
