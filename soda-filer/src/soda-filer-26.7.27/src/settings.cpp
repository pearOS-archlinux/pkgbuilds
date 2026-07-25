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


#include "settings.h"
#include <QDir>
#include <QFile>
#include <QStringBuilder>
#include <QSettings>
#include <QApplication>
#include "desktopwindow.h"
#include "utilities.h"
// #include <QDesktopServices>

using namespace Filer;

inline static const char* bookmarkOpenMethodToString(OpenDirTargetType value);
inline static OpenDirTargetType bookmarkOpenMethodFromString(const QString str);

inline static const char* wallpaperModeToString(int value);
inline static int wallpaperModeFromString(const QString str);

inline static const char* viewModeToString(Fm::FolderView::ViewMode value);
inline static Fm::FolderView::ViewMode viewModeFromString(const QString str);

inline static const char* sidePaneModeToString(Fm::SidePane::Mode value);
inline static Fm::SidePane::Mode sidePaneModeFromString(const QString& str);

inline static const char* sortOrderToString(Qt::SortOrder order);
inline static Qt::SortOrder sortOrderFromString(const QString str);

inline static const char* sortColumnToString(Fm::FolderModel::ColumnId value);
inline static Fm::FolderModel::ColumnId sortColumnFromString(const QString str);

Settings::Settings():
  QObject(),
  supportTrash_(Fm::uriExists("trash:///")), // check if trash:/// is supported
  fallbackIconThemeName_(),
  useFallbackIconTheme_(QIcon::themeName().isEmpty() || QIcon::themeName() == "hicolor"),
  bookmarkOpenMethod_(OpenInCurrentTab),
  suCommand_(),
  terminal_(),
  mountOnStartup_(true),
  mountRemovable_(true),
  autoRun_(true),
  closeOnUnmount_(true), // probono: Closes the window when the device/network share is unmounted. This is awesome!
  wallpaperMode_(0),
  wallpaper_(),
  desktopBgColor_(),
  desktopFgColor_(),
  desktopShadowColor_(),
  desktopShowHidden_(false),
  desktopSortOrder_(Qt::AscendingOrder),
  desktopSortColumn_(Fm::FolderModel::ColumnFileName),
  alwaysShowTabs_(false), // probono: Do not show tabs when there is only one tab
  showTabClose_(true),
  rememberWindowSize_(true),
  fixedWindowWidth_(640),
  fixedWindowHeight_(480),
  lastWindowWidth_(640),
  lastWindowHeight_(480),
  lastWindowMaximized_(false),
  tintWindow_(true),
  sidebarTint_(16),
  windowTopBlurHeight_(40),
  windowTopBlurPower_(10),
  windowTopBlurOpacity_(255),
  windowTopBlurSaturation_(100),
  windowTopBlurBrightness_(100),
  windowTopBlurKeyColor_(0x28, 0x27, 0x29),
  windowTopBlurKeyTolerance_(40),
  searchBarExpandWidth_(1110),
  viewModeCollapseWidth_(700),
  contentTopPadding_(0),
  toolbarIconPadding_(6),
  sidePaneTopOffset_(0),
  breadcrumbIconSize_(16),
  scrollBarWidth_(16),
  toolbarIconInnerSize_(20),
  toolbarIconOuterSize_(32),
  backForwardGroupLeftOffset_(0),
  backForwardGroupMarginLeft_(4),
  backForwardGroupMarginRight_(4),
  backForwardGroupHeight_(0),
  backForwardGroupWidth_(0),
  viewOptionsGroupMarginLeft_(4),
  viewOptionsGroupMarginRight_(4),
  viewOptionsGroupHeight_(0),
  viewOptionsGroupWidth_(0),
  iconGroupingGroupMarginLeft_(0),
  iconGroupingGroupMarginRight_(0),
  iconGroupingGroupHeight_(0),
  iconGroupingGroupWidth_(0),
  shareTagsMoreGroupMarginLeft_(4),
  shareTagsMoreGroupMarginRight_(4),
  shareTagsMoreGroupHeight_(0),
  shareTagsMoreGroupWidth_(0),
  searchButtonGroupMarginLeft_(0),
  searchButtonGroupMarginRight_(0),
  searchButtonGroupHeight_(0),
  searchButtonGroupWidth_(0),
  mainWindowTint_(4),
  transparency_(true),
  transparencyPower_(25),
  splitterPos_(260),
  sidePaneMode_(Fm::SidePane::ModePlaces),
  viewMode_(Fm::FolderView::IconMode),
  showHidden_(false),
  sortOrder_(Qt::AscendingOrder),
  sortColumn_(Fm::FolderModel::ColumnFileName),
  sortFolderFirst_(false),
  showFilter_(false),
  spatialMode_(true),
  dirInfoWrite_(false),
  // settings for use with libfm
  singleClick_(false),
  autoSelectionDelay_(600),
  confirmDelete_(true),
  noUsbTrash_(false),
  confirmTrash_(false),
  quickExec_(true), // probono: By default execute executable files without asking the user questions
  showThumbnails_(true),
  archiver_(),
  siUnit_(false),
  bigIconSize_(48),
  smallIconSize_(24),
  sidePaneIconSize_(20),
  sidePaneItemSpacing_(0),
  sidePaneItemHorizontalPadding_(6),
  sidePaneIconTextSpacing_(6),
  sidePaneCategorySpacing_(17),
  sidePaneCategorySpacingBottom_(8),
  sidePaneItemFontSize_(11),
  sidePaneCategoryFontSize_(11),
  sidePaneCategoryLeftPadding_(4),
  sidePaneSelectorRightInset_(8),
  sidePaneSelectorColor_(66, 66, 66),
  sidePaneSelectorOpacity_(255),
  sidePaneScrollBarMode_(ScrollBarAuto),
  sidePaneFontWeight_(600),
  gridSpacingTop_(5),
  gridSpacingBottom_(5),
  gridSpacingLeft_(60),
  gridSpacingRight_(60),
  iconViewTextSize_(0),
  iconViewTextAnchor_(QStringLiteral("center")),
  iconViewTextMarginTop_(0),
  iconViewTextMarginBottom_(0),
  iconViewTextWeight_(0),
  showFolderItemCount_(1),
  thumbnailIconSize_(128),
  useStacks_(false) {
}

Settings::~Settings() {

}

QString Settings::profileDir(QString profile, bool useFallback) {
  // NOTE: it's a shame that QDesktopServices does not handle XDG_CONFIG_HOME
  // try user-specific config file first
  QString dirName = QLatin1String(qgetenv("XDG_CONFIG_HOME"));
  if (dirName.isEmpty())
    dirName = QDir::homePath() % QLatin1String("/.config");
  dirName = dirName % "/filer/" % profile;
  QDir dir(dirName);

  // if user config dir does not exist, try system-wide config dirs instead
  if(!dir.exists() && useFallback) {
    QString fallbackDir;
    for(const char* const* configDir = g_get_system_config_dirs(); *configDir; ++configDir) {
      fallbackDir = QString(*configDir) % "/filer/" % profile;
      dir.setPath(fallbackDir);
      if(dir.exists()) {
	dirName = fallbackDir;
	break;
      }
    }
  }
  return dirName;
}

bool Settings::load(QString profile) {
  profileName_ = profile;
  QString fileName = profileDir(profile, true) % "/settings.conf";
  return loadFile(fileName);
}

void Settings::reloadTintSettings() {
  QString fileName = profileDir(profileName_, true) % "/settings.conf";
  QSettings settings(fileName, QSettings::IniFormat);
  // QSettings caches a file's parsed contents per-path across instances and
  // only re-reads it lazily (on its own timer, or when sync() is called) --
  // without this, a fresh QSettings pointed at the same path right after an
  // external edit could still hand back the stale in-memory copy instead of
  // what was just written to disk.
  settings.sync();
  settings.beginGroup("Window");
  tintWindow_ = settings.value("TintWindow", tintWindow_ ? 1 : 0).toInt() != 0;
  sidebarTint_ = qBound(0, settings.value("SidebarTint", sidebarTint_).toInt(), 100);
  windowTopBlurHeight_ = qMax(0, settings.value("WindowTopBlurHeight", windowTopBlurHeight_).toInt());
  windowTopBlurPower_ = qMax(0, settings.value("WindowTopBlurPower", windowTopBlurPower_).toInt());
  windowTopBlurOpacity_ = qBound(0, settings.value("WindowTopBlurOpacity", windowTopBlurOpacity_).toInt(), 255);
  windowTopBlurSaturation_ = qMax(0, settings.value("WindowTopBlurSaturation", windowTopBlurSaturation_).toInt());
  windowTopBlurBrightness_ = qMax(0, settings.value("WindowTopBlurBrightness", windowTopBlurBrightness_).toInt());
  windowTopBlurKeyColor_ = QColor(settings.value("WindowTopBlurKeyColor", windowTopBlurKeyColor_.name()).toString());
  windowTopBlurKeyTolerance_ = qMax(0, settings.value("WindowTopBlurKeyTolerance", windowTopBlurKeyTolerance_).toInt());
  searchBarExpandWidth_ = qMax(0, settings.value("SearchBarExpandWidth", searchBarExpandWidth_).toInt());
  viewModeCollapseWidth_ = qMax(0, settings.value("ViewModeCollapseWidth", viewModeCollapseWidth_).toInt());
  splitterPos_ = qMax(0, settings.value("SplitterPos", splitterPos_).toInt());
  contentTopPadding_ = qMax(0, settings.value("ContentTopPadding", contentTopPadding_).toInt());
  toolbarIconPadding_ = qMax(0, settings.value("ToolbarIconPadding", toolbarIconPadding_).toInt());
  sidePaneTopOffset_ = settings.value("SidePaneTopOffset", sidePaneTopOffset_).toInt();
  breadcrumbIconSize_ = qMax(0, settings.value("BreadcrumbIconSize", breadcrumbIconSize_).toInt());
  scrollBarWidth_ = qMax(1, settings.value("ScrollBarWidth", scrollBarWidth_).toInt());
  toolbarIconInnerSize_ = qMax(1, settings.value("ToolbarIconInnerSize", toolbarIconInnerSize_).toInt());
  toolbarIconOuterSize_ = qMax(1, settings.value("ToolbarIconOuterSize", toolbarIconOuterSize_).toInt());
  backForwardGroupLeftOffset_ = settings.value("BackForwardGroupLeftOffset", backForwardGroupLeftOffset_).toInt();
  backForwardGroupMarginLeft_ = settings.value("BackForwardGroupMarginLeft", backForwardGroupMarginLeft_).toInt();
  backForwardGroupMarginRight_ = settings.value("BackForwardGroupMarginRight", backForwardGroupMarginRight_).toInt();
  backForwardGroupHeight_ = qMax(0, settings.value("BackForwardGroupHeight", backForwardGroupHeight_).toInt());
  backForwardGroupWidth_ = qMax(0, settings.value("BackForwardGroupWidth", backForwardGroupWidth_).toInt());
  viewOptionsGroupMarginLeft_ = settings.value("ViewOptionsGroupMarginLeft", viewOptionsGroupMarginLeft_).toInt();
  viewOptionsGroupMarginRight_ = settings.value("ViewOptionsGroupMarginRight", viewOptionsGroupMarginRight_).toInt();
  viewOptionsGroupHeight_ = qMax(0, settings.value("ViewOptionsGroupHeight", viewOptionsGroupHeight_).toInt());
  viewOptionsGroupWidth_ = qMax(0, settings.value("ViewOptionsGroupWidth", viewOptionsGroupWidth_).toInt());
  iconGroupingGroupMarginLeft_ = settings.value("IconGroupingGroupMarginLeft", iconGroupingGroupMarginLeft_).toInt();
  iconGroupingGroupMarginRight_ = settings.value("IconGroupingGroupMarginRight", iconGroupingGroupMarginRight_).toInt();
  iconGroupingGroupHeight_ = qMax(0, settings.value("IconGroupingGroupHeight", iconGroupingGroupHeight_).toInt());
  iconGroupingGroupWidth_ = qMax(0, settings.value("IconGroupingGroupWidth", iconGroupingGroupWidth_).toInt());
  shareTagsMoreGroupMarginLeft_ = settings.value("ShareTagsMoreGroupMarginLeft", shareTagsMoreGroupMarginLeft_).toInt();
  shareTagsMoreGroupMarginRight_ = settings.value("ShareTagsMoreGroupMarginRight", shareTagsMoreGroupMarginRight_).toInt();
  shareTagsMoreGroupHeight_ = qMax(0, settings.value("ShareTagsMoreGroupHeight", shareTagsMoreGroupHeight_).toInt());
  shareTagsMoreGroupWidth_ = qMax(0, settings.value("ShareTagsMoreGroupWidth", shareTagsMoreGroupWidth_).toInt());
  searchButtonGroupMarginLeft_ = settings.value("SearchButtonGroupMarginLeft", searchButtonGroupMarginLeft_).toInt();
  searchButtonGroupMarginRight_ = settings.value("SearchButtonGroupMarginRight", searchButtonGroupMarginRight_).toInt();
  searchButtonGroupHeight_ = qMax(0, settings.value("SearchButtonGroupHeight", searchButtonGroupHeight_).toInt());
  searchButtonGroupWidth_ = qMax(0, settings.value("SearchButtonGroupWidth", searchButtonGroupWidth_).toInt());
  mainWindowTint_ = qBound(0, settings.value("MainWindowTint", mainWindowTint_).toInt(), 100);
  transparency_ = settings.value("Transparency", transparency_ ? 1 : 0).toInt() != 0;
  transparencyPower_ = qBound(0, settings.value("TransparencyPower", transparencyPower_).toInt(), 100);
  hiddenSidebarItems_ = settings.value("HiddenSidebarItems", hiddenSidebarItems_).toStringList();
  settings.endGroup();
}

void Settings::reloadSidePaneLayoutSettings() {
  QString fileName = profileDir(profileName_, true) % "/settings.conf";
  QSettings settings(fileName, QSettings::IniFormat);
  settings.sync(); // see reloadTintSettings() for why this is needed
  settings.beginGroup("FolderView");
  sidePaneIconSize_ = settings.value("SidePaneIconSize", sidePaneIconSize_).toInt();
  sidePaneItemSpacing_ = settings.value("SidePaneItemSpacing", sidePaneItemSpacing_).toInt();
  sidePaneItemHorizontalPadding_ = settings.value("SidePaneItemHorizontalPadding", sidePaneItemHorizontalPadding_).toInt();
  sidePaneIconTextSpacing_ = settings.value("SidePaneIconTextSpacing", sidePaneIconTextSpacing_).toInt();
  sidePaneCategorySpacing_ = settings.value("SidePaneCategorySpacing", sidePaneCategorySpacing_).toInt();
  sidePaneCategorySpacingBottom_ = settings.value("SidePaneCategorySpacingBottom", sidePaneCategorySpacingBottom_).toInt();
  sidePaneItemFontSize_ = settings.value("SidePaneItemFontSize", sidePaneItemFontSize_).toInt();
  sidePaneCategoryFontSize_ = settings.value("SidePaneCategoryFontSize", sidePaneCategoryFontSize_).toInt();
  sidePaneCategoryLeftPadding_ = settings.value("SidePaneCategoryLeftPadding", sidePaneCategoryLeftPadding_).toInt();
  sidePaneSelectorRightInset_ = settings.value("SidePaneSelectorRightInset", sidePaneSelectorRightInset_).toInt();
  sidePaneSelectorColor_ = QColor(settings.value("SidePaneSelectorColor", sidePaneSelectorColor_.name()).toString());
  sidePaneSelectorOpacity_ = settings.value("SidePaneSelectorOpacity", sidePaneSelectorOpacity_).toInt();
  sidePaneScrollBarMode_ = settings.value("SidePaneScrollBarMode", sidePaneScrollBarMode_).toInt();
  sidePaneFontWeight_ = settings.value("SidePaneFontWeight", sidePaneFontWeight_).toInt();
  gridSpacingTop_ = settings.value("GridSpacingTop", gridSpacingTop_).toInt();
  gridSpacingBottom_ = settings.value("GridSpacingBottom", gridSpacingBottom_).toInt();
  gridSpacingLeft_ = settings.value("GridSpacingLeft", gridSpacingLeft_).toInt();
  gridSpacingRight_ = settings.value("GridSpacingRight", gridSpacingRight_).toInt();
  iconViewTextSize_ = qMax(0, settings.value("IconViewTextSize", iconViewTextSize_).toInt());
  iconViewTextAnchor_ = settings.value("IconViewTextAnchor", iconViewTextAnchor_).toString();
  iconViewTextMarginTop_ = qMax(0, settings.value("IconViewTextMarginTop", iconViewTextMarginTop_).toInt());
  iconViewTextMarginBottom_ = qMax(0, settings.value("IconViewTextMarginBottom", iconViewTextMarginBottom_).toInt());
  iconViewTextWeight_ = qMax(0, settings.value("IconViewTextWeight", iconViewTextWeight_).toInt());
  showFolderItemCount_ = settings.value("ShowItemCount", showFolderItemCount_).toInt();
  settings.endGroup();
}

bool Settings::save(QString profile) {
  QString fileName = profileDir(profile.isEmpty() ? profileName_ : profile) % "/settings.conf";
  return saveFile(fileName);
}

bool Settings::loadFile(QString filePath) {
  QSettings settings(filePath, QSettings::IniFormat);
  settings.beginGroup("System");
  fallbackIconThemeName_ = settings.value("FallbackIconThemeName").toString();
  if(fallbackIconThemeName_.isEmpty()) {
    // FIXME: we should choose one from installed icon themes or get
    // the value from XSETTINGS instead of hard code a fallback value.
    fallbackIconThemeName_ = "elementary"; // fallback icon theme name
  }
  suCommand_ = settings.value("SuCommand", "gksu %s").toString();
  setTerminal(settings.value("Terminal", "xterm").toString()); // probono: TODO: Check if qterminal is on the $PATH and if it is, prefer it over xterm
  setArchiver(settings.value("Archiver", "file-roller").toString());
  setSiUnit(settings.value("SIUnit", false).toBool());

  setOnlyUserTemplates(settings.value("OnlyUserTemplates", false).toBool());
  setTemplateTypeOnce(settings.value("OemplateTypeOnce", false).toBool());
  setTemplateRunApp(settings.value("TemplateRunApp", false).toBool());

  settings.endGroup();

  settings.beginGroup("Behavior");
  bookmarkOpenMethod_ = bookmarkOpenMethodFromString(settings.value("BookmarkOpenMethod").toString());
  spatialMode_ = settings.value("SpatialMode", false).toBool();
  dirInfoWrite_ = settings.value("DirInfoWrite", true).toBool();
  // settings for use with libfm
  singleClick_ = settings.value("SingleClick", false).toBool();
  autoSelectionDelay_ = settings.value("AutoSelectionDelay", 600).toInt();
  confirmDelete_ = settings.value("ConfirmDelete", true).toBool();
  setNoUsbTrash(settings.value("NoUsbTrash", false).toBool());
  confirmTrash_ = settings.value("ConfirmTrash", false).toBool();
  setQuickExec(settings.value("QuickExec", true).toBool()); // probono: Do not ask what to do with executable files when they are double-clicked and have the executable bit set
  // bool thumbnailLocal_;
  // bool thumbnailMax;
  settings.endGroup();

  settings.beginGroup("Desktop");
  wallpaperMode_ = wallpaperModeFromString(settings.value("WallpaperMode").toString());
  wallpaper_ = settings.value("Wallpaper").toString();
  desktopBgColor_.setNamedColor(settings.value("BgColor", "#4e7fb4").toString());
  desktopFgColor_.setNamedColor(settings.value("FgColor", "#ffffff").toString());
  desktopShadowColor_.setNamedColor(settings.value("ShadowColor", "#000000").toString());
  desktopShowHidden_ = settings.value("ShowHidden", false).toBool();

  desktopSortOrder_ = sortOrderFromString(settings.value("SortOrder").toString());
  desktopSortColumn_ = sortColumnFromString(settings.value("SortColumn").toString());
  settings.endGroup();

  settings.beginGroup("Volume");
  mountOnStartup_ = settings.value("MountOnStartup", true).toBool();
  mountRemovable_ = settings.value("MountRemovable", true).toBool();
  autoRun_ = settings.value("AutoRun", true).toBool();
  closeOnUnmount_ = settings.value("CloseOnUnmount", true).toBool();
  settings.endGroup();

  settings.beginGroup("Thumbnail");
  showThumbnails_ = settings.value("ShowThumbnails", true).toBool();
  setMaxThumbnailFileSize(settings.value("MaxThumbnailFileSize", 4096).toInt());
  setThumbnailLocalFilesOnly(settings.value("ThumbnailLocalFilesOnly", true).toBool());
  settings.endGroup();

  settings.beginGroup("FolderView");
  viewMode_ = viewModeFromString(settings.value("Mode", Fm::FolderView::IconMode).toString());
  showHidden_ = settings.value("ShowHidden", false).toBool();
  sortOrder_ = sortOrderFromString(settings.value("SortOrder").toString());
  sortColumn_ = sortColumnFromString(settings.value("SortColumn").toString());
  sortFolderFirst_ = settings.value("SortFolderFirst", true).toBool();
  showFilter_ = settings.value("ShowFilter", false).toBool();

  setBackupAsHidden(settings.value("BackupAsHidden", false).toBool());
  showFullNames_ = settings.value("ShowFullNames", false).toBool();
  shadowHidden_ = settings.value("ShadowHidden", false).toBool();

  // override config in libfm's FmConfig
  bigIconSize_ = settings.value("BigIconSize", 64).toInt();
  smallIconSize_ = settings.value("SmallIconSize", 16).toInt(); // probono: 16 instead of 24
  sidePaneIconSize_ = settings.value("SidePaneIconSize", 20).toInt();
  sidePaneItemSpacing_ = settings.value("SidePaneItemSpacing", 0).toInt();
  sidePaneItemHorizontalPadding_ = settings.value("SidePaneItemHorizontalPadding", 6).toInt();
  sidePaneIconTextSpacing_ = settings.value("SidePaneIconTextSpacing", 6).toInt();
  sidePaneCategorySpacing_ = settings.value("SidePaneCategorySpacing", 17).toInt();
  sidePaneCategorySpacingBottom_ = settings.value("SidePaneCategorySpacingBottom", 8).toInt();
  sidePaneItemFontSize_ = settings.value("SidePaneItemFontSize", 11).toInt();
  sidePaneCategoryFontSize_ = settings.value("SidePaneCategoryFontSize", 11).toInt();
  sidePaneCategoryLeftPadding_ = settings.value("SidePaneCategoryLeftPadding", 4).toInt();
  sidePaneSelectorRightInset_ = settings.value("SidePaneSelectorRightInset", 8).toInt();
  sidePaneSelectorColor_ = QColor(settings.value("SidePaneSelectorColor", "#424242").toString());
  sidePaneSelectorOpacity_ = settings.value("SidePaneSelectorOpacity", 255).toInt();
  sidePaneScrollBarMode_ = settings.value("SidePaneScrollBarMode", (int)ScrollBarAuto).toInt();
  sidePaneFontWeight_ = settings.value("SidePaneFontWeight", 600).toInt();
  gridSpacingTop_ = settings.value("GridSpacingTop", 5).toInt();
  gridSpacingBottom_ = settings.value("GridSpacingBottom", 5).toInt();
  gridSpacingLeft_ = settings.value("GridSpacingLeft", 60).toInt();
  gridSpacingRight_ = settings.value("GridSpacingRight", 60).toInt();
  iconViewTextSize_ = qMax(0, settings.value("IconViewTextSize", 0).toInt());
  iconViewTextAnchor_ = settings.value("IconViewTextAnchor", QStringLiteral("center")).toString();
  iconViewTextMarginTop_ = qMax(0, settings.value("IconViewTextMarginTop", 0).toInt());
  iconViewTextMarginBottom_ = qMax(0, settings.value("IconViewTextMarginBottom", 0).toInt());
  iconViewTextWeight_ = qMax(0, settings.value("IconViewTextWeight", 0).toInt());
  showFolderItemCount_ = settings.value("ShowItemCount", 1).toInt();
  thumbnailIconSize_ = settings.value("ThumbnailIconSize", 128).toInt();
  useStacks_ = settings.value("UseStacks", false).toBool();
  settings.endGroup();

  settings.beginGroup("Window");
  fixedWindowWidth_ = settings.value("FixedWidth", 640).toInt();
  fixedWindowHeight_ = settings.value("FixedHeight", 480).toInt();
  lastWindowWidth_ = settings.value("LastWindowWidth", 640).toInt();
  lastWindowHeight_ = settings.value("LastWindowHeight", 480).toInt();
  lastWindowMaximized_ = settings.value("LastWindowMaximized", false).toBool();
  rememberWindowSize_ = settings.value("RememberWindowSize", true).toBool();
  tintWindow_ = settings.value("TintWindow", 1).toInt() != 0;
  sidebarTint_ = qBound(0, settings.value("SidebarTint", 16).toInt(), 100);
  windowTopBlurHeight_ = qMax(0, settings.value("WindowTopBlurHeight", 40).toInt());
  windowTopBlurPower_ = qMax(0, settings.value("WindowTopBlurPower", 10).toInt());
  windowTopBlurOpacity_ = qBound(0, settings.value("WindowTopBlurOpacity", 255).toInt(), 255);
  windowTopBlurSaturation_ = qMax(0, settings.value("WindowTopBlurSaturation", 100).toInt());
  windowTopBlurBrightness_ = qMax(0, settings.value("WindowTopBlurBrightness", 100).toInt());
  windowTopBlurKeyColor_ = QColor(settings.value("WindowTopBlurKeyColor", "#282729").toString());
  windowTopBlurKeyTolerance_ = qMax(0, settings.value("WindowTopBlurKeyTolerance", 40).toInt());
  searchBarExpandWidth_ = qMax(0, settings.value("SearchBarExpandWidth", 1110).toInt());
  viewModeCollapseWidth_ = qMax(0, settings.value("ViewModeCollapseWidth", 700).toInt());
  contentTopPadding_ = qMax(0, settings.value("ContentTopPadding", 0).toInt());
  toolbarIconPadding_ = qMax(0, settings.value("ToolbarIconPadding", 6).toInt());
  sidePaneTopOffset_ = settings.value("SidePaneTopOffset", 0).toInt();
  breadcrumbIconSize_ = qMax(0, settings.value("BreadcrumbIconSize", 16).toInt());
  scrollBarWidth_ = qMax(1, settings.value("ScrollBarWidth", 16).toInt());
  toolbarIconInnerSize_ = qMax(1, settings.value("ToolbarIconInnerSize", 20).toInt());
  toolbarIconOuterSize_ = qMax(1, settings.value("ToolbarIconOuterSize", 32).toInt());
  backForwardGroupLeftOffset_ = settings.value("BackForwardGroupLeftOffset", 0).toInt();
  backForwardGroupMarginLeft_ = settings.value("BackForwardGroupMarginLeft", 4).toInt();
  backForwardGroupMarginRight_ = settings.value("BackForwardGroupMarginRight", 4).toInt();
  backForwardGroupHeight_ = qMax(0, settings.value("BackForwardGroupHeight", 0).toInt());
  backForwardGroupWidth_ = qMax(0, settings.value("BackForwardGroupWidth", 0).toInt());
  viewOptionsGroupMarginLeft_ = settings.value("ViewOptionsGroupMarginLeft", 4).toInt();
  viewOptionsGroupMarginRight_ = settings.value("ViewOptionsGroupMarginRight", 4).toInt();
  viewOptionsGroupHeight_ = qMax(0, settings.value("ViewOptionsGroupHeight", 0).toInt());
  viewOptionsGroupWidth_ = qMax(0, settings.value("ViewOptionsGroupWidth", 0).toInt());
  iconGroupingGroupMarginLeft_ = settings.value("IconGroupingGroupMarginLeft", 0).toInt();
  iconGroupingGroupMarginRight_ = settings.value("IconGroupingGroupMarginRight", 0).toInt();
  iconGroupingGroupHeight_ = qMax(0, settings.value("IconGroupingGroupHeight", 0).toInt());
  iconGroupingGroupWidth_ = qMax(0, settings.value("IconGroupingGroupWidth", 0).toInt());
  shareTagsMoreGroupMarginLeft_ = settings.value("ShareTagsMoreGroupMarginLeft", 4).toInt();
  shareTagsMoreGroupMarginRight_ = settings.value("ShareTagsMoreGroupMarginRight", 4).toInt();
  shareTagsMoreGroupHeight_ = qMax(0, settings.value("ShareTagsMoreGroupHeight", 0).toInt());
  shareTagsMoreGroupWidth_ = qMax(0, settings.value("ShareTagsMoreGroupWidth", 0).toInt());
  searchButtonGroupMarginLeft_ = settings.value("SearchButtonGroupMarginLeft", 0).toInt();
  searchButtonGroupMarginRight_ = settings.value("SearchButtonGroupMarginRight", 0).toInt();
  searchButtonGroupHeight_ = qMax(0, settings.value("SearchButtonGroupHeight", 0).toInt());
  searchButtonGroupWidth_ = qMax(0, settings.value("SearchButtonGroupWidth", 0).toInt());
  mainWindowTint_ = qBound(0, settings.value("MainWindowTint", 4).toInt(), 100);
  transparency_ = settings.value("Transparency", 1).toInt() != 0;
  transparencyPower_ = qBound(0, settings.value("TransparencyPower", 25).toInt(), 100);
  hiddenSidebarItems_ = settings.value("HiddenSidebarItems").toStringList();
  alwaysShowTabs_ = settings.value("AlwaysShowTabs", false).toBool(); // probonod: false instead of true, here too (like further above)
  showTabClose_ = settings.value("ShowTabClose", true).toBool();
  splitterPos_ = settings.value("SplitterPos", 260).toInt();
  sidePaneMode_ = sidePaneModeFromString(settings.value("SidePaneMode").toString());
  settings.endGroup();

  return true;
}

bool Settings::saveFile(QString filePath) {
  QSettings settings(filePath, QSettings::IniFormat);

  settings.beginGroup("System");
  settings.setValue("FallbackIconThemeName", fallbackIconThemeName_);
  settings.setValue("SuCommand", suCommand_);
  settings.setValue("Terminal", terminal_);
  settings.setValue("Archiver", archiver_);
  settings.setValue("SIUnit", siUnit_);

  settings.setValue("OnlyUserTemplates", onlyUserTemplates_);
  settings.setValue("OemplateTypeOnce", templateTypeOnce_);
  settings.setValue("TemplateRunApp", templateRunApp_);

  settings.endGroup();

  settings.beginGroup("Behavior");
  settings.setValue("BookmarkOpenMethod", bookmarkOpenMethodToString(bookmarkOpenMethod_));
  // settings for use with libfm
  settings.setValue("SingleClick", singleClick_);
  settings.setValue("SpatialMode", spatialMode_);
  settings.setValue("DirInfoWrite", dirInfoWrite_);
  settings.setValue("AutoSelectionDelay", autoSelectionDelay_);
  settings.setValue("ConfirmDelete", confirmDelete_);
  settings.setValue("NoUsbTrash", noUsbTrash_);
  settings.setValue("ConfirmTrash", confirmTrash_);
  settings.setValue("QuickExec", quickExec_);
  // bool thumbnailLocal_;
  // bool thumbnailMax;
  settings.endGroup();

  settings.beginGroup("Desktop");
  settings.setValue("WallpaperMode", wallpaperModeToString(wallpaperMode_));
  settings.setValue("Wallpaper", wallpaper_);
  settings.setValue("BgColor", desktopBgColor_.name());
  settings.setValue("FgColor", desktopFgColor_.name());
  settings.setValue("ShadowColor", desktopShadowColor_.name());
  settings.setValue("ShowHidden", desktopShowHidden_);
  settings.setValue("SortOrder", sortOrderToString(desktopSortOrder_));
  settings.setValue("SortColumn", sortColumnToString(desktopSortColumn_));
  settings.endGroup();

  settings.beginGroup("Volume");
  settings.setValue("MountOnStartup", mountOnStartup_);
  settings.setValue("MountRemovable", mountRemovable_);
  settings.setValue("AutoRun", autoRun_);
  settings.setValue("CloseOnUnmount", closeOnUnmount_);
  settings.endGroup();

  settings.beginGroup("Thumbnail");
  settings.setValue("ShowThumbnails", showThumbnails_);
  settings.setValue("MaxThumbnailFileSize", maxThumbnailFileSize());
  settings.setValue("ThumbnailLocalFilesOnly", thumbnailLocalFilesOnly());
  settings.endGroup();

  settings.beginGroup("FolderView");
  settings.setValue("Mode", viewModeToString(viewMode_));
  settings.setValue("ShowHidden", showHidden_);
  settings.setValue("SortOrder", sortOrderToString(sortOrder_));
  settings.setValue("SortColumn", sortColumnToString(sortColumn_));
  settings.setValue("SortFolderFirst", sortFolderFirst_);
  settings.setValue("ShowFilter", showFilter_);

  settings.setValue("BackupAsHidden", backupAsHidden_);
  settings.setValue("ShowFullNames", showFullNames_);
  settings.setValue("ShadowHidden", shadowHidden_);

  // override config in libfm's FmConfig
  settings.setValue("BigIconSize", bigIconSize_);
  settings.setValue("SmallIconSize", smallIconSize_);
  settings.setValue("SidePaneIconSize", sidePaneIconSize_);
  settings.setValue("SidePaneItemSpacing", sidePaneItemSpacing_);
  settings.setValue("SidePaneItemHorizontalPadding", sidePaneItemHorizontalPadding_);
  settings.setValue("SidePaneIconTextSpacing", sidePaneIconTextSpacing_);
  settings.setValue("SidePaneCategorySpacing", sidePaneCategorySpacing_);
  settings.setValue("SidePaneCategorySpacingBottom", sidePaneCategorySpacingBottom_);
  settings.setValue("SidePaneItemFontSize", sidePaneItemFontSize_);
  settings.setValue("SidePaneCategoryFontSize", sidePaneCategoryFontSize_);
  settings.setValue("SidePaneCategoryLeftPadding", sidePaneCategoryLeftPadding_);
  settings.setValue("SidePaneSelectorRightInset", sidePaneSelectorRightInset_);
  settings.setValue("SidePaneSelectorColor", sidePaneSelectorColor_.name());
  settings.setValue("SidePaneSelectorOpacity", sidePaneSelectorOpacity_);
  settings.setValue("SidePaneScrollBarMode", sidePaneScrollBarMode_);
  settings.setValue("SidePaneFontWeight", sidePaneFontWeight_);
  settings.setValue("GridSpacingTop", gridSpacingTop_);
  settings.setValue("GridSpacingBottom", gridSpacingBottom_);
  settings.setValue("GridSpacingLeft", gridSpacingLeft_);
  settings.setValue("GridSpacingRight", gridSpacingRight_);
  settings.setValue("IconViewTextSize", iconViewTextSize_);
  settings.setValue("IconViewTextAnchor", iconViewTextAnchor_);
  settings.setValue("IconViewTextMarginTop", iconViewTextMarginTop_);
  settings.setValue("IconViewTextMarginBottom", iconViewTextMarginBottom_);
  settings.setValue("IconViewTextWeight", iconViewTextWeight_);
  settings.setValue("ShowItemCount", showFolderItemCount_);
  settings.setValue("ThumbnailIconSize", thumbnailIconSize_);
  settings.setValue("UseStacks", useStacks_);
  settings.endGroup();

  settings.beginGroup("Window");
  settings.setValue("FixedWidth", fixedWindowWidth_);
  settings.setValue("FixedHeight", fixedWindowHeight_);
  settings.setValue("LastWindowWidth", lastWindowWidth_);
  settings.setValue("LastWindowHeight", lastWindowHeight_);
  settings.setValue("LastWindowMaximized", lastWindowMaximized_);
  settings.setValue("RememberWindowSize", rememberWindowSize_);
  settings.setValue("TintWindow", tintWindow_ ? 1 : 0);
  settings.setValue("SidebarTint", sidebarTint_);
  settings.setValue("WindowTopBlurHeight", windowTopBlurHeight_);
  settings.setValue("WindowTopBlurPower", windowTopBlurPower_);
  settings.setValue("WindowTopBlurOpacity", windowTopBlurOpacity_);
  settings.setValue("WindowTopBlurSaturation", windowTopBlurSaturation_);
  settings.setValue("WindowTopBlurBrightness", windowTopBlurBrightness_);
  settings.setValue("WindowTopBlurKeyColor", windowTopBlurKeyColor_.name());
  settings.setValue("WindowTopBlurKeyTolerance", windowTopBlurKeyTolerance_);
  settings.setValue("SearchBarExpandWidth", searchBarExpandWidth_);
  settings.setValue("ViewModeCollapseWidth", viewModeCollapseWidth_);
  settings.setValue("ContentTopPadding", contentTopPadding_);
  settings.setValue("ToolbarIconPadding", toolbarIconPadding_);
  settings.setValue("SidePaneTopOffset", sidePaneTopOffset_);
  settings.setValue("BreadcrumbIconSize", breadcrumbIconSize_);
  settings.setValue("ScrollBarWidth", scrollBarWidth_);
  settings.setValue("ToolbarIconInnerSize", toolbarIconInnerSize_);
  settings.setValue("ToolbarIconOuterSize", toolbarIconOuterSize_);
  settings.setValue("BackForwardGroupLeftOffset", backForwardGroupLeftOffset_);
  settings.setValue("BackForwardGroupMarginLeft", backForwardGroupMarginLeft_);
  settings.setValue("BackForwardGroupMarginRight", backForwardGroupMarginRight_);
  settings.setValue("BackForwardGroupHeight", backForwardGroupHeight_);
  settings.setValue("BackForwardGroupWidth", backForwardGroupWidth_);
  settings.setValue("ViewOptionsGroupMarginLeft", viewOptionsGroupMarginLeft_);
  settings.setValue("ViewOptionsGroupMarginRight", viewOptionsGroupMarginRight_);
  settings.setValue("ViewOptionsGroupHeight", viewOptionsGroupHeight_);
  settings.setValue("ViewOptionsGroupWidth", viewOptionsGroupWidth_);
  settings.setValue("IconGroupingGroupMarginLeft", iconGroupingGroupMarginLeft_);
  settings.setValue("IconGroupingGroupMarginRight", iconGroupingGroupMarginRight_);
  settings.setValue("IconGroupingGroupHeight", iconGroupingGroupHeight_);
  settings.setValue("IconGroupingGroupWidth", iconGroupingGroupWidth_);
  settings.setValue("ShareTagsMoreGroupMarginLeft", shareTagsMoreGroupMarginLeft_);
  settings.setValue("ShareTagsMoreGroupMarginRight", shareTagsMoreGroupMarginRight_);
  settings.setValue("ShareTagsMoreGroupHeight", shareTagsMoreGroupHeight_);
  settings.setValue("ShareTagsMoreGroupWidth", shareTagsMoreGroupWidth_);
  settings.setValue("SearchButtonGroupMarginLeft", searchButtonGroupMarginLeft_);
  settings.setValue("SearchButtonGroupMarginRight", searchButtonGroupMarginRight_);
  settings.setValue("SearchButtonGroupHeight", searchButtonGroupHeight_);
  settings.setValue("SearchButtonGroupWidth", searchButtonGroupWidth_);
  settings.setValue("MainWindowTint", mainWindowTint_);
  settings.setValue("Transparency", transparency_ ? 1 : 0);
  settings.setValue("TransparencyPower", transparencyPower_);
  settings.setValue("HiddenSidebarItems", hiddenSidebarItems_);
  settings.setValue("AlwaysShowTabs", alwaysShowTabs_);
  settings.setValue("ShowTabClose", showTabClose_);
  settings.setValue("SplitterPos", splitterPos_);
  settings.setValue("SidePaneMode", sidePaneModeToString(sidePaneMode_));
  settings.endGroup();
  return true;
}

static const char* bookmarkOpenMethodToString(OpenDirTargetType value) {
  switch(value) {
  case OpenInCurrentTab:
  default:
    return "current_tab";
  case OpenInNewTab:
    return "new_tab";
  case OpenInNewWindow:
    return "new_window";
  case OpenInLastActiveWindow:
    return "last_window";
  }
  return "";
}

static OpenDirTargetType bookmarkOpenMethodFromString(const QString str) {

  if(str == QStringLiteral("new_tab"))
    return OpenInNewTab;
  else if(str == QStringLiteral("new_window"))
    return OpenInNewWindow;
  else if(str == QStringLiteral("last_window"))
    return OpenInLastActiveWindow;
  return OpenInCurrentTab;
}

static const char* viewModeToString(Fm::FolderView::ViewMode value) {
  const char* ret;
  switch(value) {
    case Fm::FolderView::IconMode:
    default:
      ret = "icon";
      break;
    case Fm::FolderView::CompactMode:
      ret = "compact";
      break;
    case Fm::FolderView::DetailedListMode:
      ret = "detailed";
      break;
    case Fm::FolderView::ThumbnailMode:
      ret = "thumbnail";
      break;
  }
  return ret;
}

Fm::FolderView::ViewMode viewModeFromString(const QString str) {
  Fm::FolderView::ViewMode ret;
  if(str == "icon")
    ret = Fm::FolderView::IconMode;
  else if(str == "compact")
    ret = Fm::FolderView::CompactMode;
  else if(str == "detailed")
    ret = Fm::FolderView::DetailedListMode;
  else if(str == "thumbnail")
    ret = Fm::FolderView::ThumbnailMode;
  else
    ret = Fm::FolderView::IconMode;
  return ret;
}

static const char* sortOrderToString(Qt::SortOrder order) {
  return (order == Qt::DescendingOrder ? "descending" : "ascending");
}

static Qt::SortOrder sortOrderFromString(const QString str) {
  return (str == "descending" ? Qt::DescendingOrder : Qt::AscendingOrder);
}

static const char* sortColumnToString(Fm::FolderModel::ColumnId value) {
  const char* ret;
  switch(value) {
    case Fm::FolderModel::ColumnFileName:
    default:
      ret = "name";
      break;
    case Fm::FolderModel::ColumnFileType:
      ret = "type";
      break;
    case Fm::FolderModel::ColumnFileSize:
      ret = "size";
      break;
    case Fm::FolderModel::ColumnFileMTime:
      ret = "mtime";
      break;
    case Fm::FolderModel::ColumnFileOwner:
      ret = "owner";
      break;
  }
  return ret;
}

static Fm::FolderModel::ColumnId sortColumnFromString(const QString str) {
  Fm::FolderModel::ColumnId ret;
  if(str == "name")
      ret = Fm::FolderModel::ColumnFileName;
  else if(str == "type")
    ret = Fm::FolderModel::ColumnFileType;
  else if(str == "size")
    ret = Fm::FolderModel::ColumnFileSize;
  else if(str == "mtime")
    ret = Fm::FolderModel::ColumnFileMTime;
  else if(str == "owner")
    ret = Fm::FolderModel::ColumnFileOwner;
  else
    ret = Fm::FolderModel::ColumnFileName;
  return ret;
}

static const char* wallpaperModeToString(int value) {
  const char* ret;
  switch(value) {
    case DesktopWindow::WallpaperNone:
    default:
      ret = "none";
      break;
    case DesktopWindow::WallpaperTransparent:
      ret = "transparent";
     break;
    case DesktopWindow::WallpaperStretch:
      ret = "stretch";
      break;
    case DesktopWindow::WallpaperFit:
      ret = "fit";
      break;
    case DesktopWindow::WallpaperCenter:
      ret = "center";
      break;
    case DesktopWindow::WallpaperTile:
      ret = "tile";
      break;
  }
  return ret;
}

static int wallpaperModeFromString(const QString str) {
  int ret;
  if(str == "transparent")
    ret = DesktopWindow::WallpaperTransparent;
  else if(str == "stretch")
    ret = DesktopWindow::WallpaperStretch;
  else if(str == "fit")
    ret = DesktopWindow::WallpaperFit;
  else if(str == "center")
    ret = DesktopWindow::WallpaperCenter;
  else if(str == "tile")
    ret = DesktopWindow::WallpaperTile;
  else
    ret = DesktopWindow::WallpaperNone;
  return ret;
}

static const char* sidePaneModeToString(Fm::SidePane::Mode value) {
  const char* ret;
  switch(value) {
    case Fm::SidePane::ModePlaces:
    default:
      ret = "places";
      break;
    case Fm::SidePane::ModeDirTree:
      ret = "dirtree";
      break;
    case Fm::SidePane::ModeNone:
      ret = "none";
      break;
  }
  return ret;
}

static Fm::SidePane::Mode sidePaneModeFromString(const QString& str) {
  Fm::SidePane::Mode ret;
  if(str == "none")
    ret = Fm::SidePane::ModeNone;
  else if(str == "dirtree")
    ret = Fm::SidePane::ModeDirTree;
  else
    ret = Fm::SidePane::ModePlaces;
  return ret;
}

void Settings::setTerminal(QString terminalCommand) {
    terminal_ = terminalCommand;
    // override the settings in libfm FmConfig.
    g_free(fm_config->terminal);
    fm_config->terminal = g_strdup(terminal_.toLocal8Bit().constData());
    g_signal_emit_by_name(fm_config, "changed::terminal");
  }
