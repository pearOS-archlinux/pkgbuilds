# filer-dock

Hosts the `pearos-dock` Plasma applet (`../pearos-dock-src`, vendored unmodified
from `/home/alxb421/Desktop/pkgbuilds/pearos-dock` — do not edit that source)
standalone, outside `plasmashell`, as its own always-on-top panel window.

## Why a separate process

Filer is a Qt5 application. `pearos-dock` requires Qt6 + KDE Frameworks 6 +
Plasma 6 (`libplasma`, `libtaskmanager`) — Qt5 and Qt6 cannot be linked into
one executable. `filer-dock` is therefore a standalone Qt6/KF6 executable,
spawned as a child process by `Filer` (see `Application::startDock()` /
`stopDock()` in `../src/application.cpp`, called from `desktopManager()`)
rather than embedded in Filer's own window.

## How it hosts a Plasmoid without plasmashell

`PlasmoidItem`'s `Plasmoid` attached property (and the rest of the
`org.kde.plasma.plasmoid` API surface `main.qml` uses) only resolves inside
a real `Plasma::Applet` hosted by a `Plasma::Containment`/`Plasma::Corona` —
plasmashell normally provides all three. `view.h`/`view.cpp` here are
adapted from KDE's own `plasmoidviewer` tool (`plasma-sdk`, GPL-2.0-or-later,
see SPDX headers) which exists for exactly this: running one Plasma applet
in its own window for development/testing, with a minimal `Corona` +
`ContainmentView` standing in for plasmashell. The only change from upstream
is the shell package id (`com.pearos.filerdockshell`, see below) and
`main.cpp`, which is rewritten to hardcode the PearDock applet / horizontal
bottom-edge panel instead of taking generic `--applet`/`--containment` CLI
flags.

`vendor/qmlpackages/shell/` is a "Plasma/Shell" KPackage (also adapted from
plasma-sdk's plasmoidviewer shell package) that `ContainmentView` loads to
get its `views/Desktop.qml` — adapted here to be transparent with no SDK dev
toolbar, since filer-dock is a real panel, not a testing tool.

## Building

Three independent pieces, in this order:

1. **The PearDock QML plugin** (`import PearDock` in main.qml — the C++
   backend in `../pearos-dock-src/plugin`), built from pearos-dock-src's own
   *unmodified* top-level `CMakeLists.txt`:

   ```sh
   cd ../pearos-dock-src
   mkdir -p build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
   make -j$(nproc)
   make install
   ```

   This installs the QML plugin to `~/.local/lib/qml/PearDock/` (no `sudo`
   needed — user-local prefix).

2. **Vendored plasma-workspace QML modules** (`../plasma-workspace-components-src`)
   — pulled unmodified from upstream plasma-workspace (only the top-level
   `CMakeLists.txt` there is ours; every module subdirectory is an untouched
   copy), so main.qml's imports of these resolve without plasma-workspace
   installed on the system. Same install prefix as PearDock, so it lands on
   the same `QML_IMPORT_PATH` entry already set up below:

   ```sh
   cd ../plasma-workspace-components-src
   mkdir -p build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local -DBUILD_TESTING=OFF
   make -j$(nproc)
   make install
   ```

   Vendored so far: `org.kde.plasma.workspace.trianglemousefilter`,
   `org.kde.plasma.workspace.dbus` (both pure Qt, no KF6 dependency),
   `org.kde.plasma.private.mpris` (`libkmpris` — needs `KF6::Config` +
   `KF6::I18n`; its `kded/` OSD-notification helper and autotests were left
   out, not needed for the multiplexer/model classes PearDock's QML actually
   uses), and `org.kde.taskmanager` (`libtaskmanager` — needs
   `KF6::{ItemModels,KIO,Service,WindowSystem}` + `PlasmaActivities`, all
   plain Frameworks/packages already installed here, nothing
   plasma-workspace-specific). Two notes on the taskmanager vendoring:
   - `PW::KWorkspace` was dropped from its link libraries entirely (verified
     unused by grepping for any of its headers — a stray upstream link dep),
     *except* `defaultservice.cpp/.h` (browser-default lookup, used by
     `tasktools.cpp`), which is vendored directly into `libtaskmanager/`
     alongside a trivial local `kworkspace_export.h` stand-in (no separate
     shared library, so no export annotation needed).
   - The four Wayland protocol XML files it needs
     (`org-kde-plasma-virtual-desktop.xml`, `plasma-window-management.xml`,
     `zkde-screencast-unstable-v1.xml`, `kde-output-order-v1.xml`) are
     vendored unmodified in `wayland-protocols/` (from
     `plasma-wayland-protocols`, BSD-3, protocol specs only, not code) instead
     of depending on that system package.
   - **ABI parity matters here**: `libtaskmanager.so.6` is the one vendored
     library with real system-side reverse dependents (`libnotificationmanager.so.1`,
     and a couple of `plasma/applets/*.so` plugins all link against it by
     soname) — unlike the other three modules, which nothing else on the
     system links against. The vendored source must therefore be built from
     the exact same upstream tag as the installed `plasma-workspace` package
     (`v6.7.3` here, not `master`/HEAD) and with `HAVE_X11 1` in
     `config-X11.h` (the system package was built with X11 support compiled
     in — confirmed with `nm -D`, which is where `XWindowTasksModel`'s and
     `TasksModel::requestToggleShaded`'s symbols come from). Building against
     HEAD, or with X11 support left out, silently produces a `libtaskmanager.so.6`
     missing symbols the system's `libnotificationmanager.so.1` expects at
     that soname — which only surfaces at runtime as
     `Cannot load library .../libwavetaskplugin.so: ...: undefined symbol:
     ...requestToggleShaded...` when PearDock's own plugin pulls both in.

   Also vendored: **`org.kde.plasma.private.kicker`** (`kicker/` — the
   launcher applet's model backend), the heaviest of the four since it needs
   `KF6::Runner` (KRunner client, for search-as-you-type), plus
   `IconThemes`/`Notifications`/`Auth`/`KIOWidgets`/`KIOFileWidgets`,
   `Plasma::{Plasma,PlasmaQuick,Activities,ActivitiesStats}`, and
   `ScreenSaverDBusInterface` (from the separately-installed `kscreenlocker`
   package, not plasma-workspace) — all plain Frameworks/packages, nothing
   plasma-workspace-specific once traced through. Notes specific to kicker:
   - `systementry.cpp`'s power-menu entries (lock/logout/suspend/hibernate/
     reboot/shutdown) actually **do** use libkworkspace's `SessionManagement`
     (unlike libtaskmanager, where `PW::KWorkspace` turned out unused) — so
     `sessionmanagement.cpp/.h`, `sessionmanagementbackend.cpp/.h`, and the
     pre-generated `login1_manager_interface.cpp/.h` (talks to logind
     directly over DBus, no ksmserver needed) are vendored in alongside a
     `kworkspace_export.h` stand-in and a small local
     `libkworkspace_debug.{h,cpp}` (manually declaring the one logging
     category `sessionmanagement.cpp` uses, in place of upstream's
     `ecm_qt_declare_logging_category()` call).
   - Its DBus interfaces (`org.kde.krunner.App.xml`,
     `org.kde.LogoutPrompt.xml`, `org.kde.Shutdown.xml`) are vendored as
     static XML (`krunner-dbus/`, `kicker/`); `org.freedesktop.ScreenSaver`
     resolves via the system's `ScreenSaverDBusInterface` CMake package
     (installed by `kscreenlocker`, a real dependency, not something being
     removed). `ksmserver_interface` (upstream links this too) was dropped —
     verified unused by any vendored file.
   - `plasma-shell.xml` (the one additional Wayland protocol kicker needs)
     is vendored the same way as libtaskmanager's four.
   - Three more upstream generated headers turned out to be either unused
     or optional-feature gates, so they're stubbed the same way as
     `config-X11.h`: `config-appstream.h` (`HAVE_APPSTREAMQT`, AppStream
     metadata isn't linked), `config-workspace.h` (`HAVE_ICU`, ICU isn't
     linked), `config-KDECI_BUILD.h` (its only real check in
     `kastatsfavoritesmodel.cpp` reads the `KDECI_BUILD` *environment
     variable* at runtime, not this header's macro).

   With all four vendored, filer-dock's QML imports no longer depend on
   plasma-workspace being installed on the system at all — everything left
   (`KF6::*`, `Plasma::*`, `kscreenlocker`) is a plain Frameworks/library
   dependency, not the `plasma-workspace` package itself.

3. **filer-dock itself**:

   ```sh
   mkdir -p build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

## Running

The shell package must be installed where KPackage can find it by id
(again, no `sudo` -- a user-local Plasma package location):

```sh
mkdir -p ~/.local/share/plasma/shells/com.pearos.filerdockshell
cp -r vendor/qmlpackages/shell/* ~/.local/share/plasma/shells/com.pearos.filerdockshell/
```

Then either run directly (`QML_IMPORT_PATH` must include the PearDock
plugin from step 1 above):

```sh
QML_IMPORT_PATH=$HOME/.local/lib/qml ./build/bin/filer-dock
```

or just start `Filer --desktop` — `Application::startDock()` sets
`QML_IMPORT_PATH` itself and launches `filer-dock` automatically (looking
first next to the `Filer` binary, then falling back to
`../../filer-dock/build/bin/filer-dock` for running straight from a dev
build tree).

## Known non-fatal startup noise

A handful of QML warnings print on every launch and are harmless (binding
evaluation order at startup, self-resolving once the containment's geometry
settles): `Cannot read property 'width'/'height' of null`,
`isScreenUiReady is not a function` (from the desktopcontainment plugin's
own QML, unrelated to PearDock), and `PlasmaWindowManagement protocol
hasn't activated in time` (KWin's foreign-toplevel/window-management
Wayland protocol takes a moment to hand over control; the taskmanager still
populates a moment later). These come from pearos-dock-src's own QML and
were not modified.
