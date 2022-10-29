cmake_minimum_required(VERSION 3.21)

# Properties that needs to be set BEFORE project()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0)

include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)

project(m2 C CXX ASM)

# Load external libraries
add_subdirectory(lib)
add_subdirectory(x86_64)

# protobuf
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/protobuf/bin")
    message(WARNING "libprotobuf needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/protobuf")
add_compile_definitions(PROTOBUF_USE_DLLS)
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
    file(GLOB Protobuf_DLLS "${CMAKE_CURRENT_BINARY_DIR}/lib/protobuf/bin/*.dll")
endif()
find_package(Protobuf REQUIRED)
include_directories(SYSTEM ${Protobuf_INCLUDE_DIRS})
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

# box2d
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/box2d/lib")
    message(WARNING "libbox2d needs to be built first")
    return()
endif()

# SDL2
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2/lib")
    message(WARNING "libSDL2 needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2")
find_package(SDL2 REQUIRED)

# SDL2_image
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_image/lib")
    message(WARNING "libSDL2_image needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_image")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/png")
find_package(SDL2_image REQUIRED)
set(SDL2_image_INCLUDE_DIRS "${SDL2_image_DIR}/../../include")
set(SDL2_image_LIBDIR "${SDL2_image_DIR}/../../lib")
set(SDL2_image_LIBRARIES "SDL2_image")

# SDL2_ttf
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_ttf/lib")
    message(WARNING "libSDL2_ttf needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_ttf")
find_package(SDL2_ttf REQUIRED)
set(SDL2_ttf_INCLUDE_DIRS "${SDL2_ttf_DIR}/../../../include")
set(SDL2_ttf_LIBDIR "${SDL2_ttf_DIR}/../../../lib")
set(SDL2_ttf_LIBRARIES "SDL2_ttf")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

add_subdirectory(game)

file(GLOB PROTO_FILES "pb/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}" "${IMPL_PROTO_FILES}")

file(GLOB M2_PUBLIC_HEADERS "include/m2/*.h" "include/m2/*/*.h")
file(GLOB M2_PRIVATE_HEADERS "src/*.h" "src/*/*.h" "src/*/*/*.h")
file(GLOB M2_SOURCES "src/*.cc" "src/*/*.cc" "src/*/*/*.cc")
add_executable(m2 WIN32 ${M2_PUBLIC_HEADERS} ${M2_PRIVATE_HEADERS} ${M2_SOURCES} ${IMPL_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
target_precompile_headers(m2 PRIVATE ${PROTO_HDRS})
target_include_directories(m2 PUBLIC include ${IMPL_INCLUDE_DIRS})
target_link_libraries(m2 x86_64)
if (WIN32)
    target_link_libraries(m2 debug     box2d-debug)
    target_link_libraries(m2 optimized box2d)
    target_link_libraries(m2 SDL2 SDL2_image SDL2_ttf)
    target_link_libraries(m2 ${Protobuf_LIBRARIES})
    #target_compile_options(m2 PRIVATE /analyze) # Code inspections
    # Copy DLLs
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:m2> $<TARGET_FILE_DIR:m2>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Protobuf_DLLS} $<TARGET_FILE_DIR:m2>
        COMMAND_EXPAND_LISTS
    )

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE") # Temporary
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT m2)
    set_target_properties(m2 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(m2 PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS})
    target_link_directories(m2 PUBLIC ${SDL2_image_LIBDIR} ${SDL2_ttf_LIBDIR})
    target_link_libraries(m2 box2d ${SDL2_image_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${SDL2_LIBRARIES} ${Protobuf_LIBRARIES} m)
endif()

# Tests (Build with Debug)
file(GLOB TEST_SOURCES "test/*.cc")
list(APPEND TEST_SOURCES "src/Exception.cc" "src/Log.cc" "src/M2.cc" "src/Pool.cc" "src/Vec2f.cc" "src/Vec2i.cc")
add_executable(test ${TEST_SOURCES})
target_include_directories(test PUBLIC include)
if(WIN32)
    target_link_libraries(test box2d-debug)
    target_link_libraries(test googletest_main)
    #target_compile_options(test PRIVATE /analyze) # Code inspections
    # Copy DLLs
    add_custom_command(
            TARGET test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:test> $<TARGET_FILE_DIR:test>
            COMMAND_EXPAND_LISTS
    )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(test PUBLIC /usr/local/include/SDL2)
    target_include_directories(test PUBLIC /usr/local/include)
    target_link_directories(test PUBLIC /usr/local/lib)
    target_link_libraries(test box2d SDL2_image SDL2_ttf SDL2 m)
    target_link_libraries(test gtest_main gtest gmock_main gmock)
endif()
