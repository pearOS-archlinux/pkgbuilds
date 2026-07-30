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

// A from-scratch MainWindow for this standalone Qt5 "empty Filer shell"
// project -- NOT a copy of ../mainwindow.h (that file holds the pearOS
// Welcome app's unrelated dialogs). What this class reproduces from Filer's
// real src/mainwindow.h/.cpp (4148+456 lines) is the full window-chrome
// machinery, not just a static approximation of its look:
//
//   - frameless window + traffic lights: MainWindow::MainWindow(), the
//     setWindowFlags/WA_TranslucentBackground/buildTrafficLights() block,
//     originally src/mainwindow.cpp:538-736 (see filer_titlebar.h/.cpp).
//   - the *floating unified toolbar*: removeToolBar()'d out of QMainWindow's
//     own reserved toolbar strip and raised as a plain overlay instead, so
//     the sidebar/content extend to y=0 underneath it instead of losing
//     kToolBarHeight worth of their own space -- originally
//     src/mainwindow.cpp:1181-1192 (updateToolbarOverlayGeometry(), src/
//     mainwindow.cpp:2953-2991) and the toolbar's own transparent-background
//     Paint-event handling (its "consume the native background paint
//     entirely" branch in eventFilter(), src/mainwindow.cpp:1799-1810).
//   - the window-outline stroke overlay: a raised sibling kept above every
//     other child so the 1px 20%-opacity outline always wins the paint
//     order, originally src/mainwindow.cpp:1236-1241 (ctor),
//     src/mainwindow.cpp:3220-3232ish (updateWindowStrokeOverlayGeometry())
//     and its Paint-event handling, src/mainwindow.cpp:1721-1741.
//   - the toolbar hairline that fades in on a 1.5s titlebar hover (or stays
//     out otherwise) via a 200ms QVariantAnimation, originally
//     src/mainwindow.cpp:1248-1266 (ctor), :3030-3049
//     (animateToolbarHairlineTo()/scheduleToolbarHairlineFadeOut()), the
//     Enter/Leave branches in eventFilter() (src/mainwindow.cpp:1822-1835),
//     and its own fill in paintEvent() (src/mainwindow.cpp:3630-3643).
//   - real KWin blur-behind on the sidebar column (not just a painted tint)
//     via KWindowEffects::enableBlurBehind(), originally
//     updateSidebarBlurRegion() (src/mainwindow.cpp:2877-2894).
//   - rounded corners + frosted sidebar column vs. tinted content area:
//     MainWindow::paintEvent() and MainWindow::mainAreaTintColor(),
//     originally src/mainwindow.cpp:3549-3654 (real tint formula ported
//     below in loadSettings()/paintEvent(), not approximated).
//   - the sidebar's look (Fm::SidePane's pure-Qt pieces, see
//     filer_sidebar.h/.cpp) and its Favorites/Locations item list, adapted
//     from Fm::PlacesModel::PlacesModel() (src/placesmodel.cpp:190-460) --
//     with real bookmarks (fm_bookmarks_get_all(), glib/libfm-qt) and the
//     pCloud/PearDrop hardware-specific entries left out. Pulled out into
//     its own Sidebar class -- see sidebar.h/.cpp for exactly what's kept.
//
// Deliberately NOT ported: the top scroll-blur band (topBlurOverlay_'s
// paintBlurBand(), src/mainwindow.cpp:1700-1719) samples live pixels out of
// Filer's own DesktopWindow (the desktop-icons/wallpaper widget) as content
// scrolls underneath it -- there's no folder view here to scroll, and no
// DesktopWindow to sample from without pulling in that whole subsystem.
// updateSidebarWallpaperTint() (retinting the sidebar from the wallpaper
// color directly behind the window) IS ported, Wayland-only -- this project
// has no X11 support at all, so unlike real Filer (which drives it from
// frameGeometry() directly on X11 and only falls back to a KWin-script
// D-Bus relay on Wayland, see WindowGeometryTracker), this shell always
// goes through that relay (see main.cpp's WindowGeometryRelay/
// loadWindowTrackerKwinScript()). The statusbar/breadcrumb overlay isn't
// here either: there's no folder path to show a breadcrumb trail for.
// Everything else in the real MainWindow -- tabs, folder view, path bar,
// menus, file operations, FmPath, bookmarks -- is deliberately not here.
// The central widget is a genuinely empty QWidget.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QString>
#include <QPixmap>

class QListWidget;
class QSplitter;
class QToolBar;
class QSettings;
class QResizeEvent;
class QMoveEvent;
class QShowEvent;
class QScreen;
class QVariantAnimation;
class QTimer;
class QStackedWidget;
class QLabel;
class QPushButton;
class QCheckBox;
class QTextBrowser;
class QNetworkAccessManager;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    // Ported from MainWindow::updateSidebarWallpaperTint()
    // (src/mainwindow.cpp:2902-2929). Public so main.cpp's
    // WindowGeometryRelay (this project's Wayland-only KWin-script D-Bus
    // counterpart) can feed it real compositor-side geometry directly --
    // this shell only ever has the one window, so there's no pid/caption
    // matching step like real Filer's WindowGeometryTracker needs.
    void updateSidebarWallpaperTint(const QRect& windowGlobalRect);

    // Average wallpaper color under a global-coordinate rect -- same
    // downsample-to-1px sampling trick as updateSidebarWallpaperTint(),
    // minus its sidebar-column restriction and tint-smoothing. Used by
    // AboutUsDialog (mainwindow.cpp) to pick a light-on-dark or
    // dark-on-light rendering of its logo depending on what's actually
    // behind it as it bounces around the screen.
    QColor sampleWallpaperColor(const QRect& globalRect);

protected:
    // Rounded corners (frameless-only) + the sidebar-column/content-area
    // tint split + the toolbar hairline fill -- ported from
    // MainWindow::paintEvent(), see mainwindow.cpp.
    void paintEvent(QPaintEvent* event) override;

    // Toolbar-is-the-drag-handle, toolbar hover Enter/Leave (hairline fade
    // countdown), toolbar's own Paint (consume the native background so the
    // chrome painted underneath shows through), and windowStrokeOverlay_'s
    // Paint (the outline stroke) -- ported from MainWindow::eventFilter(),
    // see mainwindow.cpp.
    bool eventFilter(QObject* watched, QEvent* event) override;

    // Repositions the floating toolBar_/windowStrokeOverlay_ overlays,
    // recomputes the KWin blur-behind region, and forces a full repaint --
    // ported from MainWindow::resizeEvent()/moveEvent()
    // (src/mainwindow.cpp:2312-2363). On some Wayland/KWin setups a
    // frameless+translucent top-level's backing store isn't reliably
    // repainted by the compositor after an interactive startSystemMove()
    // either, leaving the window showing stale (black) content until
    // something forces Qt to repaint every pixel again -- the explicit
    // update() calls here cover both cases.
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;

    // The ctor's blur-region/overlay-geometry calls run before the window is
    // shown, when the sidebar isn't visible/laid out yet -- ported from
    // MainWindow::showEvent() (src/mainwindow.cpp:2935-2946).
    void showEvent(QShowEvent* event) override;

    // Handles two unrelated QEvent::Type cases QMainWindow otherwise leaves
    // to changeEvent():
    //  - WindowStateChange: collapses the shadow margin (and the shadow
    //    drawn in it) to 0 while maximized -- a maximized frameless window
    //    should sit flush with the screen edge, not float inside its own
    //    shadow margin. Restored on de-maximize.
    //  - ActivationChange: greyscales the traffic lights and pauses
    //    tint/blur while some other window is active, same as the
    //    inactive-titlebar dimming most desktop shells do. Restored once
    //    this window is active again.
    void changeEvent(QEvent* event) override;

private:
    void loadSettings();

    // Ported from MainWindow::mainAreaTintColor() (src/mainwindow.cpp:3549-3557).
    QColor mainAreaTintColor() const;

    // Ported from the file-local sidebarColumnRight() helper
    // (src/mainwindow.cpp:2871-2875).
    int sidebarColumnRight() const;

    // Ported from MainWindow::updateToolbarOverlayGeometry()
    // (src/mainwindow.cpp:2953-2991), minus the ui.frame/ui.tabBar margin
    // reservation (no tab bar here).
    void updateToolbarOverlayGeometry();

    // Ported from MainWindow::updateWindowStrokeOverlayGeometry() (spans the
    // full window, same as windowStrokeOverlay_'s own rect).
    void updateWindowStrokeOverlayGeometry();

    // Ported from MainWindow::updateSidebarBlurRegion()
    // (src/mainwindow.cpp:2877-2894) -- real KWin blur-behind clipped to the
    // rounded sidebar column, not just a painted tint.
    void updateSidebarBlurRegion();

    // Rebuilds wallpaperPixmap_ for the screen the window currently lives on,
    // mirroring DesktopWindow::updateWallpaper()'s Stretch/Fit/Center/Tile
    // switch (src/desktopwindow.cpp:563-593).
    void loadWallpaperFromConfig(const QScreen* screen);

    // Filer's own [Desktop] Wallpaper config key (wallpaperFile_ below) is
    // only ever populated when Filer itself is acting as the desktop shell
    // -- on a normal Plasma desktop (this app's actual target) that key is
    // simply never set, silently falling back to bgColor_ instead and
    // tinting from the wrong color entirely. This reads Plasma's own
    // wallpaper config directly (~/.config/plasma-org.kde.plasma.desktop-
    // appletsrc's [Containments][ID][Wallpaper][org.kde.image][General]
    // Image= key, matched to this screen via that containment's
    // lastScreen=) so the sampled tint matches what's actually behind the
    // window.
    QString findPlasmaWallpaperPath(const QScreen* screen) const;

    // Ported from MainWindow::animateToolbarHairlineTo()/
    // scheduleToolbarHairlineFadeOut() (src/mainwindow.cpp:3030-3049).
    void animateToolbarHairlineTo(qreal target);
    void scheduleToolbarHairlineFadeOut();

    QString assetsBasePath_;

    static const int kToolBarHeight = 38;

    // Reserved band (left/right/bottom, top stays 0 like the toolbar-flush
    // original) the self-drawn drop shadow is painted into -- see
    // paintEvent(). Collapsed to 0 while maximized, see changeEvent().
    static const int kShadowMargin = 18;

    QToolBar* toolBar_ = nullptr;
    QSplitter* splitter_ = nullptr;
    QListWidget* sidebar_ = nullptr;
    QWidget* content_ = nullptr;
    QWidget* trafficLights_ = nullptr;

    // pearOS Welcome content -- see setupWelcomeContent() in mainwindow.cpp.
    // Ported from ../mainwindow.h's MainWindow::setupUi(), minus the
    // QWebEngineView-based "What's New?" window and QuickUpdate zip-install
    // flow (see welcome.h's header comment) -- "What's New?" renders in a
    // QTextBrowser page in-window instead (plain HTML, no JS/CSS -- see
    // loadWhatsNewPage()), rather than a full QWebEngineView (Qt5 WebEngine
    // isn't a dependency of this project).
    void setupWelcomeContent();

    // Fetches https://new.pearos.xyz/ via networkManager_ and renders it
    // into whatsNewBrowser_. Only fires once per run -- see whatsNewLoaded_.
    void loadWhatsNewPage();

    QString desktopEnv_;
    bool isLiveIso_ = false;
    QStackedWidget* pageStack_ = nullptr;
    QLabel* breadcrumbLabel_ = nullptr;
    QCheckBox* autostartSwitch_ = nullptr;
    QTextBrowser* whatsNewBrowser_ = nullptr;
    QNetworkAccessManager* networkManager_ = nullptr;
    bool whatsNewLoaded_ = false;

    // Mirrors isActiveWindow(), tracked separately so changeEvent()'s
    // ActivationChange branch can tell whether it just became active or
    // just became inactive -- see paintEvent()'s tint/blur pause and
    // buildTitleBar()'s greyscale effect on trafficLights_.
    bool windowActive_ = true;

    // Raised above every other child so the window's own outline stroke
    // always wins the paint order -- see eventFilter()'s Paint handling.
    QWidget* windowStrokeOverlay_ = nullptr;

    // Toolbar hairline fade state -- see animateToolbarHairlineTo().
    qreal toolbarHairlineOpacity_ = 0.0;
    QVariantAnimation* toolbarHairlineAnim_ = nullptr;
    QTimer* toolbarHairlineHoverTimer_ = nullptr;
    QTimer* toolbarHairlineIdleTimer_ = nullptr;

    // Works around a real Wayland/xdg-shell gap, not just a KWin quirk:
    // xdg_toplevel's interactive move (what startSystemMove() asks the
    // compositor to do) hands positioning entirely to the compositor by
    // design -- the client is never told its own new position, so no
    // QMoveEvent/moveEvent() fires during or after the drag at all (unlike
    // X11, where the WM sends real ConfigureNotify events Qt turns into
    // moveEvent()). That left moveEvent()'s own update() call as dead code
    // for exactly the case it was added for. A low-cost periodic repaint
    // instead of a move-driven one sidesteps needing that notification at
    // all -- ~5fps is imperceptible for a mostly-static shell window, and
    // guarantees the tint/blur/chrome never sits stale on screen for more
    // than this interval after being dragged.
    QTimer* repaintPumpTimer_ = nullptr;

    // [Window] settings read from ~/.config/filer/default/settings.conf --
    // see loadSettings(). Defaults match Settings' own ctor defaults
    // (src/settings.cpp:79-120) so this looks right even with no config
    // file present.
    bool tintWindow_ = true;
    int sidebarTint_ = 16;
    int mainWindowTint_ = 4;
    bool transparency_ = true;
    int transparencyPower_ = 25;

    // Sidebar blend-target color -- MainWindow's own sidebarTint_ member
    // (src/mainwindow.h:448ish), (28, 28, 28) fallback until
    // updateSidebarWallpaperTint() has real wallpaper pixels to sample.
    // Dynamically retinted from the wallpaper behind the sidebar column as
    // the window moves, same as the real app.
    QColor fixedTint_ = QColor(28, 28, 28);

    // [Desktop] keys read alongside loadSettings()'s [Window] group -- source
    // data for loadWallpaperFromConfig()/updateSidebarWallpaperTint().
    QString wallpaperFile_;
    int wallpaperMode_ = 2; // DesktopWindow::WallpaperStretch
    QColor bgColor_ = QColor(0x30, 0x30, 0x30);

    // Rebuilt by loadWallpaperFromConfig() whenever the window changes
    // screens; sampled by updateSidebarWallpaperTint().
    QPixmap wallpaperPixmap_;
    const QScreen* wallpaperScreen_ = nullptr;
};

#endif // MAINWINDOW_H
