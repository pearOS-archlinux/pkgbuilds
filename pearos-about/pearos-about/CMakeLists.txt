cmake_minimum_required(VERSION 3.16)

project(pearos-about-qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

qt_standard_project_setup()

qt_add_executable(pearos-about-qt
    main.cpp
    AboutWindow.cpp
    AboutWindow.h
)

qt_add_resources(pearos-about-qt "resources"
    PREFIX "/assets"
    FILES
        assets/laptop.svg
        assets/computer.svg
)

target_link_libraries(pearos-about-qt PRIVATE
    Qt6::Widgets
    Qt6::Svg
)

