cmake_minimum_required(VERSION 3.7)
project(Gravity)

find_package(SDL2 REQUIRED)

add_executable(Gravity main.c)
include_directories(Gravity ${SDL2_INCLUDE_DIRS})
target_link_libraries(Gravity ${SDL2_LIBRARIES})

