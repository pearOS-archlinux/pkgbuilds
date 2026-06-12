cmake_minimum_required(VERSION 3.16)
project(pearos-notch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Svg Network)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED xcb)

option(WITH_LAYER_SHELL "Build with LayerShellQt support for Wayland" ON)

if(WITH_LAYER_SHELL)
    find_package(LayerShellQt QUIET PATHS /usr/lib/cmake/LayerShellQt)
    if(LayerShellQt_FOUND)
        message(STATUS "LayerShellQt found — Wayland layer shell support enabled")
        set(HAS_LAYER_SHELL TRUE)
    else()
        message(WARNING "LayerShellQt not found — Wayland layer shell support disabled")
        set(HAS_LAYER_SHELL FALSE)
    endif()
else()
    set(HAS_LAYER_SHELL FALSE)
endif()

qt_add_executable(pearos-notch
    src/main.cpp
    src/Config.h
    src/Config.cpp
    src/NotchWindow.h
    src/NotchWindow.cpp
    src/MprisWatcher.h
    src/MprisWatcher.cpp
    src/SettingsDialog.h
    src/SettingsDialog.cpp
    src/resources.qrc
)

target_include_directories(pearos-notch PRIVATE src ${XCB_INCLUDE_DIRS})

target_link_libraries(pearos-notch PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Svg
    Qt6::Network
    ${XCB_LIBRARIES}
)

if(HAS_LAYER_SHELL)
    target_compile_definitions(pearos-notch PRIVATE WITH_LAYER_SHELL)
    target_link_libraries(pearos-notch PRIVATE LayerShellQt::Interface)
endif()

include(GNUInstallDirs)
install(TARGETS pearos-notch DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
    notch.svg prev.svg next.svg play.svg pause.svg settings.svg
    home.svg tray_icon.svg browser.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pearos-notch
)
install(FILES pearos-notch.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES pearos-notch.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)
