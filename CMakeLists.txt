cmake_minimum_required(VERSION 3.21)

# Options
option(DEBUG "Enable debug mode" OFF)
if(DEBUG)
    add_compile_definitions(DEBUG)
endif()

# Properties that needs to be set BEFORE project()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET 12.6)
cmake_policy(SET CMP0135 OLD)

include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)
set(CMAKE_BUILD_PARALLEL_LEVEL ${PROCESSOR_COUNT})

project(m2 C CXX ASM)

# Load external libraries
add_subdirectory(lib)

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
if (WIN32)
    set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/../include;${SDL2_DIR}/../include/SDL2") # On Windows, SDL2_INCLUDE_DIRS is not populated?!
    file(GLOB SDL2_DLLS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2/bin/*.dll")
endif()

# SDL2_image
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_image/lib")
    message(WARNING "libSDL2_image needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_image")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/png")
find_package(SDL2_image REQUIRED)
if (WIN32)
    set(SDL2_image_INCLUDE_DIRS "${SDL2_image_DIR}/../include")
    set(SDL2_image_LIBDIR "${SDL2_image_DIR}/../lib")
    file(GLOB SDL2_image_DLLS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_image/bin/*.dll" "${CMAKE_CURRENT_BINARY_DIR}/lib/png/bin/*.dll" "${CMAKE_CURRENT_BINARY_DIR}/lib/z/bin/*.dll")
else()
    set(SDL2_image_INCLUDE_DIRS "${SDL2_image_DIR}/../../include")
    set(SDL2_image_LIBDIR "${SDL2_image_DIR}/../../lib")
endif()
set(SDL2_image_LIBRARIES "SDL2_image")

# SDL2_ttf
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_ttf/lib")
    message(WARNING "libSDL2_ttf needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_ttf")
find_package(SDL2_ttf REQUIRED)
if (WIN32)
    set(SDL2_ttf_INCLUDE_DIRS "${SDL2_ttf_DIR}/../include")
    set(SDL2_ttf_LIBDIR "${SDL2_ttf_DIR}/../lib")
    file(GLOB SDL2_ttf_DLLS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_ttf/bin/*.dll")
else()
    set(SDL2_ttf_INCLUDE_DIRS "${SDL2_ttf_DIR}/../../../include")
    set(SDL2_ttf_LIBDIR "${SDL2_ttf_DIR}/../../../lib")
endif()
set(SDL2_ttf_LIBRARIES "SDL2_ttf")

if(DEBUG)
    # GTest
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/googletest")
        message(WARNING "libgtest needs to be built first")
        return()
    endif()
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/googletest")
    find_package(GTest REQUIRED)
endif()

# Compiler flags
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

# Generate protobuf files
file(GLOB PROTO_FILES "pb/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}" "${GAME_PROTO_FILES}")

file(GLOB M2_PUBLIC_HEADERS "include/m2/*.h" "include/m2/*/*.h")
file(GLOB M2_PRIVATE_HEADERS "src/*.h" "src/*/*.h" "src/*/*/*.h")
file(GLOB M2_SOURCES "src/*.cc" "src/*/*.cc" "src/*/*/*.cc")
add_executable(m2 WIN32 ${M2_PUBLIC_HEADERS} ${M2_PRIVATE_HEADERS} ${M2_SOURCES} ${GAME_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
if (NOT WIN32)
    target_precompile_headers(m2 PRIVATE ${PROTO_HDRS})
endif()
target_include_directories(m2 PUBLIC include ${GAME_INCLUDE_DIRS})
target_include_directories(m2 PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS})
target_link_directories(m2 PUBLIC ${SDL2_image_LIBDIR} ${SDL2_ttf_LIBDIR})
target_link_libraries(m2 box2d ${SDL2_image_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${SDL2_LIBRARIES} ${Protobuf_LIBRARIES})
if (WIN32)
    #target_compile_options(m2 PRIVATE /analyze) # Code inspections
    # Copy DLLs
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Protobuf_DLLS} ${SDL2_DLLS} ${SDL2_image_DLLS} ${SDL2_ttf_DLLS} $<TARGET_FILE_DIR:m2>
        COMMAND_EXPAND_LISTS
    )

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE") # Temporary
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT m2)
    set_target_properties(m2 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(m2 m)
endif()

if(GTest_FOUND)
    # Tests (Build with Debug)
    file(GLOB TEST_SOURCES "test/*.cc")
    list(APPEND TEST_SOURCES "src/Exception.cc" "src/Log.cc" "src/M2.cc" "src/Pool.cc" "src/Rational.cc" "src/Vec2f.cc" "src/Vec2i.cc")
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
        target_include_directories(test PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS})
        target_link_directories(test PUBLIC ${SDL2_image_LIBDIR} ${SDL2_ttf_LIBDIR})
        target_link_libraries(test box2d ${SDL2_image_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${SDL2_LIBRARIES} ${Protobuf_LIBRARIES} m)
        target_link_libraries(test GTest::gtest GTest::gtest_main)
    endif()
endif()
