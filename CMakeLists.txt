cmake_minimum_required(VERSION 3.17)

project(cgame)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB cgame_SOURCES "src/*.h" "src/*.c" "src/Box2D.cpp" "src/*/*.c")
add_executable(cgame ${cgame_SOURCES})
target_include_directories(cgame PUBLIC /usr/local/include/box2d /usr/local/include/SDL2)
target_link_directories(cgame PUBLIC /usr/local/lib)
target_link_libraries(cgame box2d SDL2_image SDL2_ttf SDL2 m)
