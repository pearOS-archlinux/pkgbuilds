cmake_minimum_required(VERSION 3.16)
project(filer-dock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "6.4.0")
set(KF6_MIN_VERSION "6.0.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
# NOTE: deliberately not including KDECompilerSettings -- it adds
# -DQT_NO_CAST_FROM_ASCII, which the vendored view.cpp (from plasma-sdk's
# plasmoidviewer, unmodified aside from the shell package id) doesn't wrap
# its string literals for.

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Widgets Qml Quick DBus WaylandClient WaylandClientPrivate)
find_package(LayerShellQt REQUIRED)

# The Plasmoid hosting shim (was plasmoidshim.h/.cpp here) now lives in
# ../filer-shell-common, shared with filer-topbar -- no libplasma
# (Plasma::Applet/Containment/Corona/PluginLoader, nor even Plasma::Types) is
# linked into filer-dock anymore. The actual task-list/skin/MPRIS QML modules
# PearDock's QML imports (org.kde.taskmanager, org.kde.ksvg, ...) are loaded
# by the QML engine at runtime as separate plugins and don't need to be
# linked here either.
add_subdirectory(../filer-shell-common filer-shell-common-build)

add_executable(filer-dock
    main.cpp
    view.h
    view.cpp
)

target_link_libraries(filer-dock PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::DBus
    # Private API, needed only for QWaylandWindow::fixedToplevelPositions
    # (see main.cpp) -- version-locked to the installed qtwayland.
    Qt6::WaylandClientPrivate
    filer_shell_common
    LayerShellQt::Interface
)

# Defaults to the dev-tree sibling checkout so the workflow in README.md's
# "Building" section keeps working unmodified. Packaging (see
# pkgbuilds/filer-dock/PKGBUILD) overrides this to the *installed* location
# of PearDock's KPackage content -- pearos-dock's own CMakeLists.txt installs
# that via plasma_install_package(), which lands at
# ${KDE_INSTALL_DATADIR}/plasma/plasmoids/PearDock system-wide.
set(FILER_DOCK_PEARDOCK_PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../pearos-dock-src/package" CACHE PATH
    "Path to PearDock's KPackage content (contents/ui/main.qml etc)")

target_compile_definitions(filer-dock PRIVATE
    FILER_DOCK_PEARDOCK_PACKAGE_DIR="${FILER_DOCK_PEARDOCK_PACKAGE_DIR}"
)

# The PearDock QML plugin (org.kde.plasma.plasmoid's "import PearDock" C++
# backend) is built separately from ../pearos-dock-src's own unmodified
# CMakeLists.txt/plugin -- see README.md in this directory for the dev-tree
# workflow (installs to a user-local prefix, found via QML_IMPORT_PATH). A
# packaged build installs both PearDock's plugin and plasma-workspace's
# vendored QML modules (../plasma-workspace-components-src) to the standard
# system Qt QML path (CMAKE_INSTALL_PREFIX=/usr), so Qt finds them with no
# QML_IMPORT_PATH needed at all -- filer-dock itself does not build or link
# either of them.

install(TARGETS filer-dock DESTINATION ${KDE_INSTALL_BINDIR})

# The "Plasma/Shell" KPackage ContainmentView loads for its views/Desktop.qml
# (see README.md's "How it hosts a Plasmoid without plasmashell"). Dev builds
# still need the manual copy in README.md's "Running" section; this only
# covers the packaged case.
install(DIRECTORY vendor/qmlpackages/shell/
    DESTINATION ${KDE_INSTALL_DATADIR}/plasma/shells/com.pearos.filerdockshell
)
