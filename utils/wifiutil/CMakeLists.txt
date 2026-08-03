cmake_minimum_required(VERSION 3.20)
project(wifiutil VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(wifiutil
    src/main.cpp
    src/wifi_detect.cpp
    src/driver_installer.cpp
)

target_include_directories(wifiutil PRIVATE src)
target_compile_options(wifiutil PRIVATE -Wall -Wextra -O2)

# Install binary
install(TARGETS wifiutil RUNTIME DESTINATION /usr/local/bin)
