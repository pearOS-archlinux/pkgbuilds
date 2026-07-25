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


#include "placesview.h"
#include "placesmodel.h"
#include "placesmodelitem.h"
#include "mountoperation.h"
#include "fileoperation.h"
#include "trash.h"
#include "dmgmountregistry.h"
#include "filepropsdialog.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QDebug>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include "application.h"

using namespace Fm;

namespace Fm {
// Custom-paints regular items (icon, then text shifted right by a
// configurable gap) and category headers (Favorites/Locations/Tags, text
// positioned a configurable distance from the row's top independently of
// the row's bottom edge). Neither of these is reachable through QSS: once
// a stylesheet rule matches ::item, QStyleSheetStyle computes and draws
// CE_ItemViewItem's icon/text internally rather than delegating the split
// to subElementRect() overrides, so a QProxyStyle can't reach it -- the
// only way to actually control these positions is to draw them here.
class ItemDelegate : public QStyledItemDelegate {
public:
  explicit ItemDelegate(QObject* parent = nullptr): QStyledItemDelegate(parent) {}

  void setIconTextSpacing(int spacing) { iconTextSpacing_ = spacing; }
  void setCategoryTopSpacing(int top) { categoryTopSpacing_ = top; }
  void setCategoryLeftPadding(int padding) { categoryLeftPadding_ = padding; }
  void setSelectorRightInset(int inset) { selectorRightInset_ = inset; }

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget* widget = opt.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();

    // Category headers (Favorites/Locations/Tags) are the only rows with
    // Qt::ItemIsSelectable cleared (see PlacesModel's placesRoot/devicesRoot/
    // tagsRoot construction) -- no icon, no selection pill, just a label
    // anchored `categoryTopSpacing_` px below the row's top edge.
    if(!(index.flags() & Qt::ItemIsSelectable)) {
      painter->save();
      painter->setFont(opt.font);
      QVariant fg = index.data(Qt::ForegroundRole);
      painter->setPen(fg.canConvert<QBrush>() ? fg.value<QBrush>().color() : opt.palette.color(QPalette::Text));
      QFontMetrics fm(opt.font);
      QRect textRect(opt.rect.x() + categoryLeftPadding_, opt.rect.y() + categoryTopSpacing_,
                      opt.rect.width() - categoryLeftPadding_ - 4, fm.height());
      painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, opt.text);
      painter->restore();
      return;
    }

    QRect decorationRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, widget);
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    if(iconTextSpacing_ != 0 && !decorationRect.isEmpty())
      textRect.setLeft(decorationRect.right() + 1 + iconTextSpacing_);

    QIcon icon = opt.icon;
    QString text = opt.text;
    Qt::Alignment iconAlign = opt.decorationAlignment;
    Qt::Alignment textAlign = opt.displayAlignment;
    QIcon::Mode iconMode = (opt.state & QStyle::State_Enabled)
        ? ((opt.state & QStyle::State_Selected) ? QIcon::Selected : QIcon::Normal)
        : QIcon::Disabled;

    // Draw background/selection pill only -- blanking icon/text here stops
    // CE_ItemViewItem from also drawing them at its own fixed positions;
    // they're drawn manually below, at the gap-adjusted rects computed above.
    // The pill's own rect is shrunk independently of the icon/text layout
    // above (which still uses the unmodified `opt`/row rect) so the
    // highlight's reach can be tuned without moving the icon or label --
    // fixed 8px on the left (matching the original look), configurable on
    // the right via selectorRightInset_ (Settings::sidePaneSelectorRightInset()).
    QStyleOptionViewItem bgOpt = opt;
    static const int kSelectorLeftInset = 8;
    bgOpt.rect.adjust(kSelectorLeftInset, 0, -selectorRightInset_, 0);
    bgOpt.icon = QIcon();
    bgOpt.text.clear();
    style->drawControl(QStyle::CE_ItemViewItem, &bgOpt, painter, widget);

    if(!icon.isNull())
      icon.paint(painter, decorationRect, iconAlign, iconMode);

    if(!text.isEmpty()) {
      painter->save();
      painter->setFont(opt.font);
      // Matches buildSidePaneStyleSheet()'s color/:selected/:disabled rules
      // in sidepane.cpp -- drawing manually here bypasses the stylesheet's
      // own text-color handling same as it bypasses its text positioning.
      // Palette-derived (not a hardcoded dark-mode swatch) so this follows
      // a live light/dark switch same as sidepane.cpp's stylesheet does.
      QColor color;
      if(!(opt.state & QStyle::State_Enabled)) {
        color = widget->palette().color(QPalette::Disabled, QPalette::WindowText);
        color.setAlpha(100);
      }
      else
        color = widget->palette().color(QPalette::Active, QPalette::WindowText);
      painter->setPen(color);
      painter->drawText(textRect, textAlign, text);
      painter->restore();
    }
  }

private:
  int iconTextSpacing_ = 6;
  int categoryTopSpacing_ = 17;
  int categoryLeftPadding_ = 4;
  int selectorRightInset_ = 8;
};
} // namespace Fm

PlacesView::PlacesView(QWidget* parent):
  QTreeView(parent),
  currentPath_(NULL) {
  setRootIsDecorated(false);
  setHeaderHidden(true);
  // Section headers (Favorites/Locations/Tags) are depth 0, same as the
  // standalone Recents/Shared rows, but QTreeView still reserves one
  // indentation's worth of left offset for their depth-1 children --
  // shifting Applications/Desktop/... etc. to the right of Recents/Shared.
  // Zero indentation keeps every row's icon flush with the same left edge;
  // headers are still visually distinct via styleSectionHeader()'s
  // bold/smaller/grey font in placesmodel.cpp.
  setIndentation(0);

  connect(this, &QTreeView::clicked, this, &PlacesView::onClicked);
  connect(this, &QTreeView::pressed, this, &PlacesView::onPressed);

  setIconSize(QSize(24, 24));

  // FIXME: we may share this model amont all views
  model_ = new PlacesModel(this);
  setModel(model_);

  itemDelegate_ = new ItemDelegate(this);
  setItemDelegate(itemDelegate_);

  QHeaderView* headerView = header();
  headerView->setSectionResizeMode(0, QHeaderView::Stretch);
  headerView->setSectionResizeMode(1, QHeaderView::Fixed);
  headerView->setStretchLastSection(false);
  expandAll();

  connect(model_, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent, int first, int last) {
      updateSpanning(parent);
  });
  connect(model_, &QAbstractItemModel::modelReset, this, [this]() {
      updateSpanning(QModelIndex());
  });
  connect(model_, &QAbstractItemModel::layoutChanged, this, [this]() {
      updateSpanning(QModelIndex());
  });

  updateSpanning(QModelIndex());
  applyHiddenSidebarItems();

  // the 2nd column is for the eject buttons
  setSelectionBehavior(QAbstractItemView::SelectRows); // FIXME: why this does not work?
  setAllColumnsShowFocus(false);

  setAcceptDrops(true);
  setDragEnabled(true);

  // update the umount button's column width based on icon size
  onIconSizeChanged(iconSize());
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0) // this signal requires Qt >= 5.5
  connect(this, &QAbstractItemView::iconSizeChanged, this, &PlacesView::onIconSizeChanged);
#endif
}

PlacesView::~PlacesView() {
  if(currentPath_)
    fm_path_unref(currentPath_);
  // qDebug("delete PlacesView");
}

void PlacesView::updateAccentIcons() {
  model_->updateIcons();
}

static void populateTagDirectory(const QString& tagName) {
    QString tagsDirBase = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/filer/tags";
    QString tagDirPath = tagsDirBase + "/" + tagName;
    
    // Create/clear the directory
    QDir tagDir(tagDirPath);
    if (tagDir.exists()) {
        tagDir.removeRecursively();
    }
    QDir().mkpath(tagDirPath);
    
    QString homePath = QDir::homePath();
    QStringList args;
    args << homePath << "-not" << "-path" << "*/.*" << "-exec" << "getfattr" << "-n" << "user.filer.tag" << "{}" << "+";
    
    QProcess p;
    p.start("find", args);
    p.waitForFinished(5000); // timeout after 5s
    
    QString output = QString::fromLocal8Bit(p.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    QString currentFile;
    for (const QString& line : lines) {
        if (line.startsWith("# file: ")) {
            QString rawPath = line.mid(8).trimmed();
            if (rawPath.startsWith('/')) {
                currentFile = rawPath;
            } else {
                currentFile = "/" + rawPath;
            }
        } else if (line.startsWith("user.filer.tag=") && !currentFile.isEmpty()) {
            QString val = line.mid(15).trimmed();
            if (val.startsWith('"') && val.endsWith('"')) {
                val = val.mid(1, val.length() - 2);
            }
            QString fileTagName = val.section(':', 0, 0);
            
            // Check if this file matches the selected tag, or if "All Tags..." is selected
            if (tagName == "All Tags..." || fileTagName == tagName) {
                QFileInfo fi(currentFile);
                if (fi.exists()) {
                    QString linkName = tagDirPath + "/" + fi.fileName();
                    // Handle duplicate file names in the tag view by appending a suffix
                    int suffix = 1;
                    while (QFile::exists(linkName)) {
                        linkName = tagDirPath + "/" + fi.baseName() + QString(" (%1).").arg(suffix++) + fi.completeSuffix();
                    }
                    QFile::link(currentFile, linkName);
                }
            }
        }
    }
}

void PlacesView::activateRow(int type, const QModelIndex& index) {
  if(!index.isValid())
    return;

  QStandardItem* rawItem = model_->itemFromIndex(index);
  if(!rawItem)
    return;

  // Section headers (Favorites, Locations, Tags) are not navigable
  if(rawItem == model_->placesRoot || rawItem == model_->devicesRoot || rawItem == model_->tagsRoot)
    return;

  PlacesModelItem* item = static_cast<PlacesModelItem*>(rawItem);

  // If this is a tag item, generate its virtual directory and navigate to it
  if (item->parent() == model_->tagsRoot) {
    QString tagName = item->text();
    populateTagDirectory(tagName);
    
    QString tagsDirBase = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/filer/tags";
    QString tagDirPath = tagsDirBase + "/" + tagName;
    FmPath* tagPath = fm_path_new_for_str(tagDirPath.toUtf8());
    Q_EMIT chdirRequested(type, tagPath);
    fm_path_unref(tagPath);
    return;
  }

  FmPath* path = item->path();
  if(!path) {
    // check if mounting volumes is needed
    if(item->type() == PlacesModelItem::Volume) {
      PlacesModelVolumeItem* volumeItem = static_cast<PlacesModelVolumeItem*>(item);
      if(!volumeItem->isMounted()) {
        // Mount the volume
        GVolume* volume = volumeItem->volume();
        MountOperation* op = new MountOperation(true, this);
        op->mount(volume);
        if(!op->wait())
          return;
        path = item->path();
      }
    }
  }
  if(path) {
    Q_EMIT chdirRequested(type, path);
  }
}

// mouse button pressed
void PlacesView::onPressed(const QModelIndex& index) {
  // if middle button is pressed
  if(QGuiApplication::mouseButtons() & Qt::MiddleButton) {
    // the real item is at column 0
    activateRow(1, 0 == index.column() ? index : index.sibling(index.row(), 0));
  }
}

void PlacesView::onIconSizeChanged(const QSize& size) {
  setColumnWidth(1, size.width() + 5);
  if(model_)
    model_->setTagIconSize(size.width());
}

void PlacesView::setCategorySpacing(int top, int bottom) {
  if(model_)
    model_->setCategorySpacing(top, bottom);
  itemDelegate_->setCategoryTopSpacing(top);
  viewport()->update();
}

void PlacesView::setIconTextSpacing(int spacing) {
  itemDelegate_->setIconTextSpacing(spacing);
  viewport()->update();
}

void PlacesView::setCategoryFontSize(int size) {
  if(model_)
    model_->setCategoryFontSize(size);
}

void PlacesView::setCategoryLeftPadding(int padding) {
  itemDelegate_->setCategoryLeftPadding(padding);
  viewport()->update();
}

void PlacesView::setSelectorRightInset(int inset) {
  itemDelegate_->setSelectorRightInset(inset);
  viewport()->update();
}

void PlacesView::onEjectButtonClicked(PlacesModelItem* item) {
  // The eject button is clicked for a device item (volume or mount)
  if(item->type() == PlacesModelItem::Volume) {
    PlacesModelVolumeItem* volumeItem = static_cast<PlacesModelVolumeItem*>(item);
    MountOperation* op = new MountOperation(true, this);
    if(volumeItem->canEject()) // do eject if applicable
      op->eject(volumeItem->volume());
    else // otherwise, do unmount instead
      op->unmount(volumeItem->volume());
  }
  else if(item->type() == PlacesModelItem::Mount) {
    PlacesModelMountItem* mountItem = static_cast<PlacesModelMountItem*>(item);
    MountOperation* op = new MountOperation(true, this);
    op->unmount(mountItem->mount());
  }
  else if(item->type() == PlacesModelItem::DmgVolume) {
    // Not a real GVolume -- eject just tears down the extracted temp dir
    // (see dmgmountregistry.h/filelauncher.cpp).
    char* pathStr = item->path() ? fm_path_to_str(item->path()) : NULL;
    if(pathStr) {
      Filer::DmgMountRegistry::instance().eject(QString::fromUtf8(pathStr));
      g_free(pathStr);
    }
  }
  qDebug("PlacesView::onEjectButtonClicked");
}

void PlacesView::onClicked(const QModelIndex& index) {
  if(!index.isValid())
    return;

  // For top-level items (Recents, Shared, or section headers)
  if(!index.parent().isValid()) {
    QStandardItem* item = model_->itemFromIndex(index);
    // Section headers are not clickable
    if(item == model_->placesRoot || item == model_->devicesRoot || item == model_->tagsRoot)
      return;
    // Standalone items (Recents, Shared) are navigable
    activateRow(0, index);
    return;
  }

  if(index.column() == 0) {
    activateRow(0, index);
  }
  else if(index.column() == 1) { // column 1 contains eject buttons of the mounted devices
    if(index.parent() == model_->devicesRoot->index()) { // this is a mounted device
      // the eject button is clicked
      QModelIndex itemIndex = index.sibling(index.row(), 0); // the real item is at column 0
      PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(itemIndex));
      if(item) {
        // eject the volume or the mount
        onEjectButtonClicked(item);
      }
    }
    else
      activateRow(0, index.sibling(index.row(), 0));
  }
}

void PlacesView::setCurrentPath(FmPath* path) {
  if(currentPath_)
    fm_path_unref(currentPath_);
  if(path) {
    currentPath_ = fm_path_ref(path);
    // TODO: search for item with the path in model_ and select it.
    PlacesModelItem* item = model_->itemFromPath(currentPath_);
    if(item) {
      selectionModel()->select(item->index(), QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    }
    else
      clearSelection();
  }
  else {
    currentPath_ = NULL;
    clearSelection();
  }
}


void PlacesView::dragMoveEvent(QDragMoveEvent* event) {
  QTreeView::dragMoveEvent(event);
}

void PlacesView::dropEvent(QDropEvent* event) {
  QTreeView::dropEvent(event);
}

void PlacesView::onOpenNewTab()
{
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
      return;
  PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(action->index()));
  if(item)
    Q_EMIT chdirRequested(1, item->path());
}

void PlacesView::onOpenNewWindow()
{
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
      return;
  PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(action->index()));
  if(item)
    Q_EMIT chdirRequested(2, item->path());
}

void PlacesView::onShowInEnclosingFolder() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(action->index()));
  if(!item || !item->path())
    return;
  FmPath* parentPath = fm_path_get_parent(item->path());
  if(!parentPath)
    return;
  QString folder = QString::fromUtf8(fm_path_to_str(parentPath));
  QString itemPathStr = QString::fromUtf8(fm_path_to_str(item->path()));
  fm_path_unref(parentPath);
  Filer::Application* app = static_cast<Filer::Application*>(qApp);
  Q_EMIT app->openFolderAndSelectItems(folder, QStringList() << itemPathStr);
}

void PlacesView::onRemoveFromSidebar() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(action->index()));
  if(!item || item->id().isEmpty())
    return;
  Filer::Application* app = static_cast<Filer::Application*>(qApp);
  Filer::Settings& settings = app->settings();
  QStringList hidden = settings.hiddenSidebarItems();
  if(!hidden.contains(item->id())) {
    hidden << item->id();
    settings.setHiddenSidebarItems(hidden);
    settings.save();
  }
  applyHiddenSidebarItems();
}

void PlacesView::onGetInfo() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(action->index()));
  if(!item || !item->path())
    return;
  GError* err = NULL;
  FmFileInfo* info = fm_file_info_new_from_native_file(item->path(), fm_path_to_str(item->path()), &err);
  if(err)
    g_error_free(err);
  if(info) {
    Fm::FilePropsDialog::showForFile(info);
    fm_file_info_unref(info);
  }
}

void PlacesView::onAddToDock() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(action->index()));
  if(!item || !item->path())
    return;
  QString launcherUrl = QStringLiteral("file://") + QString::fromUtf8(fm_path_to_str(item->path()));

  // filer-dock (a separate Qt6/KF6 process hosting the PearDock plasmoid,
  // see Application::startDock()) reads its pinned items from its own
  // Plasma-style config file, not from anything Filer owns -- there's no
  // IPC to it, so this edits that file directly (a "launchers=" key under
  // the PearDock applet's own Configuration/General group, the standard
  // Plasma taskmanager launcher-list property). filer-dock picks this up
  // next time it (re)reads its config; no live-reload is triggered here.
  QString configPath = QDir::homePath() + "/.config/filer-dock-appletsrc";
  QFile file(configPath);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QStringList lines;
  QString currentGroup;
  QString pearDockGroup;
  while(!file.atEnd()) {
    QString line = QString::fromUtf8(file.readLine());
    lines << line;
    QString trimmed = line.trimmed();
    if(trimmed.startsWith('[') && trimmed.endsWith(']')) {
      currentGroup = trimmed;
    } else if(trimmed == QStringLiteral("plugin=PearDock")) {
      pearDockGroup = currentGroup + QStringLiteral("[Configuration][General]");
    }
  }
  file.close();
  if(pearDockGroup.isEmpty())
    return; // PearDock applet block not found in the config

  int groupLineIdx = -1;
  int launchersLineIdx = -1;
  int groupEndIdx = lines.size();
  for(int i = 0; i < lines.size(); ++i) {
    QString trimmed = lines[i].trimmed();
    if(trimmed == pearDockGroup) {
      groupLineIdx = i;
    } else if(groupLineIdx >= 0 && i > groupLineIdx && trimmed.startsWith('[') && trimmed.endsWith(']')) {
      groupEndIdx = i;
      break;
    } else if(groupLineIdx >= 0 && trimmed.startsWith(QStringLiteral("launchers="))) {
      launchersLineIdx = i;
    }
  }

  if(groupLineIdx < 0) {
    lines << (pearDockGroup + QStringLiteral("\n"));
    lines << (QStringLiteral("launchers=") + launcherUrl + QStringLiteral("\n"));
  } else if(launchersLineIdx >= 0) {
    QString existing = lines[launchersLineIdx].trimmed().mid(QStringLiteral("launchers=").length());
    QStringList urls = existing.isEmpty() ? QStringList() : existing.split(QLatin1Char(','));
    if(!urls.contains(launcherUrl)) {
      urls << launcherUrl;
      lines[launchersLineIdx] = QStringLiteral("launchers=") + urls.join(QLatin1Char(',')) + QStringLiteral("\n");
    }
  } else {
    lines.insert(groupEndIdx, QStringLiteral("launchers=") + launcherUrl + QStringLiteral("\n"));
  }

  if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    return;
  QTextStream out(&file);
  for(const QString& l : lines)
    out << l;
}

void PlacesView::applyHiddenSidebarItems(const QModelIndex& parent) {
  Filer::Application* app = static_cast<Filer::Application*>(qApp);
  const QStringList& hidden = app->settings().hiddenSidebarItems();
  int rows = model_->rowCount(parent);
  for(int i = 0; i < rows; ++i) {
    QModelIndex idx = model_->index(i, 0, parent);
    PlacesModelItem* item = dynamic_cast<PlacesModelItem*>(model_->itemFromIndex(idx));
    bool hide = item && !item->id().isEmpty() && hidden.contains(item->id());
    setRowHidden(i, parent, hide);
    applyHiddenSidebarItems(idx);
  }
}

void PlacesView::onMountVolume() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelVolumeItem* item = static_cast<PlacesModelVolumeItem*>(model_->itemFromIndex(action->index()));
  MountOperation* op = new MountOperation(true, this);
  op->mount(item->volume());
  op->wait();
}

void PlacesView::onUnmountVolume() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelVolumeItem* item = static_cast<PlacesModelVolumeItem*>(model_->itemFromIndex(action->index()));
  MountOperation* op = new MountOperation(true, this);
  op->unmount(item->volume());
  op->wait();
}

void PlacesView::onUnmountMount() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelMountItem* item = static_cast<PlacesModelMountItem*>(model_->itemFromIndex(action->index()));
  GMount* mount = item->mount();
  MountOperation* op = new MountOperation(true, this);
  op->unmount(mount);
  op->wait();
}

void PlacesView::onEjectVolume() {
  PlacesModel::ItemAction* action = static_cast<PlacesModel::ItemAction*>(sender());
  if(!action->index().isValid())
    return;
  PlacesModelVolumeItem* item = static_cast<PlacesModelVolumeItem*>(model_->itemFromIndex(action->index()));
  MountOperation* op = new MountOperation(true, this);
  op->eject(item->volume());
  op->wait();
}

void PlacesView::contextMenuEvent(QContextMenuEvent* event) {
  QModelIndex index = indexAt(event->pos());
  if(index.isValid() && index.parent().isValid()) {
    if(index.column() != 0) // the real item is at column 0
      index = index.sibling(index.row(), 0);
    QMenu* menu = new QMenu(this);
    QAction* action;
    PlacesModelItem* item = static_cast<PlacesModelItem*>(model_->itemFromIndex(index));

    if(item->type() != PlacesModelItem::Places
            && item->type() != PlacesModelItem::Mount
            && (item->type() != PlacesModelItem::Volume
                || static_cast<PlacesModelVolumeItem*>(item)->isMounted())) {

        Filer::Application* app = static_cast<Filer::Application*>(qApp);
        Filer::Settings& settings = app->settings();

        if( ! settings.spatialMode() ){
            action = new PlacesModel::ItemAction(item->index(), tr("Open in New Window"), menu);
            connect(action, &QAction::triggered, this, &PlacesView::onOpenNewWindow);
            menu->addAction(action);
        }

    }

    switch(item->type()) {
      case PlacesModelItem::Places: {
        // Fixed/special items (Recents, Shared, pCloud Drive, PearDrop,
        // Trash) get a shorter menu than regular places (Applications,
        // Desktop, Documents, Downloads, home, ...) -- see
        // PlacesModelItem::isFixed()'s doc comment.
        bool hasPath = item->path() != NULL;
        bool isRegularPlace = !item->isFixed() && hasPath;

        action = new PlacesModel::ItemAction(item->index(), tr("Open in New Tab"), menu);
        connect(action, &QAction::triggered, this, &PlacesView::onOpenNewTab);
        menu->addAction(action);

        if(isRegularPlace) {
          action = new PlacesModel::ItemAction(item->index(), tr("Show in Enclosing Folder"), menu);
          connect(action, &QAction::triggered, this, &PlacesView::onShowInEnclosingFolder);
          menu->addAction(action);
        }

        menu->addSeparator();

        if(!item->id().isEmpty()) {
          action = new PlacesModel::ItemAction(item->index(), tr("Remove from Sidebar"), menu);
          connect(action, &QAction::triggered, this, &PlacesView::onRemoveFromSidebar);
          menu->addAction(action);
          menu->addSeparator();
        }

        action = new PlacesModel::ItemAction(item->index(), tr("Get Info"), menu);
        connect(action, &QAction::triggered, this, &PlacesView::onGetInfo);
        menu->addAction(action);

        if(isRegularPlace) {
          action = new PlacesModel::ItemAction(item->index(), tr("Add to Dock"), menu);
          connect(action, &QAction::triggered, this, &PlacesView::onAddToDock);
          menu->addAction(action);
        }

        FmPath* path = item->path();
        FmPath* trashPath;
        trashPath = fm_path_new_for_str(QString(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash/files").toUtf8());
        if(path && fm_path_equal(trashPath, path)) {
          action = new PlacesModel::ItemAction(item->index(), tr("Empty Trash"), menu);
          connect(action, &QAction::triggered, this, &Fm::Trash::emptyTrash);
          menu->addAction(action);
        }
        fm_path_unref(trashPath);
        break;
      }
      case PlacesModelItem::Volume: {
        PlacesModelVolumeItem* volumeItem = static_cast<PlacesModelVolumeItem*>(item);

        if(volumeItem->isMounted()) {
          action = new PlacesModel::ItemAction(item->index(), tr("Unmount"), menu);
          connect(action, &QAction::triggered, this, &PlacesView::onUnmountVolume);
        }
        else {
          action = new PlacesModel::ItemAction(item->index(), tr("Mount"), menu);
          connect(action, &QAction::triggered, this, &PlacesView::onMountVolume);
        }
        menu->addAction(action);

        if(volumeItem->canEject()) {
          action = new PlacesModel::ItemAction(item->index(), tr("Eject"), menu);
          connect(action, &QAction::triggered, this, &PlacesView::onEjectVolume);
          menu->addAction(action);
        }
        break;
      }
      case PlacesModelItem::Mount: {
        action = new PlacesModel::ItemAction(item->index(), tr("Unmount"), menu);
        connect(action, &QAction::triggered, this, &PlacesView::onUnmountMount);
        menu->addAction(action);
        break;
      }
      case PlacesModelItem::DmgVolume: {
        action = new PlacesModel::ItemAction(item->index(), tr("Eject \"%1\"").arg(item->text()), menu);
        connect(action, &QAction::triggered, this, [this, item]() { onEjectButtonClicked(item); });
        menu->addAction(action);
        break;
      }
    }
    if(menu->actions().size()) {
      menu->popup(mapToGlobal(event->pos()));
      connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    } else {
        menu->deleteLater();
    }
  }
}

QItemSelectionModel::SelectionFlags PlacesView::selectionCommand(const QModelIndex& index, const QEvent* event) const {
  if (index.isValid() && !index.parent().isValid()) {
    QStandardItem* rawItem = model_->itemFromIndex(index);
    if (rawItem && (rawItem == model_->placesRoot || rawItem == model_->devicesRoot || rawItem == model_->tagsRoot)) {
      return QItemSelectionModel::NoUpdate;
    }
  }
  return QTreeView::selectionCommand(index, event);
}

void PlacesView::updateSpanning(const QModelIndex& parent) {
  int numRows = model_->rowCount(parent);
  for(int i = 0; i < numRows; ++i) {
    QModelIndex idx = model_->index(i, 0, parent);
    QModelIndex col1 = model_->index(i, 1, parent);
    bool hasCol1 = col1.isValid() && model_->data(col1).isValid();
    setFirstColumnSpanned(i, parent, !hasCol1);
    if(model_->hasChildren(idx)) {
      updateSpanning(idx);
    }
  }
}

