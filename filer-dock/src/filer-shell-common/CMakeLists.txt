cmake_minimum_required(VERSION 3.16)
project(filer-shell-common)

# Shared "Plasmoid" hosting shim used by both filer-dock (PearDock,
# bottom-edge) and filer-topbar (top-bar plasmoids, top-edge) to host Plasma
# plasmoid QML directly from a plain QQuickView/QWindow, with zero libplasma
# C++ (Plasma::Applet/Containment/Corona/PluginLoader) linked in. See
# plasmoidshim.h's own header comment for the full rationale.
#
# The Plasma::Types enum values used in main.cpp call sites (PlasmoidShim's
# location/formFactor constructor args) are plain hardcoded ints, sourced
# from /usr/include/Plasma/plasma/plasma.h and /usr/include/Plasma/plasma/applet.h
# -- this library does not link Plasma::Plasma or include any libplasma header
# at all.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "6.4.0")
set(KF6_MIN_VERSION "6.0.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMQmlModule)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS GlobalAccel WindowSystem)

# SHARED, not STATIC: this library's PlasmoidShim is also linked into
# filer_shell_common_qml (the QML plugin .so PlasmoidAttachedProvider's
# attached-properties lookup needs, see plasmoidattachroot.cpp) as well as
# directly into the filer-dock/filer-topbar executables. A *static* lib
# linked into both a shared module and an executable gets a separate copy of
# PlasmoidShim's staticMetaObject in each final binary -- qobject_cast<>
# compares QMetaObject pointer identity, not RTTI, so a PlasmoidShim* created
# in the executable silently failed to cast back to PlasmoidShim* when
# checked from code running inside the .so (surfaced as attached-properties
# lookups mysteriously returning null for some plasmoids but not others,
# depending on link order). One shared object ensures one copy of the
# metaobject the whole process agrees on.
add_library(filer_shell_common SHARED
    plasmoidshim.h
    plasmoidshim.cpp
)

target_include_directories(filer_shell_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(filer_shell_common PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    KF6::GlobalAccel
)

# QML-side pieces (PlasmoidHost.qml today; ShellDialog/tooltipshim.qml join
# later) registered as one importable module, "org.pearos.shellshim", so
# every hosted plasmoid needs only one new import line.
ecm_add_qml_module(filer_shell_common_qml
    URI "org.pearos.shellshim"
    VERSION 1.0
    GENERATE_PLUGIN_SOURCE
    SOURCES
        plasmoidattachroot.h
        plasmoidattachroot.cpp
        dialogshim.h
        dialogshim.cpp
        plasmatypesshim.h
    QML_FILES
        qml/PlasmoidHost.qml
        qml/ShellToolTipArea.qml
        qml/HighlightShim.qml
        qml/ExpandableListItemShim.qml
)
target_link_libraries(filer_shell_common_qml PRIVATE filer_shell_common KF6::WindowSystem)
ecm_finalize_qml_module(filer_shell_common_qml)
