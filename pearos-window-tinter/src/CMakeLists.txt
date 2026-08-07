set(effect_SRCS
    PearosTinterEffect.cc
    WallpaperSampler.cc
    plugin.cc
    shaders.qrc
)

kconfig_add_kcfg_files(effect_SRCS
    PearosTinterConfig.kcfgc
)

add_library(kwin4_effect_pearostinter SHARED ${effect_SRCS})

set_target_properties(kwin4_effect_pearostinter PROPERTIES AUTORCC ON)

target_link_libraries(kwin4_effect_pearostinter
    Qt6::Core
    Qt6::Gui
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::ColorScheme
    KF6::CoreAddons
    KWin::kwin
)

install(
    TARGETS
        kwin4_effect_pearostinter

    DESTINATION
        ${KDE_INSTALL_QTPLUGINDIR}/kwin/effects/plugins/
)
