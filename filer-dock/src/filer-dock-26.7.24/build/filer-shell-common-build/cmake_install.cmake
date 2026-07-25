# Install script for directory: /home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-shell-common

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/libfiler_shell_common_qml.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/libfiler_shell_common_qml.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/libfiler_shell_common_qml.so"
         RPATH "\$ORIGIN/../../../../lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim" TYPE MODULE FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-dock-26.7.24/build/bin/org/pearos/shellshim/libfiler_shell_common_qml.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/libfiler_shell_common_qml.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/libfiler_shell_common_qml.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/libfiler_shell_common_qml.so"
         OLD_RPATH "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-dock-26.7.24/build/bin:"
         NEW_RPATH "\$ORIGIN/../../../../lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/libfiler_shell_common_qml.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim" TYPE FILE FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-dock-26.7.24/build/bin/org/pearos/shellshim/qmldir")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim" TYPE FILE FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-dock-26.7.24/build/bin/org/pearos/shellshim/filer_shell_common_qml.qmltypes")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/qml" TYPE FILE RENAME "PlasmoidHost.qml" FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-shell-common/qml/PlasmoidHost.qml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/qml" TYPE FILE RENAME "ShellToolTipArea.qml" FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-shell-common/qml/ShellToolTipArea.qml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/qml" TYPE FILE RENAME "HighlightShim.qml" FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-shell-common/qml/HighlightShim.qml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim/qml" TYPE FILE RENAME "ExpandableListItemShim.qml" FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-shell-common/qml/ExpandableListItemShim.qml")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "filer-shell-common" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/qml/org/pearos/shellshim" TYPE FILE RENAME "kde-qmlmodule.version" FILES "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-dock-26.7.24/build/filer-shell-common-build/filer_shell_common_qml-kde-qmlmodule.version")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/alxb421/Desktop/pkgbuilds/filer-dock/src/filer-dock-26.7.24/build/filer-shell-common-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
