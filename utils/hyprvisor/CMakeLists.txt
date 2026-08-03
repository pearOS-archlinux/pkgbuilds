cmake_minimum_required(VERSION 3.16)
project(hyprvisor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hyprvisor
    src/main.cpp
    src/gpu_detect.cpp
    src/driver_manager.cpp
)

target_include_directories(hyprvisor PRIVATE src)

target_compile_options(hyprvisor PRIVATE -Wall -Wextra -O2)

install(TARGETS hyprvisor RUNTIME DESTINATION /usr/local/bin)
