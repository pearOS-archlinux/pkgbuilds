# Install script for directory: /home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/lib/Filer.app")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/Filer.app/Filer")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/lib/Filer.app" TYPE EXECUTABLE FILES "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/Filer")
  if(EXISTS "$ENV{DESTDIR}/usr/lib/Filer.app/Filer" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/Filer.app/Filer")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/lib/Filer.app/Filer")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/Filer.app/Resources/translations/filer-qt.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_cs_CZ.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_da.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_de.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_el.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_eo.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_es.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_fr.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_he.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_hu.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_it.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_ja.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_ka.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_ko.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_lt.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_nb_NO.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_nl.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_pl.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_pt.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_pt_BR.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_ru.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_sl.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_sv.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_tr.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_uk.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_zh_Hans.qm;/usr/lib/Filer.app/Resources/translations/filer-qt_zh_TW.qm")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/lib/Filer.app/Resources/translations" TYPE FILE FILES
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_cs_CZ.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_da.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_de.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_el.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_eo.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_es.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_fr.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_he.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_hu.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_it.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_ja.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_ka.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_ko.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_lt.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_nb_NO.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_nl.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_pl.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_pt.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_pt_BR.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_ru.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_sl.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_sv.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_tr.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_uk.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_zh_Hans.qm"
    "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/filer-qt_zh_TW.qm"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/Filer.app/Resources/kwin-scripts/filer-window-tracker.js")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/lib/Filer.app/Resources/kwin-scripts" TYPE FILE FILES "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/src/kwin-scripts/filer-window-tracker.js")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/alxb421/Desktop/pkgbuilds/soda-filer/src/soda-filer-26.7.27/build/src/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
