/*
 * Adapted from plasma-sdk's plasmoidviewer main.cpp
 * (SPDX-FileCopyrightText: 2007 Frerich Raabe <raabe@kde.org>,
 *  SPDX-FileCopyrightText: 2007-2008 Aaron Seigo <aseigo@kde.org>)
 * for filer-dock: hardcoded to host the PearDock applet as a horizontal,
 * bottom-edge panel instead of taking a generic --applet/--containment CLI.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QQuickItem>
#include <QQuickWindow>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QDBusConnection>

#include <LayerShellQt/window.h>

#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>

#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <dlfcn.h>
#include <unistd.h>

#include <algorithm>

#include "plasmoidshim.h"
#include "view.h"

#include <QDBusAbstractAdaptor>

// Lets filer-topbar's search widget (a separate process) trigger
// pearos-dock-src's own Spotlight-style search popup (PearLauncher.qml's
// launcherDialog, opened via its openSearch() function) over DBus, instead
// of building a second search UI. Navigates root.taskList.pearLauncherItem
// via plain QObject::property() -- both are QML `property alias`es already
// exposed on the loaded main.qml (root.taskList: main.qml:51,
// taskList.pearLauncherItem: main.qml:1039) -- so this needs no change to
// pearos-dock-src, which stays vendored unmodified.
class SearchAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.pearos.FilerDock.Search")

public:
    explicit SearchAdaptor(QObject *rootObject)
        : QDBusAbstractAdaptor(rootObject)
        , m_rootObject(rootObject)
    {
    }

public Q_SLOTS:
    void openSearch()
    {
        QObject *taskList = m_rootObject->property("taskList").value<QObject *>();
        QObject *pearLauncherItem = taskList ? taskList->property("pearLauncherItem").value<QObject *>() : nullptr;
        if (pearLauncherItem) {
            QMetaObject::invokeMethod(pearLauncherItem, "openSearch");
        } else {
            qWarning() << "SearchAdaptor::openSearch: could not find taskList.pearLauncherItem";
        }
    }

private:
    QObject *m_rootObject;
};

// Popups (PearLauncher, PearFolderArc, task tooltips) use PlasmaCore.Dialog,
// which positions itself by setting QWindow x/y and relying on libplasma's
// PlasmaShellWaylandIntegration to forward each QEvent::Move to KWin via
// org_kde_plasma_shell set_position -- which KWin honors for any client
// (confirmed on the wire with WAYLAND_DEBUG=1: the task tooltip's
// set_position was applied verbatim). Two client-side breaks kept the
// positions wrong anyway:
//
// 1. QtWayland rewrites any client-set position on a mapped toplevel back
//    to the screen origin (QWaylandWindow::setGeometry, gated by the
//    static QWaylandWindow::fixedToplevelPositions, default true; Qt::Popup
//    and Qt::ToolTip windows are exempt). So PearLauncher's centered x/y --
//    assigned via Qt.callLater after its plain-toplevel dialog was already
//    mapped -- was discarded before generating the QEvent::Move that would
//    have reached the wire: the only set_position ever sent was the initial
//    (0,0), and KWin faithfully pinned it top-left. Disabled below.
//
// 2. A Wayland client can't query its own on-screen position, so this
//    window's Qt-side position stayed (0,0) and every popup positioned
//    relative to it (tooltips, PearFolderArc -- the visualParent path)
//    computed "above the dock" as "above y=0", landing at the top of the
//    screen. Fixed by View::syncQtWindowPosition(), which mirrors the
//    known layer-shell anchoring back into QWindow.

// Mirrors pearos-dock-src/package/contents/config/main.xml's <default>s --
// PlasmoidConfig has no KConfigLoader/kcfg parsing behind it (see
// plasmoidshim.h), so the defaults that file used to supply have to be
// seeded here instead, once, before main.qml is loaded.
static void seedPlasmoidConfigDefaults(PlasmoidConfig *config)
{
    config->seed(QStringLiteral("showOnlyCurrentScreen"), false);
    config->seed(QStringLiteral("showOnlyCurrentDesktop"), true);
    config->seed(QStringLiteral("showOnlyCurrentActivity"), true);
    config->seed(QStringLiteral("showOnlyMinimized"), false);
    config->seed(QStringLiteral("unhideOnAttention"), true);
    config->seed(QStringLiteral("groupingStrategy"), 1);
    config->seed(QStringLiteral("groupedTaskVisualization"), 0);
    config->seed(QStringLiteral("groupPopups"), true);
    config->seed(QStringLiteral("onlyGroupWhenFull"), true);
    config->seed(QStringLiteral("groupingAppIdBlacklist"), QStringList());
    config->seed(QStringLiteral("groupingLauncherUrlBlacklist"), QStringList());
    config->seed(QStringLiteral("sortingStrategy"), 1);
    config->seed(QStringLiteral("separateLaunchers"), true);
    config->seed(QStringLiteral("hideLauncherOnStart"), true);
    config->seed(QStringLiteral("maxStripes"), 1);
    config->seed(QStringLiteral("forceStripes"), false);
    config->seed(QStringLiteral("showToolTips"), true);
    config->seed(QStringLiteral("taskMaxWidth"), 1);
    config->seed(QStringLiteral("wheelEnabled"), 0);
    config->seed(QStringLiteral("wheelSkipMinimized"), true);
    config->seed(QStringLiteral("highlightWindows"), false);
    // Same pinned-app order as /etc/skel/.config/plasma-org.kde.plasma.desktop-appletsrc.
    config->seed(QStringLiteral("launchers"), QStringList{
        QStringLiteral("preferred://filemanager"),
        QStringLiteral("preferred://browser"),
        QStringLiteral("applications:org.kde.kmail2.desktop"),
        QStringLiteral("applications:org.gnome.Maps.desktop"),
        QStringLiteral("applications:org.kde.gwenview.desktop"),
        QStringLiteral("applications:pearos-calendar.desktop"),
        QStringLiteral("applications:pearos-contacts.desktop"),
        QStringLiteral("applications:pearos-todo.desktop"),
        QStringLiteral("applications:pearos-notes.desktop"),
        QStringLiteral("applications:org.kde.elisa.desktop"),
        QStringLiteral("applications:pearos-appstore.desktop"),
        QStringLiteral("applications:org.kde.kdeconnect.app.desktop"),
        QStringLiteral("applications:pearos-systemsettings.desktop"),
    });
    // Files/folders dropped between the separator and Trash (see
    // main.qml's pinnedFilesRepeater) -- file:// URLs, persisted the same
    // way `launchers` is.
    config->seed(QStringLiteral("pinnedFiles"), QStringList{});
    config->seed(QStringLiteral("middleClickAction"), 2);
    config->seed(QStringLiteral("indicateAudioStreams"), true);
    config->seed(QStringLiteral("interactiveMute"), true);
    config->seed(QStringLiteral("tooltipControls"), true);
    // Always false: filer-dock's macOS-style dock must size to its own
    // content, never stretch to fill the anchored panel width -- see the
    // detailed rationale this replaces, in this file's git history.
    config->seed(QStringLiteral("fill"), false);
    config->seed(QStringLiteral("taskHoverEffect"), true);
    config->seed(QStringLiteral("maxTextLines"), 0);
    config->seed(QStringLiteral("minimizeActiveTaskOnClick"), true);
    config->seed(QStringLiteral("reverseMode"), false);
    config->seed(QStringLiteral("iconSpacing"), 1);
    config->seed(QStringLiteral("skinName"), QStringLiteral("Tahoe Dark"));
    config->seed(QStringLiteral("iconSize"), 50);
    config->seed(QStringLiteral("magnification"), 80.0);
    config->seed(QStringLiteral("amplitud"), 1.5);
    config->seed(QStringLiteral("showReflection"), false);
    config->seed(QStringLiteral("launcherIcon"), QStringLiteral("/usr/share/extras/launchpad.svg"));
    config->seed(QStringLiteral("skinBlur"), true);
    config->seed(QStringLiteral("skinBlurRadius"), 24);
    config->seed(QStringLiteral("skinLiquidGelEffect"), true);
    config->seed(QStringLiteral("skinRefractionStrength"), 8.0);
    config->seed(QStringLiteral("skinRgbFringing"), 2.0);
    config->seed(QStringLiteral("skinPositionTaskIndicator"), 3);
    config->seed(QStringLiteral("skinLeftMargin"), 20);
    config->seed(QStringLiteral("skinTopMargin"), 20);
    config->seed(QStringLiteral("skinRightMargin"), 20);
    config->seed(QStringLiteral("skinBottomMargin"), 20);
    config->seed(QStringLiteral("skinOutsideLeftMargin"), 20);
    config->seed(QStringLiteral("skinOutsideTopMargin"), 0);
    config->seed(QStringLiteral("skinOutsideRightMargin"), 20);
    config->seed(QStringLiteral("skinOutsideBottomMargin"), -10);

    // Dock visual-tuning knobs (main.qml's dockBackground/tmf) -- not part
    // of PearDock's original main.xml schema, so there's no upstream
    // default to mirror; these fallbacks only matter if Filer's
    // settings.conf is missing or its [Dock] group hasn't been written yet
    // (see watchSharedDockSettings(), called after this in main(), which
    // overrides them live from that file).
    config->seed(QStringLiteral("bottomGap"), 18);
    config->seed(QStringLiteral("bottomBreathingRoom"), 4);
    config->seed(QStringLiteral("horizontalPad"), 14);
    config->seed(QStringLiteral("verticalPad"), 11);
}

int main(int argc, char **argv)
{
    // Startup watchdog -- see filer-topbar/main.cpp's own copy of this for
    // the full write-up (same fix, same day, same underlying incident: a
    // baloo crash loop starving the machine at boot left both filer-dock
    // and filer-topbar stuck loading with no crash and no error, just an
    // empty unresponsive window forever). SIGALRM's default disposition
    // terminates the process; Application::startDock() in the parent Filer
    // process respawns on exactly that. Cancelled once we reach app.exec().
    alarm(20);

    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);
    QQuickWindow::setDefaultAlphaBuffer(true);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("filer-dock"));
    app.setOrganizationDomain(QStringLiteral("pearos.org"));

    // Single-instance guard: the session bus releases a registered service
    // name the moment the owning process dies for *any* reason, including
    // SIGKILL -- unlike a PID/lock file, which can be left stale behind a
    // force-killed process. That staleness is exactly what let duplicate
    // filer-dock instances pile up when Filer's own process (which normally
    // terminates this one cleanly via stopDock(), see application.cpp) was
    // itself force-killed instead of allowed to shut down normally.
    if(!QDBusConnection::sessionBus().registerService(QStringLiteral("org.pearos.FilerDock"))) {
        qWarning() << "filer-dock is already running -- exiting.";
        return 0;
    }

    // See the PlasmaCore.Dialog positioning note above: without this,
    // QtWayland resets every position set on an already-mapped toplevel to
    // the screen origin, so PearLauncher's post-show centering never reaches
    // the compositor, and our own syncQtWindowPosition() would be undone the
    // same way. Process-wide static; must be cleared before windows exist.
    //
    // It's a C++17 `static inline` class member, which leaves one copy in
    // this executable and another inside libQt6WaylandClient.so -- and the
    // library's own code reads *its* copy, not ours (verified with gdb on a
    // live process: assigning the member here flipped the executable's copy
    // only, exe=0/lib=1). So the assignment below is not enough on its own;
    // the copy the platform plugin actually consults has to be patched via
    // dlsym on the already-loaded library.
    if (QGuiApplication::platformName() == QStringLiteral("wayland")) {
        QtWaylandClient::QWaylandWindow::fixedToplevelPositions = false;

        const char *symbol = "_ZN15QtWaylandClient14QWaylandWindow22fixedToplevelPositionsE";
        if (void *lib = dlopen("libQt6WaylandClient.so.6", RTLD_NOLOAD | RTLD_LAZY)) {
            if (void *addr = dlsym(lib, symbol)) {
                *static_cast<bool *>(addr) = false;
            } else {
                qWarning() << "fixedToplevelPositions not found in libQt6WaylandClient -- "
                              "popup positioning (PearLauncher centering) will be broken.";
            }
            dlclose(lib); // undoes only the RTLD_NOLOAD ref; library stays loaded
        } else {
            qWarning() << "libQt6WaylandClient.so.6 not loaded despite wayland platform?";
        }
    }

    View v;

    // Stands in for the `Plasmoid` attached property that PlasmoidItem
    // normally provides when hosted by a real Plasma::Applet -- see
    // plasmoidshim.h for what it covers and why. Config values live only in
    // this one PlasmoidConfig instance now (persisted via QSettings), so
    // unlike the old KConfigGroup-vs-live-QML-property split, main.cpp and
    // main.qml are always looking at the exact same in-memory data -- no
    // snapshot-race workarounds needed.
    // Plasma::Types::BottomEdge / Plasma::Types::Horizontal (plain hardcoded
    // ints -- see filer-shell-common/CMakeLists.txt's header comment for
    // where these values come from).
    auto *corona = new PlasmoidCoronaShim(&v);
    auto *plasmoid = new PlasmoidShim(QStringLiteral("PearDock"), /*location=*/4, /*formFactor=*/2, corona, &v);
    seedPlasmoidConfigDefaults(plasmoid->configuration());
    // Live-syncs the dock's visual-tuning knobs from Filer's own
    // settings.conf ([Dock] group) -- editing that file (which Filer
    // itself already reloads live) updates the running dock too, no
    // restart needed. See PlasmoidConfig::watchSharedSettings().
    plasmoid->configuration()->watchSharedSettings(
        QDir::homePath() + QStringLiteral("/.config/filer/default/settings.conf"),
        QStringLiteral("Dock"));
    // "fill" (main.xml: "Whether task manager should occupy all available
    // space") drives PearDock's root item's Layout.fillWidth -- true is
    // right for a real taskbar-style panel spanning the screen edge, but
    // wrong for filer-dock's macOS-style dock: with it on, the icons stay
    // fit-to-content while the skin background stretches to the full
    // anchored panel width. Force it off regardless of any previously
    // persisted value.
    plasmoid->configuration()->insert(QStringLiteral("fill"), false);
    v.rootContext()->setContextProperty(QStringLiteral("Plasmoid"), plasmoid);

    v.setSource(QUrl::fromLocalFile(QStringLiteral(FILER_DOCK_PEARDOCK_PACKAGE_DIR "/contents/ui/main.qml")));

    new SearchAdaptor(v.rootObject());
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/pearos/FilerDock"), v.rootObject());

    v.setColor(Qt::transparent);
    v.setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);

    // Default is SizeViewToRootObject (the window follows the QML content's
    // own implicit size) -- main.qml's root Item has no content-driven
    // implicit size of its own, so the window ended up taking on some
    // unrelated large size instead of the size set below.
    v.setResizeMode(QQuickView::SizeRootObjectToView);

    // PearDock's own "Icon Size" setting (main.xml: "iconSize", default 50)
    // drives how tall the dock actually renders itself -- reading it here
    // instead of hardcoding a literal keeps the container (and the headroom
    // it gives PearDock's hover-magnification zoom, which in horizontal mode
    // only grows icons *width*-wise but still needs vertical room so the
    // bounce doesn't get clipped by the window's own top edge) in sync with
    // the real config. *2 preserves this codebase's original relationship
    // (default iconSize 50 -> dockHeight 100).
    //
    // The reserved strip (exclusiveZone) must match where the dock's visible
    // skin ("pill") actually starts, not the invisible container height
    // above -- reusing dockHeight+8 for both left a dead gap between a
    // maximized window and the real dock top edge. Reverse-engineered from
    // PearDock's own main.qml (customSkin's BorderImage anchors): the pill's
    // top inset within the container is `skinOutsideTopMargin + topoutimage
    // + (containerHeight - 76)`, where `topoutimage` is always `44 -
    // iconSize` (both branches of PearDock's if/else reduce to the same
    // expression). Reserving `containerHeight + 8 - thatInset` for the
    // exclusive zone makes the containerHeight term cancel out entirely,
    // leaving a constant that depends only on PearDock's own configured
    // iconSize and top margin. GAP_ABOVE_DOCK (in place of the
    // algebraically-exact 40, which still measured too generous live) is
    // intentionally cut down further, purely a fudge factor tuned by feel --
    // shrink it more if a gap persists, grow it if the window starts
    // overlapping the dock skin.
    struct DockMetrics { int height; int exclusiveZone; };
    auto computeDockMetrics = [config = plasmoid->configuration()]() -> DockMetrics {
        constexpr int GAP_ABOVE_DOCK = 28;
        const int iconSize = config->value(QStringLiteral("iconSize")).toInt();
        const int skinOutsideTopMargin = config->value(QStringLiteral("skinOutsideTopMargin")).toInt();
        DockMetrics m;
        m.height = iconSize * 2;
        m.exclusiveZone = std::max(iconSize, GAP_ABOVE_DOCK + iconSize - skinOutsideTopMargin);
        return m;
    };

    DockMetrics metrics = computeDockMetrics();
    // Initial Qt-side surface size (SizeRootObjectToView needs *something*
    // to seed content with before show()); the layer-shell configure event
    // that follows corrects the actual width to match the anchored span.
    v.resize(QGuiApplication::primaryScreen()->geometry().width(), metrics.height);

    // Plain QWindow::setGeometry()/move() only *suggests* a position on
    // Wayland -- regular toplevel surfaces are positioned entirely by the
    // compositor's placement policy, which is why the dock kept showing up
    // wherever KWin's default placement put it instead of at the bottom.
    // wlr-layer-shell (via LayerShellQt) is the protocol actual panels/bars
    // use to get real compositor-anchored placement. Must be set up before
    // show() -- that's when the underlying layer surface is created.
    //
    // Anchoring only to the bottom edge (letting the compositor center a
    // narrower surface) measured ~146px off-center and didn't respond to
    // setDesiredSize() -- some KWin quirk with implicit centering of a
    // partially-anchored layer surface. Anchoring all three of
    // bottom+left+right instead stretches the surface to the full screen
    // width (the standard, reliable way real panels/bars position
    // themselves), and PearDock's own QML already self-centers its task
    // icons within whatever width it's given (see `centerOffset` in
    // main.qml) -- exactly what it does when a real Plasma panel spans the
    // full screen edge, which is the environment it was actually designed
    // for.
    if(auto *layerWindow = LayerShellQt::Window::get(&v)) {
        layerWindow->setScope(QStringLiteral("dock"));
        layerWindow->setLayer(LayerShellQt::Window::LayerTop);
        layerWindow->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorBottom)
                                 | LayerShellQt::Window::AnchorLeft
                                 | LayerShellQt::Window::AnchorRight);
        // Reserve real screen space at the bottom (like a real panel/dock)
        // so maximized windows resize to avoid it instead of running full
        // height underneath it -- exclusiveZone(0) (floating, unreserved)
        // was the wrong default for a dock users expect to behave like an
        // actual panel.
        layerWindow->setExclusiveZone(metrics.exclusiveZone);
        layerWindow->setMargins(QMargins(0, 0, 0, 6)); // small gap above the screen edge, macOS-dock style
        layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        layerWindow->setDesiredSize(QSize(0, metrics.height)); // width: 0 = fill the anchored (left+right) span
        // Keep QWindow's idea of our position matching the layer-shell
        // anchoring (bottom edge, 8px margin) -- popups position themselves
        // relative to it. Re-synced on every resize (view.cpp) and here on
        // screen geometry changes (resolution switch etc.).
        v.setBottomMargin(6);
        QObject::connect(QGuiApplication::primaryScreen(), &QScreen::geometryChanged,
                         &v, [&v] { v.syncQtWindowPosition(); });

        // PearDock's settings dialog writes iconSize/margin changes straight
        // into this same PlasmoidConfig instance -- main.cpp and main.qml
        // share it, so a plain signal connection (no polling needed, unlike
        // the old KConfigGroup-backed version) is enough to keep the
        // reserved zone in sync without requiring a filer-dock restart every
        // time the user tweaks the dock's icon size.
        QObject::connect(plasmoid->configuration(), &QQmlPropertyMap::valueChanged,
                         &v, [&v, layerWindow, computeDockMetrics](const QString &key, const QVariant &) {
            if (key != QStringLiteral("iconSize") && key != QStringLiteral("skinOutsideTopMargin")) {
                return;
            }
            const DockMetrics fresh = computeDockMetrics();
            v.resize(QGuiApplication::primaryScreen()->geometry().width(), fresh.height);
            layerWindow->setDesiredSize(QSize(0, fresh.height));
            layerWindow->setExclusiveZone(fresh.exclusiveZone);
            v.syncQtWindowPosition();
        });
    } else {
        qWarning() << "LayerShellQt unavailable -- dock will use whatever position KWin's default placement picks.";
    }

    v.show();

    // Made it -- cancel the startup watchdog armed at the top of main().
    alarm(0);

    return app.exec();
}

#include "main.moc"
