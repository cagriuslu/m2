cmake_minimum_required(VERSION 3.21)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(cgame)

add_subdirectory(lib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB cgame_SOURCES "src/*.h" "src/*.c" "src/*/*.c" "src/*.cpp")
add_executable(cgame WIN32 ${cgame_SOURCES})
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(cgame PUBLIC /usr/local/include/box2d /usr/local/include/SDL2)
    target_link_directories(cgame PUBLIC /usr/local/lib)
    target_link_libraries(cgame box2d SDL2_image SDL2_ttf SDL2 m)
elseif(WIN32)
    target_link_libraries(cgame debug     box2d-debug SDL_gpu-debug)
    target_link_libraries(cgame optimized box2d       SDL_gpu)
    target_link_libraries(cgame SDL2 SDL2_image SDL2_ttf)

    # Copy DLLs
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:cgame> $<TARGET_FILE_DIR:cgame>
        COMMAND_EXPAND_LISTS
    )
endif()

if(WIN32)
    # Temporary
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")

    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT cgame)
    set_target_properties(cgame PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()
