cmake_minimum_required(VERSION 3.20)
project(SystemSettings VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2 DBus Widgets Concurrent)
find_package(KF6WindowSystem QUIET)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

set(BACKEND_SOURCES
    backend/networkmanager.cpp
    backend/bluetoothmanager.cpp
    backend/audiomanager.cpp
    backend/displaymanager.cpp
    backend/batterymanager.cpp
    backend/systeminfo.cpp
    backend/wallpapermanager.cpp
    backend/updatemanager.cpp
    backend/appearancemanager.cpp
    backend/keyboardmanager.cpp
    backend/trackpadmanager.cpp
    backend/usermanager.cpp
    backend/datetimemanager.cpp
    backend/privacymanager.cpp
    backend/lockscreenmanager.cpp
    backend/startupmanager.cpp
    backend/pearidmanager.cpp
    backend/dockmanager.cpp
    backend/touchidmanager.cpp
)

qt_add_executable(SystemSettings
    main.cpp
    ${BACKEND_SOURCES}
)

set_source_files_properties(qml/components/Theme.qml     PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/components/Navigator.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(SystemSettings
    URI PearOSSettings
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/Theme.qml
        qml/components/Navigator.qml
        qml/components/PageBase.qml
        qml/components/NavSection.qml
        qml/components/NavItem.qml
        qml/components/Spacer.qml
        qml/components/CardSection.qml
        qml/components/SettingsToggle.qml
        qml/components/SettingsCard.qml
        qml/components/SettingsSlider.qml
        qml/components/SectionTitle.qml
        qml/components/CardDivider.qml
        qml/pages/GeneralPage.qml
        qml/pages/AppearancePage.qml
        qml/pages/WiFiPage.qml
        qml/pages/BluetoothPage.qml
        qml/pages/NetworkPage.qml
        qml/pages/SoundPage.qml
        qml/pages/BatteryPage.qml
        qml/pages/DisplaysPage.qml
        qml/pages/WallpaperPage.qml
        qml/pages/AboutPage.qml
        qml/pages/StoragePage.qml
        qml/pages/SoftwareUpdatePage.qml
        qml/pages/KeyboardPage.qml
        qml/pages/TrackpadPage.qml
        qml/pages/UsersPage.qml
        qml/pages/DateTimePage.qml
        qml/pages/PrivacyPage.qml
        qml/pages/LockScreenPage.qml
        qml/pages/NotificationsPage.qml
        qml/pages/StubPage.qml
        qml/pages/PearIDPage.qml
        qml/pages/PearIntelligencePage.qml
        qml/pages/DesktopDockPage.qml
        qml/pages/TouchIDPage.qml
)

target_link_libraries(SystemSettings PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
    Qt6::Widgets
    Qt6::Concurrent
)

if(KF6WindowSystem_FOUND)
    target_link_libraries(SystemSettings PRIVATE KF6::WindowSystem)
    target_compile_definitions(SystemSettings PRIVATE HAVE_KWINDOWSYSTEM)
endif()

install(TARGETS SystemSettings DESTINATION /usr/share/extras/system-settings-cpp)
