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

#ifndef FM_MAIN_WINDOW_H
#define FM_MAIN_WINDOW_H

#include "ui_main-win.h"
#include <QMainWindow>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QTabWidget>
#include <libfm/fm.h>
#include <QMessageBox>
#include <QPointer>
#include <QTabBar>
#include <QStackedWidget>
#include <QSplitter>
#include "launcher.h"

class QToolButton;
class QVariantAnimation;

namespace Filer {

class TabPage;
class Settings;

class MainWindow : public QMainWindow {
Q_OBJECT
public:
  MainWindow(FmPath* path = NULL);
  virtual ~MainWindow();

  void chdir(FmPath* path);
  void addTab(FmPath* path);
  void addWindow(FmPath* path);

  TabPage* currentPage() {
    return reinterpret_cast<TabPage*>(ui.stackedWidget->currentWidget());
  }

  void updateFromSettings(Settings& settings);

  void disableMenuItems();

  void enableMenuItems();

protected Q_SLOTS:

  void onPathEntryReturnPressed();

  void on_actionNewTab_triggered();
  void on_actionNewWin_triggered();
  void on_actionNewFolder_triggered();
  void on_actionNewBlankFile_triggered();
  void on_actionCloseTab_triggered();
  void on_actionCloseWindow_triggered();
  void on_actionOpen_triggered(); // probono
  void on_actionOpenWith_triggered(); // probono
  void on_actionOpenAndCloseCurrentWindow_triggered(); // probono
  void on_actionShowContents_triggered(); // probono
  void on_actionFileProperties_triggered();
  void on_actionFolderProperties_triggered();

  void on_actionCut_triggered();
  void on_actionCopy_triggered();
  void on_actionPaste_triggered();
  void on_actionDuplicate_triggered(); // probono
  void on_actionEmptyTrash_triggered(); // probono
  void on_actionDelete_triggered();
  void on_actionDeleteWithoutTrash_triggered(); // probono
  void on_actionRename_triggered();
  void on_actionQuickLook_triggered();
  void on_actionSelectAll_triggered();
  void on_actionInvertSelection_triggered();
  void on_actionPreferences_triggered();

  void on_actionGoBack_triggered();
  void on_actionGoForward_triggered();
  void on_actionGoUp_triggered();
  void on_actionGoUpAndCloseCurrentWindow_triggered(); // probono
  void on_actionHome_triggered();
  void on_actionReload_triggered();

  void on_actionIconView_triggered();
  void on_actionDetailedList_triggered();
  void on_actionThumbnailView_triggered();
  void on_actionGoToFolder_triggered();

  void on_actionGo_triggered();
  void on_actionShowHidden_triggered(bool check);

  void on_actionByFileName_triggered(bool checked);
  void on_actionByMTime_triggered(bool checked);
  void on_actionByOwner_triggered(bool checked);
  void on_actionByFileType_triggered(bool checked);
  void on_actionByFileSize_triggered(bool checked);
  void on_actionAscending_triggered(bool checked);
  void on_actionDescending_triggered(bool checked);
  void on_actionFolderFirst_triggered(bool checked);
  void on_actionCaseSensitive_triggered(bool checked);
  void on_actionFilter_triggered(bool checked);

  void on_actionApplications_triggered();
  void on_actionUtilities_triggered();
  void on_actionDocuments_triggered();
  void on_actionDownloads_triggered();
  void on_actionComputer_triggered();
  void on_actionTrash_triggered();
  void on_actionNetwork_triggered();
  void on_actionDesktop_triggered();
  void on_actionAddToBookmarks_triggered();
  void on_actionEditBookmarks_triggered();

  void on_actionOpenTerminal_triggered();
  void on_actionOpenAsRoot_triggered();
  void on_actionFindFiles_triggered();

  void on_actionAbout_triggered();

  void onBookmarkActionTriggered();

  void onTabBarCloseRequested(int index);
  void onTabBarCurrentChanged(int index);
  void onTabBarTabMoved(int from, int to);

  void onFilterStringChanged(QString str);

  void onShortcutPrevTab();
  void onShortcutNextTab();
  void onShortcutJumpToTab();

  void onStackedWidgetWidgetRemoved(int index);

  void onTabPageTitleChanged(QString title);
  void onTabPageStatusChanged(int type, QString statusText);
  void onTabPageOpenDirRequested(FmPath* path, int target);
  void onTabPageSortFilterChanged();

  void onSidePaneChdirRequested(int type, FmPath* path);
  void onSidePaneOpenFolderInNewWindowRequested(FmPath* path);
  void onSidePaneOpenFolderInNewTabRequested(FmPath* path);
  void onSidePaneOpenFolderInTerminalRequested(FmPath* path);
  void onSidePaneCreateNewFolderRequested(FmPath* path);
  void onSidePaneModeChanged(Fm::SidePane::Mode mode);
  void onSplitterMoved(int pos, int index);

  void onBackForwardContextMenu(QPoint pos);

  void onRaiseWindow(const QString& path);
  void onCloseWindow(const QString& path);

  void onRaiseWindowAndSelectItems(const QString& path, const QStringList& items);

protected:
  // void changeEvent( QEvent * event);
  void closeTab(int index);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void moveEvent(QMoveEvent* event);
  virtual void closeEvent(QCloseEvent *event);
  virtual void paintEvent(QPaintEvent *event);
  virtual void showEvent(QShowEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);

private:
  void showOrRefreshQuickLook(bool allowToggleClose);
  void updateSidebarBlurRegion();
  void updateSidebarSpacerWidth();
  static void onBookmarksChanged(FmBookmarks* bookmarks, MainWindow* pThis);
  void loadBookmarksMenu();
  void updateUIForCurrentPage();
  // Window title / toolbar folder-name label text for tabPage: its plain
  // title() (e.g. "Documents"), plus " -- Local" (em dash) when the path is
  // a real native filesystem location -- matching Finder's own "Documents
  // -- Macbook Air" style title, minus the actual hostname since this isn't
  // tied to one. Excludes the "Recents" tab specifically: it's backed by a
  // real native directory of symlinks (see isRecentsPath() in tabpage.cpp),
  // so fm_path_is_native() alone can't tell it apart from a normal folder.
  QString titleBarTextFor(TabPage* tabPage) const;
  void updateViewMenuForCurrentPage();
  void updateStatusBarForCurrentPage();
  bool isSpatialMode() const;
  // Rebuilds the breadcrumb trail (volume name, then each path segment) for
  // the given folder path, replacing the old free-space "available" label.
  void updateBreadcrumb(const QString& pathName);
  // Wraps a set of toolbar actions in one pill-shaped container so they read
  // as a single segmented control instead of separate buttons. When
  // groupToolTip is non-empty, per-button tooltips are cleared in favor of
  // one tooltip on the whole group.
  QWidget* createToolbarButtonGroup(const QList<QAction*>& actions, const QString& groupToolTip = QString());
  // Applies every per-group margin/height/width setting (Settings::
  // backForwardGroupMarginLeft() and its ~19 siblings) plus the two global
  // icon-size settings (toolbarIconInnerSize()/toolbarIconOuterSize()) to
  // all five toolbar button groups -- called once at construction and
  // again from updateFromSettings() so they're all live-reloadable.
  void applyToolbarGroupSettings(Settings& settings);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  Ui::MainWindow ui;
  QWidget* sidebarSpacer_;
  // Widgets painted with the "Liquid Glass" backdrop (frosted fill + soft
  // outer shadow + inner bevel highlight, pulled from the pearOS Figma
  // design system node 483:9282) via eventFilter()'s QEvent::Paint branch:
  // the back/forward, view-mode, and share/tag/more toolbar button groups
  // (see createToolbarButtonGroup()), and the search field's holder (see
  // the ctor -- filterBar itself is transparent, its holder paints the
  // glass behind it). Deliberately NOT the traffic-light dots -- tried,
  // reverted per feedback. Corner radius defaults to half the widget's
  // height, giving a true pill/stadium shape (not an arbitrary fixed
  // radius) -- override via the "glassRadius" property (qreal) on a widget
  // before adding it here only if a panel genuinely needs a different
  // shape.
  QList<QWidget*> glassPanels_;
  static void paintLiquidGlassPanel(QWidget* w);
  // The toolbar's five button groups (see the ctor and
  // applyToolbarGroupSettings()) -- kept as members so live settings
  // reloads can restyle them without re-finding them via ui.toolBar.
  // backForwardGroup_/groupByGroup_/searchButtonGroup_ used to be local
  // ctor variables; promoted here for the same reason.
  // Dedicated spacer between sidebarSpacer_ and backForwardGroup_ (see
  // Settings::backForwardGroupLeftOffset()) -- the only one of the five
  // groups that sits next to another fixed toolbar element rather than
  // another group, so its own MarginLeft alone can't control that gap.
  QWidget* backForwardLeftSpacer_ = nullptr;
  QWidget* backForwardGroup_ = nullptr;
  QWidget* groupByGroup_ = nullptr;
  QWidget* shareTagsMoreGroup_ = nullptr;
  QWidget* searchButtonGroup_ = nullptr;
  QLineEdit* pathEntry;
  QLineEdit* filterBar; // pill-shaped search field, lives in the toolbar (see MainWindow ctor)
  // Collapsible search field (Settings::searchBarExpandWidth()): below the
  // threshold width, searchButton_ (a lone glass-circle icon) sits in the
  // toolbar and searchHolder_ (filterBar's glass pill, see the ctor) is
  // hidden; clicking searchButton_ swaps them and animates searchHolder_'s
  // width open (a bounce via QEasingCurve::OutBack). Losing focus while
  // still under the threshold reverses it. At/above the threshold,
  // searchHolder_ just stays shown/searchButton_ hidden, unconditionally
  // -- see updateSearchBarMode(), called from resizeEvent() and
  // updateFromSettings().
  QToolButton* searchButton_ = nullptr;
  QWidget* searchHolder_ = nullptr;
  // QToolBar::addWidget() returns the QAction that actually represents the
  // widget in the toolbar's own internal layout (QToolBarLayout) -- toggling
  // visibility via widget->setVisible() directly left QToolBarLayout's own
  // bookkeeping unaware of the change, and its next relayout pass (queued,
  // not synchronous, so it silently happened on a later event loop tick
  // after any of our own code had run) re-asserted the widget visible again
  // regardless of what we'd just set. Toggling these actions' visibility
  // instead keeps the toolbar's layout in sync, so it stays hidden/shown
  // for real. See updateSearchBarMode()/updateViewModeCollapseMode().
  QAction* searchHolderAction_ = nullptr;
  QAction* searchButtonAction_ = nullptr;
  QVariantAnimation* searchAnim_ = nullptr;
  bool searchManuallyExpanded_ = false;
  void updateSearchBarMode();
  void animateSearchBar(int fromWidth, int toWidth, bool collapseAtEnd);
  // Collapsible view-mode segmented control (Settings::viewModeCollapseWidth()):
  // below the threshold, viewModeGroup_ (the 4-button Icon/List/Column/
  // Gallery segmented control) hides in favor of viewModeCollapsedButton_,
  // a single round glass button (gallery-view icon + a chevron, composited
  // into one icon -- see the ctor) whose popup menu holds the same 4
  // actions. No animation here (unlike the search bar): this is a passive
  // toolbar-overflow response to width, not a user-initiated reveal.
  QWidget* viewModeGroup_ = nullptr;
  QToolButton* viewModeCollapsedButton_ = nullptr;
  QAction* viewModeGroupAction_ = nullptr;
  QAction* viewModeCollapsedButtonAction_ = nullptr;
  void updateViewModeCollapseMode();
  // (Re)applies ui.toolBar's stylesheet -- factored out of the ctor so
  // Settings::toolbarIconPadding() can be live-reloaded (see
  // MainWindow::updateFromSettings()) without duplicating the rest of the
  // rule set.
  void updateToolbarStyleSheet();
  // Settings::scrollBarWidth(): a single QScrollBar width/height override
  // set on `this`, cascading via Qt's own stylesheet inheritance to every
  // scrollbar in the window (folder view, sidebar, column view...) without
  // touching their native handle/groove/arrow appearance -- see the doc
  // comment on Settings::scrollBarWidth().
  void updateScrollBarStyleSheet();
  QLabel* folderNameLabel_; // Finder-style bold folder name shown instead of pathEntry until Ctrl+L
  QAction* pathEntryAction_;
  QAction* folderNameLabelAction_;
  QAction* actionGalleryView_;
  QAction* actionColumnView_;
  QWidget* breadcrumbBar_; // path breadcrumbs, left side of the status bar
  QHBoxLayout* breadcrumbLayout_;
  QWidget* breadcrumbSpacer_; // insets breadcrumbBar_ to start under the content area, not the sidebar
  FmBookmarks* bookmarks;
  Launcher fileLauncher_;

  // Unified/vibrancy toolbar: ui.toolBar is pulled out of QMainWindow's
  // managed top toolbar area (see removeToolBar() in the ctor) and
  // positioned as a floating overlay instead, so the folder view's content
  // extends up underneath it and is visible (blurred) through it while
  // scrolling, like Finder's unified toolbar -- see updateToolbarOverlayGeometry()
  // and paintBlurBand().
  void updateToolbarOverlayGeometry();
  // Same treatment, bottom edge, for the breadcrumb bar -- but the real
  // ui.statusbar isn't reused as the floating widget itself the way
  // ui.toolBar is: an earlier attempt called setStatusBar(nullptr) to
  // detach it the same way removeToolBar() detaches a toolbar, but
  // QMainWindow::setStatusBar() unconditionally schedules whatever status
  // bar it's currently holding for deleteLater() whenever you pass it a
  // different one (nullptr included) -- there's no removeStatusBar()
  // equivalent. ui.statusbar was still technically alive for the next few
  // lines (deleteLater() defers actual destruction), so nothing crashed
  // immediately, but it was gone by the next event-loop turn, and every
  // pointer this code kept to it (raised, positioned, repainted) was then
  // dangling -- crashed on the first real repaint/resize after show().
  // So instead: ui.statusbar is shrunk to zero height (still alive, still
  // technically "the" status bar QMainWindow knows about, just reserving
  // ~0 space in its managed layout) so ui.splitter -- and the sidebar's
  // own item view inside it -- genuinely reclaims that height rather than
  // just visually overlapping it, and its actual content
  // (breadcrumbSpacer_/breadcrumbBar_) is moved onto this fresh widget,
  // floated the same way ui.toolBar is. See the ctor.
  QWidget* statusBarOverlay_;
  void updateStatusBarOverlayGeometry();
  // The same faint wallpaper-hinted tint MainWindow::paintEvent() gives the
  // toolbar/content area (as opposed to the sidebar column's own, stronger
  // tint) -- factored out so statusBarOverlay_'s own paint handling (see
  // eventFilter()) can fill itself with the identical color instead of
  // sitting transparent over whatever ui.splitter paints directly behind it.
  QColor mainAreaTintColor() const;
  // Dedicated widget for the window-wide top blur band (Settings::
  // windowTopBlurHeight()) -- a raised sibling of ui.splitter/ui.toolBar,
  // stacked ABOVE the folder/sidebar items but BELOW ui.toolBar's own
  // children (buttons, traffic lights), so it reads as a frosted veil over
  // the items instead of hiding uselessly underneath them (which is what
  // painting it in MainWindow::paintEvent() did -- parents always paint
  // before children, so ui.splitter's own items covered it completely).
  // Transparent for mouse events so clicks/scrolls still reach whatever's
  // under it. Painted via its own QEvent::Paint, handled in eventFilter()
  // same way ui.toolBar's is.
  QWidget* topBlurOverlay_;
  void updateTopBlurOverlayGeometry();
  // The window-outline hairline stroke (see paintEvent()'s doc comment)
  // used to be painted directly in MainWindow::paintEvent(), which runs
  // before any child widget -- wherever a raised child's own content
  // reaches exactly to the window's edge (statusBarOverlay_'s bottom, ui.
  // toolBar's top corners), that child painted over the stroke there,
  // leaving visible gaps in the outline. This dedicated widget is raised
  // above every other child (see the ctor and each place a sibling gets
  // raised, e.g. updateToolbarOverlayGeometry()/updateStatusBarOverlayGeometry())
  // so the stroke always wins regardless of what else changed z-order.
  // Covers the whole window and is transparent to mouse events, like
  // topBlurOverlay_.
  QWidget* windowStrokeOverlay_;
  void updateWindowStrokeOverlayGeometry();
  // Grabs a live snapshot of `source` under rect `r`, blurs it, feathers
  // its bottom edge to transparent, clips its top corners to `cornerRadius`
  // per roundLeft/roundRight, and draws the result via `p` (bound to
  // `destWidget`, used to map `r` into `source`'s coordinate space).
  // Used by paintEvent() for the window-wide top blur band -- see
  // Settings::windowTopBlurHeight(). `p` is bound to `this`/MainWindow
  // (not ui.toolBar, the old floating-toolbar-only blur band this
  // replaced) so the band's height isn't capped at the toolbar's own.
  // saturation/brightness are multipliers (1.0 = unchanged) applied to the
  // snapshot's HSV S/V channels before blurring -- see
  // Settings::windowTopBlurSaturation()/windowTopBlurBrightness(). tintOpacity
  // (0..1, see Settings::windowTopBlurOpacity()) is NOT a `p.setOpacity()`
  // applied to the whole band -- that would blend the blurred pixmap against
  // the real, live (sharp) content underneath (topBlurOverlay_ sits above
  // ui.splitter but is translucent-backed), so any opacity below 1.0 let
  // sharp icon/text edges "leak through" the blur while the flat background
  // looked fully blurred (same blend ratio, but background has no
  // high-frequency detail to leak). Instead tintOpacity blends `tintColor`
  // as a frosted-glass veil ON TOP of the fully-opaque blurred content, so
  // the band always completely covers/replaces the live content -- items
  // read as genuinely blurred, not part-sharp. tintColor should be a color
  // that actually differs from the blurred content (e.g. sidebarTint_, the
  // wallpaper-derived accent already used for the rest of the window's
  // vibrancy in paintEvent()) -- blending toward plain QPalette::Window gray
  // is imperceptible against already-dark blurred content regardless of
  // alpha. baseColor is separate: just the flat fallback fill used when
  // there's nothing to grab/blur (source rect empty). keyColor/keyTolerance
  // (see Settings::windowTopBlurKeyColor()/windowTopBlurKeyTolerance()) key
  // out a range of shades close to keyColor to transparent before blurring
  // -- keyTolerance <= 0 disables this entirely.
  void paintBlurBand(QPainter& p, QWidget* destWidget, const QRect& r, QWidget* source, bool roundLeft, bool roundRight,
                      const QColor& baseColor, int bandBlurRadius, qreal cornerRadius,
                      qreal saturation = 1.0, qreal brightness = 1.0, qreal tintOpacity = 0.0,
                      const QColor& tintColor = QColor(), const QColor& keyColor = QColor(),
                      int keyTolerance = 0) const;
  QTimer* toolbarBlurTimer_;
  // QPointer (not a raw pointer): FolderView::setViewMode() synchronously
  // `delete`s its old child QListView/QTreeView when switching to/from
  // Detailed List mode. If a repaint reaches code using this pointer at
  // exactly that moment (e.g. via the view-mode toolbar button's own
  // mouseReleaseEvent repaint cascade, re-entering through
  // MainWindow::eventFilter()), a raw pointer here could already be
  // dangling rather than null -- a plain `if(!v)` guard only catches null.
  // QPointer auto-clears itself when the pointee is destroyed, so the
  // guard actually works. (This crashed for real on startup before this
  // fix -- see the git history for the reproduction.)
  QPointer<QAbstractItemView> toolbarBlurLastSidebarView_;
  // Extracted from toolbarBlurTimer_'s own lambda (see the ctor) so
  // resizeEvent() can also call it directly -- the 80ms timer alone left a
  // visible lag/flicker during a live drag-resize (many resize events per
  // tick), since the scrollbar range/geometry override below only caught
  // up periodically instead of tracking the resize in step.
  void applyScrollInsets();

  // Toolbar/titlebar hairline (the one below ui.toolBar, painted in
  // paintEvent()): invisible at rest, fading in on either (a) hovering the
  // titlebar for kToolbarHairlineHoverDelayMs, or (b) scrolling the current
  // folder view, then fading back out once neither condition holds. The
  // breadcrumb bar's own hairline is NOT part of this -- it stays always
  // visible (see its own paint handling in eventFilter()).
  qreal toolbarHairlineOpacity_ = 0.0;
  QVariantAnimation* toolbarHairlineAnim_;
  QTimer* toolbarHairlineHoverTimer_; // single-shot; fires -> fade in
  QTimer* toolbarHairlineIdleTimer_;  // single-shot; fires -> fade out
  // Last-seen content view scrollbar value, so applyScrollInsets() (already
  // polling every 80ms) can detect an actual scroll without a dedicated
  // signal connection per view (views come and go with the current tab).
  int toolbarHairlineLastScrollValue_ = 0;
  bool toolbarHairlineHasLastScrollValue_ = false;
  void animateToolbarHairlineTo(qreal target);
  void scheduleToolbarHairlineFadeOut();

public:
  // Retints the sidebar column from the wallpaper behind windowGlobalRect
  // (this window's own on-screen geometry) -- macOS-style vibrancy. On
  // Wayland this is fed by WindowGeometryTracker (see application.cpp),
  // since a Wayland client can't query its own global position; on X11,
  // moveEvent()/resizeEvent() call it directly with frameGeometry().
  void updateSidebarWallpaperTint(const QRect& windowGlobalRect);

private:
  QColor sidebarTint_;
};

}

#endif // FM_MAIN_WINDOW_H
