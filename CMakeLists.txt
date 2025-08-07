cmake_minimum_required(VERSION 4.0)
project(pomodoro_timer C)

set(CMAKE_C_STANDARD 11)

# Use pkg-config to find GTK 3
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Include GTK headers
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(pomodoro_timer main.c)

target_link_libraries(pomodoro_timer ${GTK3_LIBRARIES})
