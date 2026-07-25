#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "PW::LibNotificationManager" for configuration "Release"
set_property(TARGET PW::LibNotificationManager APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(PW::LibNotificationManager PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Qt6::DBus;KF6::ConfigGui;KF6::I18n;KF6::WindowSystem;KF6::ItemModels;KF6::Notifications;KF6::KIOFileWidgets;Plasma::Plasma;PW::LibTaskManager;KF6::Screen;KF6::Service;Qt6::Qml"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libnotificationmanager.so.6.7.3"
  IMPORTED_SONAME_RELEASE "libnotificationmanager.so.1"
  )

list(APPEND _cmake_import_check_targets PW::LibNotificationManager )
list(APPEND _cmake_import_check_files_for_PW::LibNotificationManager "${_IMPORT_PREFIX}/lib/libnotificationmanager.so.6.7.3" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
