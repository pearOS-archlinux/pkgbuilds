cmake_minimum_required(VERSION 3.16)

project(pear-calendar VERSION 1.0.0 LANGUAGES CXX)

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

# Optional: KDE blur behind (KWin)
find_package(KF6 COMPONENTS WindowSystem QUIET)
if(TARGET KF6::WindowSystem)
  set(HAVE_KWINDOW_EFFECTS 1)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

# Source files
set(SOURCES
    src/main.cpp

    # GUI
    src/gui/mainwindow.cpp
)

# Header files
set(HEADERS
    src/utils/logger.h
    src/utils/types.h
    
    # GUI
    src/gui/mainwindow.h
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    assets/sidebar_icons.qrc
)

# Link libraries
set(LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Concurrent)
if(HAVE_KWINDOW_EFFECTS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_KWINDOW_EFFECTS)
  list(APPEND LIBS KF6::WindowSystem)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

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
install(FILES assets/pearos-calendar.desktop
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
    DESTINATION share/pearos-calendar
)

# Create data directory for user-writable events (no files, only directory with permissive perms)
install(DIRECTORY DESTINATION share/extras/pearos-calendar
    DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_WRITE WORLD_EXECUTE
)

# Arch package helper target (run from build dir: `make pkg`)
add_custom_target(pkg
    COMMAND makepkg -f -p PKGBUILD
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
