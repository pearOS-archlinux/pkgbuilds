#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "PW::KLookAndFeel" for configuration "Release"
set_property(TARGET PW::KLookAndFeel APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(PW::KLookAndFeel PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Qt6::DBus;Qt6::Widgets;KF6::ColorScheme;KF6::GuiAddons;KF6::I18n;KF6::IconThemes;KF6::KIOGui;PW::krdb;KF6::WindowSystem"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libklookandfeel.so.6.7.3"
  IMPORTED_SONAME_RELEASE "libklookandfeel.so.6"
  )

list(APPEND _cmake_import_check_targets PW::KLookAndFeel )
list(APPEND _cmake_import_check_files_for_PW::KLookAndFeel "${_IMPORT_PREFIX}/lib/libklookandfeel.so.6.7.3" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
