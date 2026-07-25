set(kcm_SRCS
    PearosMagicLampEffectKCM.cc
    plugin.cc
)

kconfig_add_kcfg_files(kcm_SRCS
    ../PearosMagicLampConfig.kcfgc
)

qt6_wrap_ui(kcm_SRCS PearosMagicLampEffectKCM.ui)

qt6_add_dbus_interface(kcm_SRCS ${KWIN_EFFECTS_INTERFACE} kwineffects_interface)

add_library(kwin_pearosmagiclamp_config MODULE ${kcm_SRCS})

target_link_libraries(kwin_pearosmagiclamp_config
    Qt6::Core
    Qt6::DBus
    Qt6::Gui
    Qt6::Widgets
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::ConfigWidgets
    KF6::KCMUtils
)

install(
    TARGETS
        kwin_pearosmagiclamp_config

    DESTINATION
        ${KDE_INSTALL_QTPLUGINDIR}/kwin/effects/configs/
)
