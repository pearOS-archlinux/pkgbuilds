#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "PW::LibTaskManager" for configuration "Release"
set_property(TARGET PW::LibTaskManager APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(PW::LibTaskManager PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Qt6::Qml;Qt6::Concurrent;Qt6::DBus;Qt6::WaylandClient;Plasma::Activities;KF6::ConfigCore;KF6::I18n;KF6::KIOCore;KF6::KIOGui;KF6::JobWidgets;KF6::Service;KF6::WindowSystem;PW::KWorkspace;KF6::IconThemes"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libtaskmanager.so.6.7.3"
  IMPORTED_SONAME_RELEASE "libtaskmanager.so.6"
  )

list(APPEND _cmake_import_check_targets PW::LibTaskManager )
list(APPEND _cmake_import_check_files_for_PW::LibTaskManager "${_IMPORT_PREFIX}/lib/libtaskmanager.so.6.7.3" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
