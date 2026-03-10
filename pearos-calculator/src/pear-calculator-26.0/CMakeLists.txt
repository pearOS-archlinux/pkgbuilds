cmake_minimum_required(VERSION 3.16)
project(calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(calculator
    main.cpp
    MainWindow.cpp
    MainWindow.h
    CalculatorWidget.cpp
    CalculatorWidget.h
)

target_link_libraries(calculator PRIVATE Qt6::Widgets)

include(GNUInstallDirs)
install(TARGETS calculator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES calculator.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES settings.json DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/extras/calculator)
install(FILES assets/calculator.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/extras/calculator)

# make package din build/ invocă package din rădăcina proiectului
add_custom_target(package
  COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR} package
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Running make package from project root..."
)
