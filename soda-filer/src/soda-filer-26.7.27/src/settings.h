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


#ifndef PCMANFM_SETTINGS_H
#define PCMANFM_SETTINGS_H

#include <QObject>
#include <libfm/fm.h>
#include "folderview.h"
#include "foldermodel.h"
#include "desktopwindow.h"
#include "sidepane.h"
#include "thumbnailloader.h"

namespace Filer {

enum OpenDirTargetType {
    OpenInCurrentTab,
    OpenInNewTab,
    OpenInNewWindow,
    OpenInLastActiveWindow
};

// Sidebar scrollbar visibility policy -- see SidePane::applyScrollBarMode().
enum SidePaneScrollBarMode {
    ScrollBarAuto = 0,        // native "as needed" -- style/platform decides transient behavior for mouse vs. trackpad
    ScrollBarWhileScrolling,  // hidden at rest, appears while scrolling, auto-hides shortly after
    ScrollBarAlways
};

class Settings : public QObject {
  Q_OBJECT
public:
  Settings();
  virtual ~Settings();

  bool load(QString profile = "default");
  bool save(QString profile = QString());

  bool loadFile(QString filePath);
  bool saveFile(QString filePath);

  QString profileDir(QString profile, bool useFallback = false);

  // Re-reads just the sidebar/window wallpaper vibrancy keys (TintWindow,
  // SidebarTint, MainWindowTint) plus HiddenSidebarItems from the current
  // profile's settings.conf -- called by Application's file watcher so
  // editing the config file directly while Filer is running takes effect
  // immediately, without a full Settings::load() (which would also
  // re-trigger other settings' side effects, e.g. desktop/theme setup,
  // that a live file edit shouldn't repeat).
  //
  // HiddenSidebarItems specifically NEEDS this: without it, a manual edit
  // to that key while Filer is running gets silently reverted on quit --
  // Application::onAboutToQuit() unconditionally calls Settings::save(),
  // which re-serializes the whole in-memory Settings object (still holding
  // whatever HiddenSidebarItems was at startup) back over the file,
  // clobbering the edit.
  void reloadTintSettings();

  // Re-reads the sidebar layout keys (SidePaneIconSize, SidePaneItemSpacing,
  // SidePaneItemHorizontalPadding) from the current profile's settings.conf
  // -- same rationale/caller as reloadTintSettings(): lets Application's
  // file watcher apply a direct config-file edit live, without a full
  // Settings::load().
  void reloadSidePaneLayoutSettings();

  // setter/getter functions
  QString profileName() const {
    return profileName_;
  }

  bool supportTrash() const {
    return supportTrash_;
  }

  QString fallbackIconThemeName() const {
    return fallbackIconThemeName_;
  }

  bool useFallbackIconTheme() const {
    return useFallbackIconTheme_;
  }

  void setFallbackIconThemeName(QString iconThemeName) {
    fallbackIconThemeName_ = iconThemeName;
  }

  OpenDirTargetType bookmarkOpenMethod() {
    return bookmarkOpenMethod_;
  }

  void setBookmarkOpenMethod(OpenDirTargetType bookmarkOpenMethod) {
    bookmarkOpenMethod_ = bookmarkOpenMethod;
  }

  QString suCommand() const {
    return suCommand_;
  }

  void setSuCommand(QString suCommand) {
    suCommand_ = suCommand;
  }

  QString terminal() {
    return terminal_;
  }
  void setTerminal(QString terminalCommand);

  QString archiver() const {
    return archiver_;
  }

  void setArchiver(QString archiver) {
    archiver_ = archiver;
    // override libfm FmConfig
    g_free(fm_config->archiver);
    fm_config->archiver = g_strdup(archiver_.toLocal8Bit().constData());
  }

  bool mountOnStartup() const {
    return mountOnStartup_;
  }

  void setMountOnStartup(bool mountOnStartup) {
    mountOnStartup_ = mountOnStartup;
  }

  bool mountRemovable() {
    return mountRemovable_;
  }

  void setMountRemovable(bool mountRemovable) {
    mountRemovable_ = mountRemovable;
  }

  bool autoRun() const {
    return autoRun_;
  }

  void setAutoRun(bool autoRun) {
    autoRun_ = autoRun;
  }

  bool closeOnUnmount() const {
    return closeOnUnmount_;
  }

  void setCloseOnUnmount(bool value) {
    closeOnUnmount_ = value;
  }

  DesktopWindow::WallpaperMode wallpaperMode() const {
    return DesktopWindow::WallpaperMode(wallpaperMode_);
  }

  void setWallpaperMode(int wallpaperMode) {
    wallpaperMode_ = wallpaperMode;
  }

  QString wallpaper() const {
    return wallpaper_;
  }

  void setWallpaper(QString wallpaper) {
    wallpaper_ = wallpaper;
  }

  const QColor& desktopBgColor() const {
    return desktopBgColor_;
  }

  void setDesktopBgColor(QColor desktopBgColor) {
    desktopBgColor_ = desktopBgColor;
  }

  const QColor& desktopFgColor() const {
    return desktopFgColor_;
  }

  void setDesktopFgColor(QColor desktopFgColor) {
    desktopFgColor_ = desktopFgColor;
  }

  const QColor& desktopShadowColor() const {
    return desktopShadowColor_;
  }

  void setDesktopShadowColor(QColor desktopShadowColor) {
    desktopShadowColor_ = desktopShadowColor;
  }

  QFont desktopFont() const {
    return desktopFont_;
  }

  void setDesktopFont(QFont font) {
    desktopFont_ = font;
  }

  bool desktopShowHidden() const {
    return desktopShowHidden_;
  }

  void setDesktopShowHidden(bool desktopShowHidden) {
    desktopShowHidden_ = desktopShowHidden;
  }

  Qt::SortOrder desktopSortOrder() const {
    return desktopSortOrder_;
  }

  void setDesktopSortOrder(Qt::SortOrder desktopSortOrder) {
    desktopSortOrder_ = desktopSortOrder;
  }

  Fm::FolderModel::ColumnId desktopSortColumn() const {
    return desktopSortColumn_;
  }

  void setDesktopSortColumn(Fm::FolderModel::ColumnId desktopSortColumn) {
    desktopSortColumn_ = desktopSortColumn;
  }

  bool alwaysShowTabs() const {
    return alwaysShowTabs_;
  }

  void setAlwaysShowTabs(bool alwaysShowTabs) {
    alwaysShowTabs_ = alwaysShowTabs;
  }

  bool showTabClose() const {
    return showTabClose_;
  }

  void setShowTabClose(bool showTabClose) {
    showTabClose_ = showTabClose;
  }

  bool rememberWindowSize() const {
    return rememberWindowSize_;
  }

  void setRememberWindowSize(bool rememberWindowSize) {
    rememberWindowSize_ = rememberWindowSize;
  }

  int windowWidth() const {
    if(rememberWindowSize_)
      return lastWindowWidth_;
    else
      return fixedWindowWidth_;
  }

  int windowHeight() const {
    if(rememberWindowSize_)
      return lastWindowHeight_;
    else
      return fixedWindowHeight_;
  }

  bool windowMaximized() const {
    if(rememberWindowSize_)
      return lastWindowMaximized_;
    else
      return false;
  }

  int fixedWindowWidth() const {
    return fixedWindowWidth_;
  }

  void setFixedWindowWidth(int fixedWindowWidth) {
    fixedWindowWidth_ = fixedWindowWidth;
  }

  int fixedWindowHeight() const {
    return fixedWindowHeight_;
  }

  void setFixedWindowHeight(int fixedWindowHeight) {
    fixedWindowHeight_ = fixedWindowHeight;
  }

  void setLastWindowWidth(int lastWindowWidth) {
      lastWindowWidth_ = lastWindowWidth;
  }

  void setLastWindowHeight(int lastWindowHeight) {
      lastWindowHeight_ = lastWindowHeight;
  }

  void setLastWindowMaximized(bool lastWindowMaximized) {
      lastWindowMaximized_ = lastWindowMaximized;
  }

  // Sidebar/window wallpaper vibrancy (see MainWindow::updateSidebarWallpaperTint()
  // and paintEvent()). tintWindow() gates the whole feature off; the two
  // percentages independently control how strongly the sidebar column vs.
  // the rest of the window picks up the wallpaper color behind it.
  bool tintWindow() const {
    return tintWindow_;
  }

  void setTintWindow(bool tintWindow) {
    tintWindow_ = tintWindow;
  }

  int sidebarTint() const {
    return sidebarTint_;
  }

  void setSidebarTint(int sidebarTint) {
    sidebarTint_ = qBound(0, sidebarTint, 100);
  }

  // Height (px) of the blurred band across the full window width at the
  // very top, behind the traffic-light close/minimize/maximize buttons --
  // 0 disables it. See MainWindow::paintBlurBand().
  int windowTopBlurHeight() const {
    return windowTopBlurHeight_;
  }

  void setWindowTopBlurHeight(int height) {
    windowTopBlurHeight_ = qMax(0, height);
  }

  // Blur radius (px, QGraphicsBlurEffect) for the top blur band.
  int windowTopBlurPower() const {
    return windowTopBlurPower_;
  }
  void setWindowTopBlurPower(int power) {
    windowTopBlurPower_ = qMax(0, power);
  }

  // Opacity (0-255) of the top blur band.
  int windowTopBlurOpacity() const {
    return windowTopBlurOpacity_;
  }
  void setWindowTopBlurOpacity(int opacity) {
    windowTopBlurOpacity_ = qBound(0, opacity, 255);
  }

  // Color saturation (%, 100 = unchanged, 0 = grayscale, >100 = boosted)
  // applied to the top blur band's content before blurring.
  int windowTopBlurSaturation() const {
    return windowTopBlurSaturation_;
  }
  void setWindowTopBlurSaturation(int saturation) {
    windowTopBlurSaturation_ = qMax(0, saturation);
  }

  // Brightness (%, 100 = unchanged) applied to the top blur band's content
  // before blurring.
  int windowTopBlurBrightness() const {
    return windowTopBlurBrightness_;
  }
  void setWindowTopBlurBrightness(int brightness) {
    windowTopBlurBrightness_ = qMax(0, brightness);
  }

  // Chroma-key transparency for the top blur band: pixels close to this
  // color (and a soft range of similar shades around it, see
  // windowTopBlurKeyTolerance()) become transparent instead of blurred
  // opaquely, so a flat-colored background (e.g. the toolbar/window's own
  // fill) reveals what's really behind the band instead of reading as a
  // solid tinted patch. Default matches the app's own dark chrome color.
  QColor windowTopBlurKeyColor() const {
    return windowTopBlurKeyColor_;
  }
  void setWindowTopBlurKeyColor(const QColor& color) {
    windowTopBlurKeyColor_ = color;
  }

  // How far (0-255, per-channel-ish distance) a color can be from
  // windowTopBlurKeyColor() and still count as "close" -- 0 disables the
  // effect entirely; pixels within range fade toward transparent the
  // closer they are to the exact key color.
  int windowTopBlurKeyTolerance() const {
    return windowTopBlurKeyTolerance_;
  }
  void setWindowTopBlurKeyTolerance(int tolerance) {
    windowTopBlurKeyTolerance_ = qMax(0, tolerance);
  }

  // Window width (px) at/above which the toolbar search field stays
  // permanently expanded; below it, a collapsed search button takes its
  // place until clicked (see MainWindow::updateSearchBarMode()).
  int searchBarExpandWidth() const {
    return searchBarExpandWidth_;
  }
  void setSearchBarExpandWidth(int width) {
    searchBarExpandWidth_ = qMax(0, width);
  }

  // Window width (px) below which the 4-button view-mode segmented control
  // collapses into a single round icon+chevron button (a popup menu of the
  // same 4 modes) -- see MainWindow::updateViewModeCollapseMode().
  int viewModeCollapseWidth() const {
    return viewModeCollapseWidth_;
  }
  void setViewModeCollapseWidth(int width) {
    viewModeCollapseWidth_ = qMax(0, width);
  }

  // Extra top padding (px) above the folder view's items (icons/list rows),
  // added on top of the toolbar-height reservation in
  // MainWindow::updateToolbarOverlayGeometry() -- purely cosmetic breathing
  // room, unlike that reservation which exists to not hide content under
  // the toolbar/tab bar. Sidebar is unaffected.
  int contentTopPadding() const {
    return contentTopPadding_;
  }
  void setContentTopPadding(int padding) {
    contentTopPadding_ = qMax(0, padding);
  }

  // Padding (px) inside each toolbar button, between its border and its
  // icon -- the "QToolBar QToolButton { padding: Npx }" rule in the ctor's
  // stylesheet used to hardcode this at 6. Live-reloadable (see
  // MainWindow::updateToolbarStyleSheet(), called both from the ctor and
  // updateFromSettings()).
  int toolbarIconPadding() const {
    return toolbarIconPadding_;
  }
  void setToolbarIconPadding(int padding) {
    toolbarIconPadding_ = qMax(0, padding);
  }

  // Extra offset (px) added on top of the toolbar-height pass-through
  // inset the sidebar's item view already gets (see
  // MainWindow::applyScrollInsets()) -- row 0 sits (toolbar height +
  // this) px down at rest instead of just the toolbar height, same
  // scroll-through behavior otherwise (real rows scroll up through the
  // extra strip too, not just an empty margin).
  int sidePaneTopOffset() const {
    return sidePaneTopOffset_;
  }
  void setSidePaneTopOffset(int offset) {
    sidePaneTopOffset_ = offset;
  }

  // Icon size (px) shown before each breadcrumb segment's label (see
  // MainWindow::updateBreadcrumb()) -- the real per-item colored icon
  // (Fm::FileInfo's own, via IconTheme::icon(fm_file_info_get_icon())),
  // not a generic glyph.
  int breadcrumbIconSize() const {
    return breadcrumbIconSize_;
  }
  void setBreadcrumbIconSize(int size) {
    breadcrumbIconSize_ = qMax(0, size);
  }

  // Thickness (px) of every QScrollBar in the window (folder view, sidebar,
  // column view, ...) -- see MainWindow::updateScrollBarStyleSheet(). Keeps
  // the native/system scrollbar look (handle, groove, arrows all still
  // theme-drawn) and only overrides the width/height metric, unlike the
  // thin custom translucent scrollbar tried and reverted earlier (see
  // SidePane::styleSheet()'s own comment on that).
  int scrollBarWidth() const {
    return scrollBarWidth_;
  }
  void setScrollBarWidth(int width) {
    scrollBarWidth_ = qMax(1, width);
  }

  // Toolbar icon sizing, shared by every group below: innerSize is the
  // actual SVG/pixmap size drawn inside each button (feeds both
  // ui.toolBar's global iconSize -- used by the plain-action buttons in
  // the BackForward/ViewOptions groups -- and the manually-sized round
  // buttons/composites in IconGrouping/Search/ShareTagsMore). outerSize is
  // the diameter of the round "liquid glass" buttons (IconGrouping,
  // Search) before their own +10 wide-composite bump; segmented groups
  // (BackForward/ViewOptions/ShareTagsMore) aren't affected by it directly
  // -- their height/width come from their own Group*Height/Width keys or
  // natural sizing.
  int toolbarIconInnerSize() const {
    return toolbarIconInnerSize_;
  }
  void setToolbarIconInnerSize(int size) {
    toolbarIconInnerSize_ = qMax(1, size);
  }
  int toolbarIconOuterSize() const {
    return toolbarIconOuterSize_;
  }
  void setToolbarIconOuterSize(int size) {
    toolbarIconOuterSize_ = qMax(1, size);
  }

  // Per-group left/right margin and explicit height/width (0 = auto/
  // natural sizing) for each of the toolbar's five button groups: the
  // Back/Forward pill, the view-mode segmented control, the Group By
  // (icon grouping) round button, the Share/Tags/More pill, and the
  // collapsed round Search button. Every group gets the same four knobs,
  // generated here to keep the ten near-identical accessor pairs from
  // ballooning this header.
// Margins may be negative (to pull a group tighter than its default 4px
// breathing room, or overlap it slightly) -- only Height/Width are clamped
// to 0, since those mean "auto" and a negative fixed size is meaningless.
#define FILER_TOOLBAR_GROUP_SETTING(Pascal, camel)                            \
  int camel##MarginLeft() const { return camel##MarginLeft_; }               \
  void set##Pascal##MarginLeft(int v) { camel##MarginLeft_ = v; }             \
  int camel##MarginRight() const { return camel##MarginRight_; }             \
  void set##Pascal##MarginRight(int v) { camel##MarginRight_ = v; }           \
  int camel##Height() const { return camel##Height_; }                       \
  void set##Pascal##Height(int v) { camel##Height_ = qMax(0, v); }           \
  int camel##Width() const { return camel##Width_; }                         \
  void set##Pascal##Width(int v) { camel##Width_ = qMax(0, v); }
  // Gap (px) between the sidebar column and the Back/Forward group
  // specifically -- the only one of the five groups that sits flush
  // against another fixed toolbar element (sidebarSpacer_, which mirrors
  // the sidebar's own width) rather than another group, so it needs its
  // own dedicated spacer widget instead of just a neighboring group's own
  // MarginLeft/Right (see the ctor and applyToolbarGroupSettings()).
  // Floored at 0 -- unlike a group's own MarginLeft/Right, this is a plain
  // QWidget spacer, and a widget can't have negative width; to pull
  // backForwardGroup_ closer than 0 (even overlapping the sidebar), use its
  // own (unclamped) backForwardGroupMarginLeft() instead.
  int backForwardGroupLeftOffset() const {
    return backForwardGroupLeftOffset_;
  }
  void setBackForwardGroupLeftOffset(int offset) {
    backForwardGroupLeftOffset_ = qMax(0, offset);
  }

  FILER_TOOLBAR_GROUP_SETTING(BackForwardGroup, backForwardGroup)
  FILER_TOOLBAR_GROUP_SETTING(ViewOptionsGroup, viewOptionsGroup)
  FILER_TOOLBAR_GROUP_SETTING(IconGroupingGroup, iconGroupingGroup)
  FILER_TOOLBAR_GROUP_SETTING(ShareTagsMoreGroup, shareTagsMoreGroup)
  FILER_TOOLBAR_GROUP_SETTING(SearchButtonGroup, searchButtonGroup)
#undef FILER_TOOLBAR_GROUP_SETTING

  int mainWindowTint() const {
    return mainWindowTint_;
  }

  void setMainWindowTint(int mainWindowTint) {
    mainWindowTint_ = qBound(0, mainWindowTint, 100);
  }

  // Sidebar transparency/blur (see MainWindow::paintEvent() and
  // updateSidebarBlurRegion()) -- transparency() gates a real alpha channel
  // on the frosted sidebar fill (0 = fully opaque regardless of
  // transparencyPower()); transparencyPower() (0-100) controls how much of
  // KWin's blur-behind shows through, higher = more transparent/see-through.
  bool transparency() const {
    return transparency_;
  }

  void setTransparency(bool transparency) {
    transparency_ = transparency;
  }

  int transparencyPower() const {
    return transparencyPower_;
  }

  void setTransparencyPower(int transparencyPower) {
    transparencyPower_ = qBound(0, transparencyPower, 100);
  }

  // IDs (PlacesModelItem::id(), e.g. "recents", "trash") of sidebar items
  // the user removed via "Remove from Sidebar" -- persisted so they stay
  // hidden across restarts.
  const QStringList& hiddenSidebarItems() const {
    return hiddenSidebarItems_;
  }

  void setHiddenSidebarItems(const QStringList& hiddenSidebarItems) {
    hiddenSidebarItems_ = hiddenSidebarItems;
  }

  int splitterPos() const {
    return splitterPos_;
  }

  void setSplitterPos(int splitterPos) {
    splitterPos_ = splitterPos;
  }

  Fm::SidePane::Mode sidePaneMode() const {
    return sidePaneMode_;
  }

  void setSidePaneMode(Fm::SidePane::Mode sidePaneMode) {
    sidePaneMode_ = sidePaneMode;
  }

  Fm::FolderView::ViewMode viewMode() const {
    return viewMode_;
  }

  void setViewMode(Fm::FolderView::ViewMode viewMode) {
    viewMode_ = viewMode;
  }

  bool showHidden() const {
    return showHidden_;
  }

  void setShowHidden(bool showHidden) {
    showHidden_ = showHidden;
  }

  Qt::SortOrder sortOrder() const {
    return sortOrder_;
  }

  void setSortOrder(Qt::SortOrder sortOrder) {
    sortOrder_ = sortOrder;
  }

  Fm::FolderModel::ColumnId sortColumn() const {
    return sortColumn_;
  }

  void setSortColumn(Fm::FolderModel::ColumnId sortColumn) {
    sortColumn_ = sortColumn;
  }

  bool sortFolderFirst() const {
    return sortFolderFirst_;
  }

  void setSortFolderFirst(bool folderFirst) {
    sortFolderFirst_ = folderFirst;
  }

  bool showFilter() const {
    return showFilter_;
  }

  void setShowFilter(bool value) {
    showFilter_ = value;
  }

  bool spatialMode() const {
    return spatialMode_;
  }

  void setSpatialMode(bool value) {
    spatialMode_ = value;
  }

  bool dirInfoWrite() const {
    return dirInfoWrite_;
  }

  void setDirInfoWrite(bool value) {
    dirInfoWrite_ = value;
  }

  // settings for use with libfm
  bool singleClick() const {
    return singleClick_;
  }

  void setSingleClick(bool singleClick) {
    singleClick_ = singleClick;
  }

  int autoSelectionDelay() const {
    return autoSelectionDelay_;
  }

  void setAutoSelectionDelay(int value) {
    autoSelectionDelay_ = value;
  }

  bool confirmDelete() const {
    return confirmDelete_;
  }

  void setConfirmDelete(bool confirmDelete) {
    confirmDelete_ = confirmDelete;
  }

  bool noUsbTrash() const {
    return noUsbTrash_;
  }

  void setNoUsbTrash(bool noUsbTrash) {
    noUsbTrash_ = noUsbTrash;
    fm_config->no_usb_trash = noUsbTrash_; // also set this to libfm since FmFileOpsJob reads this config value before trashing files.
  }

  bool confirmTrash() const {
    return confirmTrash_;
  }

  void setConfirmTrash(bool value) {
    confirmTrash_ = value;
  }

  bool quickExec() const {
    return quickExec_;
  }

  void setQuickExec(bool value) {
    quickExec_ = value;
    fm_config->quick_exec = quickExec_;
  }

  // bool thumbnailLocal_;
  // bool thumbnailMax;

  int bigIconSize() const {
    return bigIconSize_;
  }

  void setBigIconSize(int bigIconSize) {
    bigIconSize_ = bigIconSize;
  }

  // Extra spacing (px) added to each side of a folder view grid cell, on
  // top of the cell's natural icon+label size -- independently
  // configurable per side, unlike the single Big Icon Size-derived cell
  // size FolderItemDelegate used before.
  int gridSpacingTop() const {
    return gridSpacingTop_;
  }
  void setGridSpacingTop(int spacing) {
    gridSpacingTop_ = spacing;
  }

  int gridSpacingBottom() const {
    return gridSpacingBottom_;
  }
  void setGridSpacingBottom(int spacing) {
    gridSpacingBottom_ = spacing;
  }

  int gridSpacingLeft() const {
    return gridSpacingLeft_;
  }
  void setGridSpacingLeft(int spacing) {
    gridSpacingLeft_ = spacing;
  }

  int gridSpacingRight() const {
    return gridSpacingRight_;
  }
  void setGridSpacingRight(int spacing) {
    gridSpacingRight_ = spacing;
  }

  // Folder view (icon/thumbnail mode) label styling -- see
  // FolderItemDelegate::drawText(). 0 for size/weight means "don't
  // override, use the item view's own default font". Live-reloadable the
  // same way GridSpacingTop/etc. above are (see
  // Settings::reloadSidePaneLayoutSettings() and FolderView::updateGridSize()).
  int iconViewTextSize() const {
    return iconViewTextSize_;
  }
  void setIconViewTextSize(int size) {
    iconViewTextSize_ = qMax(0, size);
  }

  // "left", "center", or "right" -- anything else falls back to "center".
  QString iconViewTextAnchor() const {
    return iconViewTextAnchor_;
  }
  void setIconViewTextAnchor(const QString& anchor) {
    iconViewTextAnchor_ = anchor;
  }

  int iconViewTextMarginTop() const {
    return iconViewTextMarginTop_;
  }
  void setIconViewTextMarginTop(int margin) {
    iconViewTextMarginTop_ = qMax(0, margin);
  }

  int iconViewTextMarginBottom() const {
    return iconViewTextMarginBottom_;
  }
  void setIconViewTextMarginBottom(int margin) {
    iconViewTextMarginBottom_ = qMax(0, margin);
  }

  // QFont::Weight value (Qt6 CSS-like scale: Normal=400, Medium=500,
  // DemiBold=600, Bold=700, ...) -- 0 means "don't override".
  int iconViewTextWeight() const {
    return iconViewTextWeight_;
  }
  void setIconViewTextWeight(int weight) {
    iconViewTextWeight_ = qMax(0, weight);
  }

  // Whether the "N items" subtext under each directory in the folder view
  // is shown -- 1 to show, 0 to hide. See FolderItemDelegate::itemCountText().
  int showFolderItemCount() const {
    return showFolderItemCount_;
  }
  void setShowFolderItemCount(int show) {
    showFolderItemCount_ = show;
  }

  bool useStacks() const {
    return useStacks_;
  }

  void setUseStacks(bool useStacks) {
    useStacks_ = useStacks;
  }

  int smallIconSize() const {
    return smallIconSize_;
  }

  void setSmallIconSize(int smallIconSize) {
    smallIconSize_ = smallIconSize;
  }

  int sidePaneIconSize() const {
    return sidePaneIconSize_;
  }

  void setSidePaneIconSize(int sidePaneIconSize) {
    sidePaneIconSize_ = sidePaneIconSize;
  }

  // Vertical gap (px) between sidebar item rows -- the sidebar item pill's
  // top/bottom margin in sidepane.cpp's stylesheet.
  int sidePaneItemSpacing() const {
    return sidePaneItemSpacing_;
  }

  void setSidePaneItemSpacing(int spacing) {
    sidePaneItemSpacing_ = spacing;
  }

  // Left/right padding (px) inside each sidebar item pill, between the
  // pill's edge and its icon/text content.
  int sidePaneItemHorizontalPadding() const {
    return sidePaneItemHorizontalPadding_;
  }

  void setSidePaneItemHorizontalPadding(int padding) {
    sidePaneItemHorizontalPadding_ = padding;
  }

  // Gap (px) between a sidebar item's icon and its text label.
  int sidePaneIconTextSpacing() const {
    return sidePaneIconTextSpacing_;
  }

  void setSidePaneIconTextSpacing(int spacing) {
    sidePaneIconTextSpacing_ = spacing;
  }

  // Gap (px) above each category header's label (Favorites/Locations/Tags),
  // i.e. the space between the previous row and the category name.
  int sidePaneCategorySpacing() const {
    return sidePaneCategorySpacing_;
  }

  void setSidePaneCategorySpacing(int spacing) {
    sidePaneCategorySpacing_ = spacing;
  }

  // Gap (px) below each category header's label, independent of the gap
  // above it (see sidePaneCategorySpacing()).
  int sidePaneCategorySpacingBottom() const {
    return sidePaneCategorySpacingBottom_;
  }

  void setSidePaneCategorySpacingBottom(int spacing) {
    sidePaneCategorySpacingBottom_ = spacing;
  }

  // Font size (px) of regular sidebar item labels (Recents, Applications,
  // Desktop, ...) -- independent of sidePaneCategoryFontSize().
  int sidePaneItemFontSize() const {
    return sidePaneItemFontSize_;
  }

  void setSidePaneItemFontSize(int size) {
    sidePaneItemFontSize_ = size;
  }

  // Font size (px) of category header labels (Favorites, Locations, Tags).
  int sidePaneCategoryFontSize() const {
    return sidePaneCategoryFontSize_;
  }

  void setSidePaneCategoryFontSize(int size) {
    sidePaneCategoryFontSize_ = size;
  }

  // Left padding (px) of category header labels (Favorites, Locations,
  // Tags), independent of sidePaneItemHorizontalPadding() which only
  // applies to regular item rows.
  int sidePaneCategoryLeftPadding() const {
    return sidePaneCategoryLeftPadding_;
  }

  void setSidePaneCategoryLeftPadding(int padding) {
    sidePaneCategoryLeftPadding_ = padding;
  }

  // Distance (px) from the selection pill's right edge to the row's right
  // edge -- smaller values let the highlight reach further right (closer
  // to the scrollbar), larger values make it stop further short.
  int sidePaneSelectorRightInset() const {
    return sidePaneSelectorRightInset_;
  }

  void setSidePaneSelectorRightInset(int inset) {
    sidePaneSelectorRightInset_ = inset;
  }

  // Selection pill fill color (hex, e.g. "#424242") and opacity (0-255).
  QColor sidePaneSelectorColor() const {
    return sidePaneSelectorColor_;
  }

  void setSidePaneSelectorColor(const QColor& color) {
    sidePaneSelectorColor_ = color;
  }

  int sidePaneSelectorOpacity() const {
    return sidePaneSelectorOpacity_;
  }

  void setSidePaneSelectorOpacity(int opacity) {
    sidePaneSelectorOpacity_ = opacity;
  }

  // See SidePaneScrollBarMode.
  int sidePaneScrollBarMode() const {
    return sidePaneScrollBarMode_;
  }

  void setSidePaneScrollBarMode(int mode) {
    sidePaneScrollBarMode_ = mode;
  }

  // CSS-style numeric font weight (100-900) for sidebar item labels.
  int sidePaneFontWeight() const {
    return sidePaneFontWeight_;
  }

  void setSidePaneFontWeight(int weight) {
    sidePaneFontWeight_ = weight;
  }

  int thumbnailIconSize() const {
    return thumbnailIconSize_;
  }

  bool showThumbnails() {
    return showThumbnails_;
  }

  void setShowThumbnails(bool show) {
    showThumbnails_ = show;
  }

  void setThumbnailLocalFilesOnly(bool value) {
    Fm::ThumbnailLoader::setLocalFilesOnly(value);
  }

  bool thumbnailLocalFilesOnly() {
    return Fm::ThumbnailLoader::localFilesOnly();
  }

  int maxThumbnailFileSize() {
    return Fm::ThumbnailLoader::maxThumbnailFileSize();
  }

  void setMaxThumbnailFileSize(int size) {
    Fm::ThumbnailLoader::setMaxThumbnailFileSize(size);
  }

  void setThumbnailIconSize(int thumbnailIconSize) {
    thumbnailIconSize_ = thumbnailIconSize;
  }

  bool siUnit() {
    return siUnit_;
  }

  void setSiUnit(bool siUnit) {
    siUnit_ = siUnit;
    // override libfm FmConfig settings. FIXME: should we do this?
    fm_config->si_unit = (gboolean)siUnit_;
  }

  bool backupAsHidden() const {
    return backupAsHidden_;
  }

  void setBackupAsHidden(bool value) {
    backupAsHidden_ = value;
    fm_config->backup_as_hidden = backupAsHidden_; // also set this to libfm since fm_file_info_is_hidden() reads this value internally.
  }

  bool showFullNames() const {
    return showFullNames_;
  }

  void setShowFullNames(bool value) {
    showFullNames_ = value;
  }

  bool shadowHidden() const {
    return shadowHidden_;
  }

  void setShadowHidden(bool value) {
    shadowHidden_ = value;
  }

  bool onlyUserTemplates() const {
    return onlyUserTemplates_;
  }

  void setOnlyUserTemplates(bool value) {
    onlyUserTemplates_ = value;
    fm_config->only_user_templates = onlyUserTemplates_;
  }

  bool templateTypeOnce() const {
    return templateTypeOnce_;
  }

  void setTemplateTypeOnce(bool value) {
    templateTypeOnce_ = value;
    fm_config->template_type_once = templateTypeOnce_;
  }

  bool templateRunApp() const {
    return templateRunApp_;
  }

  void setTemplateRunApp(bool value) {
    templateRunApp_ = value;
    fm_config->template_run_app = templateRunApp_;
  }

private:
  QString profileName_;
  bool supportTrash_;

  // Filer specific
  QString fallbackIconThemeName_;
  bool useFallbackIconTheme_;

  OpenDirTargetType bookmarkOpenMethod_;
  QString suCommand_;
  QString terminal_;
  bool mountOnStartup_;
  bool mountRemovable_;
  bool autoRun_;
  bool closeOnUnmount_;

  int wallpaperMode_;
  QString wallpaper_;
  QColor desktopBgColor_;
  QColor desktopFgColor_;
  QColor desktopShadowColor_;
  QFont desktopFont_;

  bool desktopShowHidden_;
  Qt::SortOrder desktopSortOrder_;
  Fm::FolderModel::ColumnId desktopSortColumn_;

  bool alwaysShowTabs_;
  bool showTabClose_;
  bool rememberWindowSize_;
  int fixedWindowWidth_;
  int fixedWindowHeight_;
  int lastWindowWidth_;
  int lastWindowHeight_;
  bool lastWindowMaximized_;
  bool tintWindow_;
  int sidebarTint_;
  int windowTopBlurHeight_;
  int windowTopBlurPower_;
  int windowTopBlurOpacity_;
  int windowTopBlurSaturation_;
  int windowTopBlurBrightness_;
  QColor windowTopBlurKeyColor_;
  int windowTopBlurKeyTolerance_;
  int searchBarExpandWidth_;
  int viewModeCollapseWidth_;
  int contentTopPadding_;
  int toolbarIconPadding_;
  int sidePaneTopOffset_;
  int breadcrumbIconSize_;
  int scrollBarWidth_;
  int toolbarIconInnerSize_;
  int toolbarIconOuterSize_;
  int backForwardGroupLeftOffset_;
  int backForwardGroupMarginLeft_;
  int backForwardGroupMarginRight_;
  int backForwardGroupHeight_;
  int backForwardGroupWidth_;
  int viewOptionsGroupMarginLeft_;
  int viewOptionsGroupMarginRight_;
  int viewOptionsGroupHeight_;
  int viewOptionsGroupWidth_;
  int iconGroupingGroupMarginLeft_;
  int iconGroupingGroupMarginRight_;
  int iconGroupingGroupHeight_;
  int iconGroupingGroupWidth_;
  int shareTagsMoreGroupMarginLeft_;
  int shareTagsMoreGroupMarginRight_;
  int shareTagsMoreGroupHeight_;
  int shareTagsMoreGroupWidth_;
  int searchButtonGroupMarginLeft_;
  int searchButtonGroupMarginRight_;
  int searchButtonGroupHeight_;
  int searchButtonGroupWidth_;
  int mainWindowTint_;
  bool transparency_;
  int transparencyPower_;
  QStringList hiddenSidebarItems_;
  int splitterPos_;
  Fm::SidePane::Mode sidePaneMode_;

  Fm::FolderView::ViewMode viewMode_;
  bool showHidden_;
  Qt::SortOrder sortOrder_;
  Fm::FolderModel::ColumnId sortColumn_;
  bool sortFolderFirst_;
  bool showFilter_;
  bool spatialMode_;
  bool dirInfoWrite_;

  // settings for use with libfm
  bool singleClick_;
  int autoSelectionDelay_;
  bool confirmDelete_;
  bool noUsbTrash_; // do not trash files on usb removable devices
  bool confirmTrash_; // Confirm before moving files into "trash can"
  bool quickExec_; // Don't ask options on launch executable file

  bool showThumbnails_;

  QString archiver_;
  bool siUnit_;
  bool backupAsHidden_;
  bool showFullNames_;
  bool shadowHidden_;

  int bigIconSize_;
  int smallIconSize_;
  int sidePaneIconSize_;
  int sidePaneItemSpacing_;
  int sidePaneItemHorizontalPadding_;
  int sidePaneIconTextSpacing_;
  int sidePaneCategorySpacing_;
  int sidePaneCategorySpacingBottom_;
  int sidePaneItemFontSize_;
  int sidePaneCategoryFontSize_;
  int sidePaneCategoryLeftPadding_;
  int sidePaneSelectorRightInset_;
  QColor sidePaneSelectorColor_;
  int sidePaneSelectorOpacity_;
  int sidePaneScrollBarMode_;
  int sidePaneFontWeight_;
  int gridSpacingTop_;
  int gridSpacingBottom_;
  int gridSpacingLeft_;
  int gridSpacingRight_;
  int iconViewTextSize_;
  QString iconViewTextAnchor_;
  int iconViewTextMarginTop_;
  int iconViewTextMarginBottom_;
  int iconViewTextWeight_;
  int showFolderItemCount_;
  int thumbnailIconSize_;
  bool useStacks_;

  bool onlyUserTemplates_;
  bool templateTypeOnce_;
  bool templateRunApp_;
};

}

#endif // PCMANFM_SETTINGS_H
