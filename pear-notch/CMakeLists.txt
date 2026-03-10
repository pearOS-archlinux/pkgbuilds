cmake_minimum_required(VERSION 3.14)
project(pear-notch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(LIBRSVG REQUIRED librsvg-2.0)
pkg_check_modules(LIBPULSE REQUIRED libpulse)
pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0)
pkg_check_modules(GDKPIXBUF REQUIRED gdk-pixbuf-2.0)

add_executable(pear-notch main.cpp)

target_include_directories(pear-notch PRIVATE
  ${CAIRO_INCLUDE_DIRS}
  ${X11_INCLUDE_DIRS}
  ${LIBRSVG_INCLUDE_DIRS}
  ${LIBPULSE_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GDKPIXBUF_INCLUDE_DIRS}
)

target_link_libraries(pear-notch
  ${CAIRO_LIBRARIES}
  ${X11_LIBRARIES}
  ${LIBRSVG_LIBRARIES}
  ${LIBPULSE_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GDKPIXBUF_LIBRARIES}
  pthread
)

target_compile_options(pear-notch PRIVATE ${CAIRO_CFLAGS_OTHER} ${X11_CFLAGS_OTHER} ${LIBRSVG_CFLAGS_OTHER} ${LIBPULSE_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER} ${GDKPIXBUF_CFLAGS_OTHER})
