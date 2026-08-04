kcoreaddons_add_plugin(kcm_pearos SOURCES kcm_pearos.cpp INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}")
target_include_directories(kcm_pearos PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/)
target_link_libraries(kcm_pearos breezeenhancedcommon6 breezeenhanced_STATIC KF6::I18n KF6::KCMUtils Qt::DBus)
kcmutils_generate_desktop_file(kcm_pearos)
