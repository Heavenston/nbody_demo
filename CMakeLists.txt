cmake_minimum_required(VERSION 3.7)
project(Gravity)

find_package(SDL2 REQUIRED)

add_executable(gravity main.c)
include_directories(gravity ${SDL2_INCLUDE_DIRS})
target_link_libraries(gravity ${SDL2_LIBRARIES} m)

