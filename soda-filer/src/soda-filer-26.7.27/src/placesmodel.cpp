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


#include "placesmodel.h"
#include "icontheme.h"
#include "tags.h"
#include <gio/gio.h>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QGuiApplication>
#include <QMimeData>
#include <QStandardPaths>
#include <QSettings>
#include <QTimer>
#include <QDir>
#include <QPainter>
#include <QPixmap>
#include <QImageReader>
#include "utilities.h"
#include "placesmodelitem.h"
#include "recentfilesmanager.h"

using namespace Fm;

// Section header row spec (uikit design reference,
// ~/Desktop/uikit-svg/examples/fullwindow.css "Section Header"/"Section"):
// bold 11px text, 39px-tall row (~17px breathing room above the label,
// ~8px below). setSizeHint() is used instead of QSS padding because the
// asymmetric top/bottom split isn't expressible as a single item-view
// padding rule, and per-item QFont/size-hint overrides win over the
// shared QSS rule that sizes regular rows (verified: QSS font-size does
// not clobber a per-item QStandardItem::setFont()/setSizeHint()).
static void styleSectionHeader(QStandardItem* header) {
  // Same "Mutern VF" family as the regular sidebar item rows (see
  // buildSidePaneStyleSheet() in sidepane.cpp) and desktop icon labels
  // (DesktopItemDelegate) -- headers just request a heavier weight so
  // fontconfig picks a bolder named instance than the items' own.
  QFont f(QStringLiteral("Mutern VF"));
  f.setWeight(QFont::DemiBold);
  f.setPixelSize(11);
  header->setFont(f);
  // Row height (the gap around each category) is set separately by
  // PlacesModel::setCategorySpacing() -- see Settings::sidePaneCategorySpacing().
  // #8A8A8A: dark-mode section header color (the system runs pearOS-dark;
  // see ~/Desktop/uikit-svg/sidebars/sidebar_dark_example.css), distinct
  // from regular items' #F5F5F5 label color in sidepane.cpp.
  header->setForeground(QColor("#8A8A8A"));
}

// The bundled sidebar SVGs (recents.svg, desktop.svg, etc.) each carry their
// own hardcoded fill color; recolor them flat with the active accent color
// (QPalette::Highlight -- the same "accent color" source folderitemdelegate.cpp
// already follows for selected-item subtext), matching Finder, whose sidebar
// glyphs are tinted with the system accent color instead of each icon's
// individual brand color. The "Locations" section (pCloud Drive, home,
// PearDrop, Trash) intentionally does NOT follow the accent color -- it's
// tinted a fixed light gray instead, to visually separate "your stuff"
// (Recents..Downloads) from "places on disk" (Locations).
static const QColor kLocationsIconColor(181, 181, 181);

static QIcon themedSidebarIcon(const QString& svgPath, const QColor& color) {
  // Most of the bundled sidebar SVGs bake in a tiny explicit width/height
  // (e.g. recents.svg is "15x15"); QPixmap(svgPath) rasterizes at exactly
  // that native size, and QIcon's pixmap engine caps actualSize() to a
  // raster pixmap's native resolution -- it will never scale UP past it,
  // only down. So with Settings::sidePaneIconSize() set higher than ~15,
  // these icons silently stayed capped at their tiny native size no matter
  // what the sidebar's iconSize() was (peardrop.svg has no explicit
  // width/height, only a viewBox, so it happened to rasterize much larger
  // and wasn't affected -- which is what made this look inconsistent).
  // Rasterizing well above any realistic sidebar icon size up front lets
  // the icon scale DOWN cleanly to whatever size is actually configured.
  static const int kRasterSize = 256;
  QImageReader reader(svgPath);
  QSize nativeSize = reader.size();
  if(nativeSize.isValid() && !nativeSize.isEmpty())
    reader.setScaledSize(nativeSize.scaled(kRasterSize, kRasterSize, Qt::KeepAspectRatio));
  QImage image = reader.read();
  if(image.isNull())
    return QIcon(svgPath);

  // Center onto a square transparent canvas so every sidebar icon lines up
  // on the same visual center regardless of its own SVG's aspect ratio --
  // e.g. documents.svg's viewBox is narrower (12x15) than most of the
  // others (~15-19px wide), which without this left it looking shifted by
  // a pixel or two relative to its neighbors once scaled to iconSize.
  QImage canvas(kRasterSize, kRasterSize, QImage::Format_ARGB32_Premultiplied);
  canvas.fill(Qt::transparent);
  QPainter canvasPainter(&canvas);
  canvasPainter.drawImage(QPoint((kRasterSize - image.width()) / 2, (kRasterSize - image.height()) / 2), image);
  canvasPainter.end();

  QPixmap pixmap = QPixmap::fromImage(canvas);
  QPainter painter(&pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(pixmap.rect(), color);
  painter.end();
  return QIcon(pixmap);
}

// Same 2px-inset-on-16px ratio as the original hardcoded dot, scaled to an
// arbitrary size -- used both at construction and by setTagIconSize() so
// the "Tags" section's color dots track Settings::sidePaneIconSize() live
// like every other sidebar icon, instead of staying a fixed 16x16.
static QPixmap tagDotPixmap(const QColor& color, int size) {
  QPixmap pix(size, size);
  pix.fill(Qt::transparent);
  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setBrush(color);
  painter.setPen(Qt::NoPen);
  qreal inset = size * 0.125;
  qreal diameter = size * 0.75;
  painter.drawEllipse(QRectF(inset, inset, diameter, diameter));
  painter.end();
  return pix;
}

// QGuiApplication::palette().color(QPalette::Highlight) depends on Qt's
// platform theme plugin actually pushing a live palette update when Plasma's
// accent color changes -- on this setup that didn't reliably happen (icons
// stayed tinted with a stale/previous accent after switching, e.g. to
// green, in System Settings), even with IconTheme's existing 300ms delay
// past the kdeglobals write. Reading the accent color straight out of
// kdeglobals sidesteps that relay entirely: [General]/AccentColor is the
// explicit override key Plasma writes when the user picks a custom accent;
// [Colors:Selection]/Background is the color-scheme-derived one used when
// no override is set. Falls back to the (possibly stale) palette only if
// neither key is present, so this still degrades gracefully off Plasma.
static QColor currentAccentColor() {
  QString kdeglobals = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kdeglobals");
  QSettings settings(kdeglobals, QSettings::IniFormat);
  // QSettings' ini parser auto-detects a comma-separated value (exactly
  // what these keys store, e.g. "173,80,51") as a QStringList rather than a
  // plain string -- calling .toString() straight on that QVariant silently
  // returns an empty string (there's no direct QStringList -> QString
  // conversion), which always looked like a missing/empty key regardless of
  // what was actually on disk. Reading it as a QStringList first, and only
  // falling back to a plain string for a key that genuinely holds one
  // (e.g. some other install's AccentColor stored as a single hex string),
  // handles both.
  auto readTriplet = [&settings](const QString& key) -> QString {
    QVariant v = settings.value(key);
    if(v.type() == QVariant::StringList) {
      QStringList list = v.toStringList();
      return list.size() == 3 ? list.join(QLatin1Char(',')) : QString();
    }
    return v.toString();
  };
  QString raw = readTriplet(QStringLiteral("General/AccentColor"));
  if(raw.isEmpty())
    // Not "Background" -- kdeglobals' actual key here is BackgroundNormal
    // (confirmed against a real [Colors:Selection] section on this system).
    raw = readTriplet(QStringLiteral("Colors:Selection/BackgroundNormal"));
  const QStringList parts = raw.split(QLatin1Char(','));
  if(parts.size() == 3) {
    bool okR = false, okG = false, okB = false;
    int r = parts.at(0).trimmed().toInt(&okR);
    int g = parts.at(1).trimmed().toInt(&okG);
    int b = parts.at(2).trimmed().toInt(&okB);
    if(okR && okG && okB)
      return QColor(r, g, b);
  }
  return QGuiApplication::palette().color(QPalette::Highlight);
}

static QIcon themedSidebarIcon(const QString& svgPath) {
  return themedSidebarIcon(svgPath, currentAccentColor());
}

PlacesModel::PlacesModel(QObject* parent):
  QStandardItemModel(parent),
  showApplications_(true),
  showDesktop_(true),
  ejectIcon_(QIcon::fromTheme("media-eject")) {

  setColumnCount(2);

  // ── 1. Recents (standalone top-level item, no section header) ──
  // Points at a directory of symlinks kept in sync by RecentFilesManager
  // as files are opened, so the existing FmFolder/FolderModel machinery can
  // browse it like any other folder.
  FmPath* recentsPath = fm_path_new_for_str(
      Filer::RecentFilesManager::instance().recentDirPath().toUtf8());
  recentsItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/recents.svg"), tr("Recents"), recentsPath);
  recentsItem->setId(QStringLiteral("recents"));
  recentsItem->setFixed(true);
  fm_path_unref(recentsPath);
  appendRow(recentsItem);
  themedIconItems_.append(qMakePair(recentsItem, QStringLiteral(":/icons/sidebar/recents.svg")));

  // ── 2. Shared (standalone top-level item) ──
  // Shared is a placeholder — no real path yet; clicking it is a no-op
  sharedItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/shared.svg"), tr("Shared"), NULL);
  sharedItem->setId(QStringLiteral("shared"));
  sharedItem->setFixed(true);
  appendRow(sharedItem);
  themedIconItems_.append(qMakePair(sharedItem, QStringLiteral(":/icons/sidebar/shared.svg")));

  // ── 3. Favorites section ──
  placesRoot = new QStandardItem(tr("Favorites"));
  placesRoot->setSelectable(false);
  placesRoot->setColumnCount(2);
  styleSectionHeader(placesRoot);
  appendRow(placesRoot);

  // Applications → /Applications (or ~/Applications fallback)
  FmPath* appsPath;
  if(QDir("/Applications").exists())
    appsPath = fm_path_new_for_str("/Applications");
  else
    appsPath = fm_path_new_for_str(
        QString(QDir::homePath() + "/Applications").toUtf8());
  applicationsItem = new PlacesModelItem(themedSidebarIcon(":/icons/applications.svg"), tr("Applications"), appsPath);
  applicationsItem->setId(QStringLiteral("applications"));
  fm_path_unref(appsPath);
  themedIconItems_.append(qMakePair(applicationsItem, QStringLiteral(":/icons/applications.svg")));
  placesRoot->appendRow(applicationsItem);

  // Desktop
  desktopItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/desktop.svg"), tr("Desktop"), fm_path_get_desktop());
  desktopItem->setId(QStringLiteral("desktop"));
  placesRoot->appendRow(desktopItem);
  themedIconItems_.append(qMakePair(desktopItem, QStringLiteral(":/icons/sidebar/desktop.svg")));

  // Documents
  FmPath* docsPath = fm_path_new_for_str(
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toUtf8());
  documentsItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/documents.svg"), tr("Documents"), docsPath);
  documentsItem->setId(QStringLiteral("documents"));
  fm_path_unref(docsPath);
  placesRoot->appendRow(documentsItem);
  themedIconItems_.append(qMakePair(documentsItem, QStringLiteral(":/icons/sidebar/documents.svg")));

  // Downloads
  FmPath* dlPath = fm_path_new_for_str(
      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toUtf8());
  downloadsItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/downloads.svg"), tr("Downloads"), dlPath);
  downloadsItem->setId(QStringLiteral("downloads"));
  fm_path_unref(dlPath);
  placesRoot->appendRow(downloadsItem);
  themedIconItems_.append(qMakePair(downloadsItem, QStringLiteral(":/icons/sidebar/downloads.svg")));

  // ── 4. Locations section ──
  devicesRoot = new QStandardItem(tr("Locations"));
  devicesRoot->setSelectable(false);
  devicesRoot->setColumnCount(2);
  styleSectionHeader(devicesRoot);

  // pCloud Drive
  QString pcloudDir = QDir::homePath() + "/pCloudDrive";
  if(QDir(pcloudDir).exists()) {
    FmPath* pcloudPath = fm_path_new_for_str(pcloudDir.toUtf8());
    pcloudItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/pcloud.svg", kLocationsIconColor), tr("pCloud Drive"), pcloudPath);
    fm_path_unref(pcloudPath);
  } else {
    // Create the item even if the dir doesn't exist so the sidebar
    // always shows the expected layout.
    FmPath* pcloudPath = fm_path_new_for_str(pcloudDir.toUtf8());
    pcloudItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/pcloud.svg", kLocationsIconColor), tr("pCloud Drive"), pcloudPath);
    fm_path_unref(pcloudPath);
  }
  pcloudItem->setId(QStringLiteral("pcloud"));
  pcloudItem->setFixed(true);
  devicesRoot->appendRow(pcloudItem);
  grayIconItems_.append(qMakePair(pcloudItem, QStringLiteral(":/icons/sidebar/pcloud.svg")));

  // Home directory
  homeItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/userhome.svg", kLocationsIconColor), g_get_user_name(), fm_path_get_home());
  homeItem->setId(QStringLiteral("home"));
  devicesRoot->appendRow(homeItem);
  grayIconItems_.append(qMakePair(homeItem, QStringLiteral(":/icons/sidebar/userhome.svg")));

  // PearDrop (placeholder — like AirDrop, no real path)
  peardropItem = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/peardrop.svg", kLocationsIconColor), tr("PearDrop"), NULL);
  peardropItem->setId(QStringLiteral("peardrop"));
  peardropItem->setFixed(true);
  devicesRoot->appendRow(peardropItem);
  grayIconItems_.append(qMakePair(peardropItem, QStringLiteral(":/icons/sidebar/peardrop.svg")));

  // Trash
  trashMonitor_ = NULL;
  trashItem_ = NULL;
  createTrashItem();

  // volumes
  volumeMonitor = g_volume_monitor_get();
  if(volumeMonitor) {
    g_signal_connect(volumeMonitor, "volume-added", G_CALLBACK(onVolumeAdded), this);
    g_signal_connect(volumeMonitor, "volume-removed", G_CALLBACK(onVolumeRemoved), this);
    g_signal_connect(volumeMonitor, "volume-changed", G_CALLBACK(onVolumeChanged), this);
    g_signal_connect(volumeMonitor, "mount-added", G_CALLBACK(onMountAdded), this);
    g_signal_connect(volumeMonitor, "mount-changed", G_CALLBACK(onMountChanged), this);
    g_signal_connect(volumeMonitor, "mount-removed", G_CALLBACK(onMountRemoved), this);

    // add volumes to side-pane
    GList* vols = g_volume_monitor_get_volumes(volumeMonitor);
    GList* l;
    for(l = vols; l; l = l->next) {
        GVolume* volume = G_VOLUME(l->data);
        onVolumeAdded(volumeMonitor, volume, this);
        g_object_unref(volume);
    }
    g_list_free(vols);

    /* add mounts to side-pane */
    vols = g_volume_monitor_get_mounts(volumeMonitor);
    for(l = vols; l; l = l->next) {
        GMount* mount = G_MOUNT(l->data);
        GVolume* volume = g_mount_get_volume(mount);
        if(volume)
        g_object_unref(volume);
        else { /* network mounts or others */
        PlacesModelItem* item = new PlacesModelMountItem(mount);
        // Removable/network storage always sits above PearDrop, no matter
        // when it's mounted relative to app startup.
        devicesRoot->insertRow(peardropItem->row(), item);
        }
        g_object_unref(mount);
    }
    g_list_free(vols);
  }

  appendRow(devicesRoot);

  computerItem = NULL;
  networkItem = NULL;

  // ── 5. Tags section ──
  tagsRoot = new QStandardItem(tr("Tags"));
  tagsRoot->setSelectable(false);
  tagsRoot->setColumnCount(2);
  styleSectionHeader(tagsRoot);
  appendRow(tagsRoot);

  // Add colour tag items from the tag palette (Red, Orange, Yellow, Green, Purple, Gray)
  // We skip Blue from the palette to match the user's requested list
  const QVector<TagInfo>& palette = tagPalette();
  for(const TagInfo& tag : palette) {
    // The user requested: Red, Orange, Yellow, Green, Purple, Gray
    // The palette has Blue too — skip it
    if(tag.name == QStringLiteral("Blue"))
      continue;

    PlacesModelItem* tagItem = new PlacesModelItem(QIcon(tagDotPixmap(tag.color, tagIconSize_)), tag.name, NULL);
    tagItem->setEditable(false);
    tagsRoot->appendRow(tagItem);
    tagIconItems_.append(qMakePair(tagItem, tag.color));
  }

  // "All Tags..." entry
  PlacesModelItem* allTagsItem = new PlacesModelItem(
      QIcon::fromTheme("tag"), tr("All Tags..."), NULL);
  allTagsItem->setEditable(false);
  tagsRoot->appendRow(allTagsItem);

  // update some icons when the icon theme is changed
  connect(IconTheme::instance(), &IconTheme::changed, this, &PlacesModel::updateIcons);
  // Also re-tint the accent-colored sidebar SVGs when kdeglobals changes even
  // if the icon theme *name* didn't (e.g. only the accent color changed).
  connect(IconTheme::instance(), &IconTheme::configFileChanged, this, &PlacesModel::updateIcons);

  // DMG volumes (see dmgmountregistry.h/filelauncher.cpp) -- picks up any
  // already open at construction time, then tracks new ones live.
  Filer::DmgMountRegistry& dmgRegistry = Filer::DmgMountRegistry::instance();
  connect(&dmgRegistry, &Filer::DmgMountRegistry::mountAdded, this, &PlacesModel::onDmgMountAdded);
  connect(&dmgRegistry, &Filer::DmgMountRegistry::mountRemoved, this, &PlacesModel::onDmgMountRemoved);
  for(const Filer::DmgMount& mount : dmgRegistry.mounts())
    onDmgMountAdded(mount);

  setCategorySpacing(17, 8); // matches Settings::sidePaneCategorySpacing()/sidePaneCategorySpacingBottom()'s defaults
}

void PlacesModel::setCategorySpacing(int top, int bottom) {
  categorySpacingTop_ = top;
  categorySpacingBottom_ = bottom;
  applyCategoryLayout();
}

void PlacesModel::setCategoryFontSize(int size) {
  QFont f = placesRoot->font();
  f.setPixelSize(size);
  placesRoot->setFont(f);
  devicesRoot->setFont(f);
  tagsRoot->setFont(f);
  applyCategoryLayout();
}

void PlacesModel::applyCategoryLayout() {
  QFontMetrics fm(placesRoot->font());
  QSize size(-1, categorySpacingTop_ + fm.height() + categorySpacingBottom_);
  placesRoot->setSizeHint(size);
  devicesRoot->setSizeHint(size);
  tagsRoot->setSizeHint(size);
}

void PlacesModel::onDmgMountAdded(const Filer::DmgMount& mount) {
  if(dmgVolumeItems_.contains(mount.path))
    return;
  FmPath* path = fm_path_new_for_str(mount.path.toUtf8());
  PlacesModelItem* item = new PlacesModelDmgVolumeItem(mount.name, path);
  fm_path_unref(path);
  QStandardItem* ejectBtn = new QStandardItem(ejectIcon_, "");
  // Removable/network storage (and DMG volumes alike) always sit above
  // PearDrop, no matter when they're mounted relative to app startup.
  devicesRoot->insertRow(peardropItem->row(), QList<QStandardItem*>() << item << ejectBtn);
  dmgVolumeItems_.insert(mount.path, item);
}

void PlacesModel::onDmgMountRemoved(const QString& path) {
  PlacesModelItem* item = dmgVolumeItems_.take(path);
  if(item)
    devicesRoot->removeRow(item->row());
}

void PlacesModel::createTrashItem() {
  GFile* gf;
  // gf = fm_file_new_for_uri("trash:///");
  gf = fm_file_new_for_commandline_arg(QString(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash/files").toUtf8());
  // check if trash is supported by the current vfs
  // if gvfs is not installed, this can be unavailable.
  if(!g_file_query_exists(gf, NULL)) {
    g_object_unref(gf);
    trashItem_ = NULL;
    trashMonitor_ = NULL;
    return;
  }

  FmPath* path;
  path = fm_path_new_for_str(QString(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash/files").toUtf8());
  // trashItem_ = new PlacesModelItem("user-trash", tr("Trash"), fm_path_get_trash()); // Do not use trash://
  trashItem_ = new PlacesModelItem(themedSidebarIcon(":/icons/sidebar/trash.svg", kLocationsIconColor), tr("Trash"), path);
  trashItem_->setId(QStringLiteral("trash"));
  trashItem_->setFixed(true);
  fm_path_unref(path);

  trashMonitor_ = fm_monitor_directory(gf, NULL);
  if(trashMonitor_)
    g_signal_connect(trashMonitor_, "changed", G_CALLBACK(onTrashChanged), this);
  g_object_unref(gf);

  devicesRoot->appendRow(trashItem_);
  QTimer::singleShot(0, this, SLOT(updateTrash()));
}

PlacesModel::~PlacesModel() {
  if(volumeMonitor) {
    g_signal_handlers_disconnect_by_func(volumeMonitor, (gpointer)G_CALLBACK(onVolumeAdded), this);
    g_signal_handlers_disconnect_by_func(volumeMonitor, (gpointer)G_CALLBACK(onVolumeRemoved), this);
    g_signal_handlers_disconnect_by_func(volumeMonitor, (gpointer)G_CALLBACK(onVolumeChanged), this);
    g_signal_handlers_disconnect_by_func(volumeMonitor, (gpointer)G_CALLBACK(onMountAdded), this);
    g_signal_handlers_disconnect_by_func(volumeMonitor, (gpointer)G_CALLBACK(onMountChanged), this);
    g_signal_handlers_disconnect_by_func(volumeMonitor, (gpointer)G_CALLBACK(onMountRemoved), this);
    g_object_unref(volumeMonitor);
  }
  if(trashMonitor_) {
    g_signal_handlers_disconnect_by_func(trashMonitor_, (gpointer)G_CALLBACK(onTrashChanged), this);
    g_object_unref(trashMonitor_);
  }
}

// static
// probno: TODO: Replace GFileMonitor with QFileSystemWatcher
void PlacesModel::onTrashChanged(GFileMonitor* monitor, GFile* gf, GFile* other, GFileMonitorEvent evt, PlacesModel* pThis) {
  QTimer::singleShot(0, pThis, SLOT(updateTrash()));
}

void PlacesModel::updateTrash() {
  // Trash item count is no longer used to swap the icon: the uikit design
  // reference (~/Desktop/uikit-svg) has no "full trash" variant, so
  // trashItem_ just keeps the SVG icon it was created with.
}

void PlacesModel::setShowApplications(bool show) {
  if(showApplications_ != show) {
    showApplications_ = show;
  }
}

void PlacesModel::setShowDesktop(bool show) {
  if(showDesktop_ != show) {
    showDesktop_ = show;
  }
}

void PlacesModel::setShowTrash(bool show) {
  if(show) {
    if(!trashItem_)
      createTrashItem();
  }
  else {
    if(trashItem_) {
      if(trashMonitor_) {
        g_signal_handlers_disconnect_by_func(trashMonitor_, (gpointer)G_CALLBACK(onTrashChanged), this);
        g_object_unref(trashMonitor_);
        trashMonitor_ = NULL;
      }
      devicesRoot->removeRow(trashItem_->row()); // delete trashItem_;
      trashItem_ = NULL;
    }
  }
}

PlacesModelItem* PlacesModel::itemFromPath(FmPath* path) {
  if(!path) return NULL;
  PlacesModelItem* item = itemFromPath(placesRoot, path);
  if(!item)
    item = itemFromPath(devicesRoot, path);
  // Also check the standalone top-level items
  if(!item && recentsItem && recentsItem->path() && fm_path_equal(recentsItem->path(), path))
    item = recentsItem;
  if(!item && sharedItem && sharedItem->path() && fm_path_equal(sharedItem->path(), path))
    item = sharedItem;
  return item;
}

PlacesModelItem* PlacesModel::itemFromPath(QStandardItem* rootItem, FmPath* path) {
  if(!rootItem || !path) return NULL;
  int rowCount = rootItem->rowCount();
  for(int i = 0; i < rowCount; ++i) {
    QStandardItem* childItem = rootItem->child(i, 0);
    if(!childItem) continue;
    PlacesModelItem* item = static_cast<PlacesModelItem*>(childItem);
    if(item->path() && fm_path_equal(item->path(), path))
      return item;
  }
  return NULL;
}

PlacesModelVolumeItem* PlacesModel::itemFromVolume(GVolume* volume) {
  if(!volume || !devicesRoot) return NULL;
  int rowCount = devicesRoot->rowCount();
  for(int i = 0; i < rowCount; ++i) {
    QStandardItem* childItem = devicesRoot->child(i, 0);
    if(!childItem) continue;
    PlacesModelItem* item = static_cast<PlacesModelItem*>(childItem);
    if(item->type() == PlacesModelItem::Volume) {
      PlacesModelVolumeItem* volumeItem = static_cast<PlacesModelVolumeItem*>(item);
      if(volumeItem->volume() == volume)
        return volumeItem;
    }
  }
  return NULL;
}

PlacesModelMountItem* PlacesModel::itemFromMount(GMount* mount) {
  if(!mount || !devicesRoot) return NULL;
  int rowCount = devicesRoot->rowCount();
  for(int i = 0; i < rowCount; ++i) {
    QStandardItem* childItem = devicesRoot->child(i, 0);
    if(!childItem) continue;
    PlacesModelItem* item = static_cast<PlacesModelItem*>(childItem);
    if(item->type() == PlacesModelItem::Mount) {
      PlacesModelMountItem* mountItem = static_cast<PlacesModelMountItem*>(item);
      if(mountItem->mount() == mount)
        return mountItem;
    }
  }
  return NULL;
}

void PlacesModel::onMountAdded(GVolumeMonitor* monitor, GMount* mount, PlacesModel* pThis) {
  GVolume* vol = g_mount_get_volume(mount);
  if(vol) { // mount-added is also emitted when a volume is newly mounted.
    PlacesModelVolumeItem* item = pThis->itemFromVolume(vol);
    if(item && !item->path()) {
      // update the mounted volume and show a button for eject.
      GFile* gf = g_mount_get_root(mount);
      FmPath* path = fm_path_new_for_gfile(gf);
      g_object_unref(gf);
      item->setPath(path);
      if(path)
        fm_path_unref(path);
      // update the mount indicator (eject button)
      QStandardItem* ejectBtn = item->parent()->child(item->row(), 1);
      Q_ASSERT(ejectBtn);
      ejectBtn->setIcon(pThis->ejectIcon_);
    }
    g_object_unref(vol);
  }
  else { // network mounts and others
    PlacesModelMountItem* item = pThis->itemFromMount(mount);
    /* for some unknown reasons, sometimes we get repeated mount-added
     * signals and added a device more than one. So, make a sanity check here. */
    if(!item) {
      item = new PlacesModelMountItem(mount);
      QStandardItem* eject_btn = new QStandardItem(pThis->ejectIcon_, "");
      // Removable/network storage always sits above PearDrop, no matter
      // when it's mounted relative to app startup.
      pThis->devicesRoot->insertRow(pThis->peardropItem->row(), QList<QStandardItem*>() << item << eject_btn);
    }
  }
}

void PlacesModel::onMountChanged(GVolumeMonitor* monitor, GMount* mount, PlacesModel* pThis) {
  PlacesModelMountItem* item = pThis->itemFromMount(mount);
  if(item)
    item->update();
}

void PlacesModel::onMountRemoved(GVolumeMonitor* monitor, GMount* mount, PlacesModel* pThis) {
  GVolume* vol = g_mount_get_volume(mount);
  qDebug() << "volume umounted: " << vol;
  if(vol) {
    // a volume is unmounted
    g_object_unref(vol);
  }
  else { // network mounts and others
    PlacesModelMountItem* item = pThis->itemFromMount(mount);
    if(item) {
      pThis->devicesRoot->removeRow(item->row());
    }
  }
}

void PlacesModel::onVolumeAdded(GVolumeMonitor* monitor, GVolume* volume, PlacesModel* pThis) {
  // for some unknown reasons, sometimes we get repeated volume-added
  // signals and added a device more than one. So, make a sanity check here.
  PlacesModelVolumeItem* volumeItem = pThis->itemFromVolume(volume);
  if(!volumeItem) {
    volumeItem = new PlacesModelVolumeItem(volume);
    QStandardItem* ejectBtn = new QStandardItem();
    if(volumeItem->isMounted())
      ejectBtn->setIcon(pThis->ejectIcon_);
    // Removable/network storage always sits above PearDrop, no matter
    // when it's mounted relative to app startup.
    pThis->devicesRoot->insertRow(pThis->peardropItem->row(), QList<QStandardItem*>() << volumeItem << ejectBtn);
  }
}

void PlacesModel::onVolumeChanged(GVolumeMonitor* monitor, GVolume* volume, PlacesModel* pThis) {
  PlacesModelVolumeItem* item = pThis->itemFromVolume(volume);
  if(item) {
    item->update();
    if(!item->isMounted()) { // the volume is unmounted, remove the eject button if needed
      // remove the eject button for the volume (at column 1 of the same row)
      QStandardItem* ejectBtn = item->parent()->child(item->row(), 1);
      Q_ASSERT(ejectBtn);
      ejectBtn->setIcon(QIcon());
    }
  }
}

void PlacesModel::onVolumeRemoved(GVolumeMonitor* monitor, GVolume* volume, PlacesModel* pThis) {
  PlacesModelVolumeItem* item = pThis->itemFromVolume(volume);
  if(item) {
    pThis->devicesRoot->removeRow(item->row());
  }
}

void PlacesModel::setTagIconSize(int size) {
  if(size <= 0 || size == tagIconSize_)
    return;
  tagIconSize_ = size;
  for(const auto& pair : qAsConst(tagIconItems_)) {
    pair.first->QStandardItem::setIcon(QIcon(tagDotPixmap(pair.second, tagIconSize_)));
  }
}

void PlacesModel::updateIcons() {
  // the icon theme is changed and we need to update the icons
  PlacesModelItem* item;
  int row;
  int n = placesRoot->rowCount();
  for(row = 0; row < n; ++row) {
    QStandardItem* childItem = placesRoot->child(row);
    if(childItem) {
      item = static_cast<PlacesModelItem*>(childItem);
      item->updateIcon();
    }
  }
  n = devicesRoot->rowCount();
  for(row = 0; row < n; ++row) {
    QStandardItem* childItem = devicesRoot->child(row);
    if(childItem) {
      item = static_cast<PlacesModelItem*>(childItem);
      item->updateIcon();
    }
  }

  // PlacesModelItem::updateIcon() only re-resolves FmIcon-backed icons; the
  // bundled monochrome sidebar SVGs are tinted with the accent color
  // (QPalette::Highlight) instead and need to be regenerated by hand so they
  // follow accent color changes live (see themedSidebarIcon()).
  for(const auto& pair : qAsConst(themedIconItems_)) {
    // PlacesModelItem::setIcon() is overloaded for FmIcon*/GIcon* only,
    // which hides QStandardItem::setIcon(const QIcon&); call it explicitly.
    pair.first->QStandardItem::setIcon(themedSidebarIcon(pair.second));
  }
  // Locations-section icons are tinted a fixed gray rather than the accent
  // color, so re-tint them with kLocationsIconColor instead (accent-color
  // changes shouldn't affect them, but re-running is cheap and keeps this
  // symmetric with the accent-colored loop above).
  for(const auto& pair : qAsConst(grayIconItems_)) {
    pair.first->QStandardItem::setIcon(themedSidebarIcon(pair.second, kLocationsIconColor));
  }
  if(trashItem_) {
    trashItem_->QStandardItem::setIcon(themedSidebarIcon(QStringLiteral(":/icons/sidebar/trash.svg"), kLocationsIconColor));
  }
}

Qt::ItemFlags PlacesModel::flags(const QModelIndex& index) const {
  if(index.column() == 1) // make 2nd column of every row selectable.
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if(!index.parent().isValid()) { // root items
    QStandardItem* item = itemFromIndex(index);
    // Section headers (Favorites, Locations, Tags) are disabled (just labels)
    if(item == placesRoot || item == devicesRoot || item == tagsRoot)
      return Qt::NoItemFlags;
    // Standalone top-level items (Recents, Shared) are clickable
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }
  return QStandardItemModel::flags(index);
}

bool PlacesModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
  // With bookmarks section removed, we only handle file drops onto items
  QStandardItem* item = itemFromIndex(parent);
  if(data->hasUrls()) {
    if(row == -1 && column == -1) { // drop uris on an item
      if(item && item->parent()) { // need to be a child item
        PlacesModelItem* placesItem = static_cast<PlacesModelItem*>(item);
        if(placesItem->path()) {
          qDebug() << "dropped dest:" << placesItem->text();
          // TODO: copy or move the dragged files to the dir pointed by the item.
          qDebug() << "drop on" << item->text();
        }
      }
    }
  }
  return false;
}

QMimeData* PlacesModel::mimeData(const QModelIndexList& indexes) const {
  return NULL;
}

QStringList PlacesModel::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

Qt::DropActions PlacesModel::supportedDropActions() const {
  return QStandardItemModel::supportedDropActions();
}
