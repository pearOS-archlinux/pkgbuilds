cmake_minimum_required(VERSION 3.16)
project(pearos-welcome CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Widgets + KWindowSystem (for real KWin blur-behind, see
# updateSidebarBlurRegion() in mainwindow.cpp) -- no libfm-qt, no glib/gio,
# no Filer library dependency at all. See mainwindow.cpp/.h for what's
# reproduced from Filer's real MainWindow (full chrome machinery) vs.
# deliberately left out (everything else).
find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets DBus Network)
find_package(KF5WindowSystem REQUIRED)

add_executable(pearos-welcome
    main.cpp
    mainwindow.cpp
    welcome.cpp
    filer_titlebar.cpp
    titlebar.cpp
    titlebar_buttons.cpp
    filer_sidebar.cpp
)
target_link_libraries(pearos-welcome PRIVATE Qt5::Widgets Qt5::DBus Qt5::Network KF5::WindowSystem)

# Wayland-side counterpart to MainWindow::updateSidebarWallpaperTint()'s
# X11 frameGeometry() path -- see main.cpp's WindowGeometryRelay/
# loadWindowTrackerKwinScript(). Copied next to the built executable so it's
# found there in dev builds; a real install would also want this under
# CMAKE_INSTALL_PREFIX.
set(KWIN_SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kwin-scripts")
if(EXISTS "${KWIN_SCRIPTS_DIR}")
  add_custom_command(TARGET pearos-welcome POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${KWIN_SCRIPTS_DIR}" "$<TARGET_FILE_DIR:pearos-welcome>/kwin-scripts"
  )
endif()

# Traffic-light PNGs live in assets/titlebuttons/ -- copy that directory
# next to the built executable so assetsBasePath_ (mainwindow.cpp) finds it
# whether run from build/ or anywhere else.
set(SHARED_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(EXISTS "${SHARED_ASSETS_DIR}")
  add_custom_command(TARGET pearos-welcome POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHARED_ASSETS_DIR}" "$<TARGET_FILE_DIR:pearos-welcome>/assets"
  )
endif()

# pearOS Welcome's real stylesheet -- see MainWindow::MainWindow()'s
# loadStylesheet block in mainwindow.cpp. Copied next to the built
# executable, same reasoning as SHARED_ASSETS_DIR above.
set(SHARED_STYLES_QSS "${CMAKE_CURRENT_SOURCE_DIR}/styles.qss")
if(EXISTS "${SHARED_STYLES_QSS}")
  add_custom_command(TARGET pearos-welcome POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SHARED_STYLES_QSS}" "$<TARGET_FILE_DIR:pearos-welcome>/styles.qss"
  )
endif()

# updater.sh -- see update_system_prep_and_upgrade_cmd() in welcome.cpp,
# reads it relative to get_base_path().
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/updater.sh")
  add_custom_command(TARGET pearos-welcome POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/updater.sh" "$<TARGET_FILE_DIR:pearos-welcome>/updater.sh"
  )
endif()
