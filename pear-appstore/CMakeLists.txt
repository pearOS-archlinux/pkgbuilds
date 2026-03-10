cmake_minimum_required(VERSION 3.16)

project(pearos-appstore VERSION 0.2.28 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Concurrent
)

# Find libalpm
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALPM REQUIRED libalpm)

# Optional: KDE blur behind (KWin)
find_package(KF6 COMPONENTS WindowSystem QUIET)
if(TARGET KF6::WindowSystem)
  set(HAVE_KWINDOW_EFFECTS 1)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${ALPM_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/utils/cache_helper.cpp
    src/utils/app_cache.cpp
    # Core
    src/core/alpm_wrapper.cpp
    src/core/aur_helper.cpp
    src/core/appstream_helper.cpp
    src/core/package_manager.cpp
    
    # GUI
    src/gui/mainwindow.cpp
    src/gui/home_widget.cpp
    src/gui/search_widget.cpp
    src/gui/installed_widget.cpp
    src/gui/updates_widget.cpp
    src/gui/user_page_widget.cpp
    src/gui/settings_widget.cpp
    src/gui/package_card.cpp
    src/gui/package_details_dialog.cpp
    src/gui/loading_overlay.cpp
    src/gui/scroll_bar_fade.cpp
)

# Header files
set(HEADERS
    src/utils/logger.h
    src/utils/types.h
    src/utils/cache_helper.h
    src/utils/app_cache.h
    
    # Core
    src/core/alpm_wrapper.h
    src/core/aur_helper.h
    src/core/appstream_helper.h
    src/core/package_manager.h
    
    # GUI
    src/gui/mainwindow.h
    src/gui/home_widget.h
    src/gui/search_widget.h
    src/gui/installed_widget.h
    src/gui/updates_widget.h
    src/gui/user_page_widget.h
    src/gui/settings_widget.h
    src/gui/package_card.h
    src/gui/package_details_dialog.h
    src/gui/loading_overlay.h
    src/gui/scroll_bar_fade.h
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    assets/sidebar_icons.qrc
)

# Link libraries
set(LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Concurrent ${ALPM_LIBRARIES})
if(HAVE_KWINDOW_EFFECTS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_KWINDOW_EFFECTS)
  list(APPEND LIBS KF6::WindowSystem)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

# Link directories
link_directories(${ALPM_LIBRARY_DIRS})

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install desktop file
install(FILES assets/pearos-appstore.desktop
    DESTINATION share/applications
)

# Copy stylesheet to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/stylesheet.qss
    ${CMAKE_BINARY_DIR}/stylesheet.qss
    COPYONLY
)

# Install stylesheet
install(FILES stylesheet.qss
    DESTINATION share/pearos-appstore
)

# Arch package helper target (run from build dir: `make pkg`)
add_custom_target(pkg
    COMMAND makepkg -f -p PKGBUILD
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
