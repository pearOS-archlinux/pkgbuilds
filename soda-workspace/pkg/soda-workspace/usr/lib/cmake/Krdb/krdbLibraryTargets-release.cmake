#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "PW::krdb" for configuration "Release"
set_property(TARGET PW::krdb APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(PW::krdb PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Qt6::Widgets;Qt6::DBus;KF6::CoreAddons;KF6::DBusAddons;KF6::GuiAddons;KF6::I18n;KF6::WindowSystem;KF6::ColorScheme"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libkrdb.so"
  IMPORTED_SONAME_RELEASE "libkrdb.so"
  )

list(APPEND _cmake_import_check_targets PW::krdb )
list(APPEND _cmake_import_check_files_for_PW::krdb "${_IMPORT_PREFIX}/lib/libkrdb.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
