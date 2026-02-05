cmake_minimum_required(VERSION 3.16)
project(pearos-welcome CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets Network)

qt_standard_project_setup()

# If running from build dir, app can find assets/styles next to executable
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
if(NOT EXISTS "${ASSETS_DIR}/styles.qss")
  set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()

add_executable(pearos-welcome welcome.cpp)
target_link_libraries(pearos-welcome PRIVATE Qt6::Widgets Qt6::WebEngineWidgets Qt6::Network)

# So that from build dir we can run and still find ../styles.qss and ../assets/
# Optional: copy assets into build dir
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/styles.qss")
  add_custom_command(TARGET pearos-welcome POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:pearos-welcome>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/styles.qss" "$<TARGET_FILE_DIR:pearos-welcome>/styles.qss"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/list_windows.js" "$<TARGET_FILE_DIR:pearos-welcome>/list_windows.js"
  )
elseif(EXISTS "${CMAKE_SOURCE_DIR}/styles.qss")
  add_custom_command(TARGET pearos-welcome POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:pearos-welcome>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/styles.qss" "$<TARGET_FILE_DIR:pearos-welcome>/styles.qss"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/list_windows.js" "$<TARGET_FILE_DIR:pearos-welcome>/list_windows.js"
  )
endif()
