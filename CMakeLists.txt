cmake_minimum_required(VERSION 3.21)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(m2)

add_subdirectory(game)
add_subdirectory(lib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

file(GLOB M2_SOURCES
        "include/m2/*.h"
        "include/m2/*/*.h"
        "src/*.h"   "src/*.cc"
        "src/*/*.h" "src/*/*.cc"
        "src/*/*/*.h" "src/*/*/*.cc")

add_executable(m2 WIN32 ${M2_SOURCES} ${IMPL_SOURCES})
target_include_directories(m2 PUBLIC include ${IMPL_INCLUDE_DIRS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(m2 PUBLIC /usr/local/include/box2d /usr/local/include/SDL2)
    target_link_directories(m2 PUBLIC /usr/local/lib)
    target_link_libraries(m2 box2d SDL2_image SDL2_ttf SDL2 m)
elseif(WIN32)
    target_link_libraries(m2 debug     box2d-debug SDL_gpu-debug)
    target_link_libraries(m2 optimized box2d       SDL_gpu)
    target_link_libraries(m2 SDL2 SDL2_image SDL2_ttf)

    # Enable this option enable code inspections
#    target_compile_options(m2 PRIVATE /analyze)

    # Copy DLLs
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:m2> $<TARGET_FILE_DIR:m2>
        COMMAND_EXPAND_LISTS
    )
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE") # Temporary
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT m2)
    set_target_properties(m2 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()
