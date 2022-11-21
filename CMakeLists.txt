cmake_minimum_required(VERSION 3.7)
project(Gravity)

find_package(SDL2 REQUIRED)

add_executable(gravity main.c world_data.c)
include_directories(gravity ${SDL2_INCLUDE_DIRS})
target_link_libraries(gravity ${SDL2_LIBRARIES})

IF (NOT WIN32)
  target_link_libraries(gravity m)
ENDIF()

if(MSVC)
  target_compile_options(gravity PRIVATE /W4 /WX)
else()
  target_compile_options(gravity PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()