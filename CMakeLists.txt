cmake_minimum_required(VERSION 3.16)
project(xdamageviz C)

set(CMAKE_C_STANDARD 99)

find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

include_directories(${X11_INCLUDE_DIR} include)

link_directories(${X11_LIBRARIES})
set(SOURCE_FILES src/main.c src/box_renderer.c src/options.c src/x11_util.c)

add_executable(xdamageviz ${SOURCE_FILES})

target_link_libraries(xdamageviz ${X11_LIBRARIES} Xdamage cairo pthread)