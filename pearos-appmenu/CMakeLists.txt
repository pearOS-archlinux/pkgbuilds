cmake_minimum_required(VERSION 3.16)

project(plasma-appmenu VERSION 6.7.80)

set(QT_MIN_VERSION "6.10.0")
set(KF6_MIN_VERSION "6.26.0")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQmlModule)
include(ECMQtDeclareLoggingCategory)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Widgets Quick DBus)
find_package(Plasma REQUIRED)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS WindowSystem I18n Config CoreAddons)
find_package(LibTaskManager REQUIRED)

# Build bundled libdbusmenuqt (Qt6 version not available as a system package)
add_subdirectory(libdbusmenuqt)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.appmenu")

plasma_add_applet(org.kde.plasma.appmenu
    QML_SOURCES
        qml/configGeneral.qml
        qml/config.qml
        qml/main.qml
        qml/MenuDelegate.qml
    CPP_SOURCES
        appmenuapplet.cpp
        appmenumodel.cpp
    RESOURCES
        main.xml
    QML_ARGS DEPENDENCIES QtCore org.kde.plasma.core
)

target_link_libraries(org.kde.plasma.appmenu PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Quick
    Qt::DBus
    Plasma::Plasma
    KF6::WindowSystem
    KF6::I18n
    PW::LibTaskManager
    dbusmenuqt
)
