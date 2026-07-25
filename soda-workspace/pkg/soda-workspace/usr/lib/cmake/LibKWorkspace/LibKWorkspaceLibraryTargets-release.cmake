#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "PW::KWorkspace" for configuration "Release"
set_property(TARGET PW::KWorkspace APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(PW::KWorkspace PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Qt6::DBus;Qt6::WaylandClient;KF6::I18n;KF6::WindowSystem;KF6::ConfigCore;KF6::Service;KF6::Package;Plasma::Plasma"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libkworkspace6.so.6.7.3"
  IMPORTED_SONAME_RELEASE "libkworkspace6.so.6"
  )

list(APPEND _cmake_import_check_targets PW::KWorkspace )
list(APPEND _cmake_import_check_files_for_PW::KWorkspace "${_IMPORT_PREFIX}/lib/libkworkspace6.so.6.7.3" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
