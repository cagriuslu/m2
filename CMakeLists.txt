cmake_minimum_required(VERSION 3.21)

# Options
option(GAME_IS_DETERMINISTIC "Compile the game in deterministic mode" OFF) # Usually set by the game's CMakeLists.txt
option(USE_M2_PHYSICS "Use the M2 physics engine instead of Box2D" OFF) # Usually set by the game's CMakeLists.txt
option(DEBUG "Enable debug mode" OFF)
option(BUILDING_DEPS "Signifies that CMake is executed recursively to build the dependencies" OFF)
if(DEBUG)
    add_compile_definitions(DEBUG)
endif()

# Properties that needs to be set BEFORE project()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET 13.3)
cmake_policy(SET CMP0135 OLD)

# Enable parallel build
include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)
set(CMAKE_BUILD_PARALLEL_LEVEL ${PROCESSOR_COUNT})

project(m2 C CXX)

find_program(CCACHE_PROGRAM ccache)
if(NOT CCACHE_PROGRAM)
    message(FATAL_ERROR "ccache is required but was not found on PATH. Install it and re-run CMake (e.g. `sudo apt-get install ccache`, `brew install ccache`, or see https://ccache.dev/).")
endif()
set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")

# Load external libraries
add_subdirectory(lib)
FetchContent_MakeAvailable(libbox2d)

# When cross-compiling for wasm the Emscripten toolchain restricts find_library/find_path/find_package to the Emscripten
# sysroot (via CMAKE_FIND_ROOT_PATH_MODE_* = ONLY). Our dependencies are cross-compiled and installed under
# build/<game>-WASM/lib/... (outside sysroot), so without `BOTH` they are invisible to the find_package() and
# find_library() calls.
if(EMSCRIPTEN)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
endif()

# protobuf
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/protobuf/bin")
    BuildDependency(libprotobuf)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/protobuf")
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
endif()
set(Protobuf_USE_STATIC_LIBS ON)
# For Emscripten only libprotobuf is cross-compiled. protoc must come from the native build.
if(EMSCRIPTEN)
    # Mirrors the binaryDir naming in CMakePresets.json: POSIX presets suffix the host OS name
    # (e.g. build/RPG-Linux-DEBUG), while the Windows presets don't (build/RPG-DEBUG).
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(HOST_NATIVE_BUILD_DIR "${CMAKE_SOURCE_DIR}/build/${GAME}-DEBUG")
    else()
        set(HOST_NATIVE_BUILD_DIR "${CMAKE_SOURCE_DIR}/build/${GAME}-${CMAKE_HOST_SYSTEM_NAME}-DEBUG")
    endif()
    set(HOST_PROTOC_EXECUTABLE "${HOST_NATIVE_BUILD_DIR}/lib/protobuf/bin/protoc")
    if(NOT EXISTS "${HOST_PROTOC_EXECUTABLE}")
        message(FATAL_ERROR "Host protoc not found at ${HOST_PROTOC_EXECUTABLE}. Build native ${GAME} first.")
    endif()
    set(Protobuf_PROTOC_EXECUTABLE "${HOST_PROTOC_EXECUTABLE}" CACHE FILEPATH "Host protoc reused by the wasm build" FORCE)
    message(STATUS "Using host protoc for wasm codegen: ${Protobuf_PROTOC_EXECUTABLE}")
endif()
find_package(Protobuf REQUIRED)
include_directories(SYSTEM ${Protobuf_INCLUDE_DIRS})
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

# SDL3
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL3/lib")
    BuildDependency(libSDL3)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL3")
find_package(SDL3 REQUIRED)

# SDL3_image
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL3_image/lib")
    BuildDependency(libSDL3_image)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL3_image")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/z")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/png")
find_package(SDL3_image REQUIRED)

# SDL3_ttf
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL3_ttf/lib")
    BuildDependency(libSDL3_ttf)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL3_ttf")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/freetype")
find_package(SDL3_ttf REQUIRED)

# sqlite
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/sqlite/libsqlite3.a")
    BuildDependency(sqlite3)
endif()

# genORM
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/genORM/lib")
    BuildDependency(genORM-cxx)
endif()
set(GENORM_CXX_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/lib/genORM/include")
set(GENORM_CXX_LIBDIR "${CMAKE_CURRENT_BINARY_DIR}/lib/genORM/lib")
set(GENORM_CXX_LIBRARIES "${GENORM_CXX_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}genORM-cxx${CMAKE_STATIC_LIBRARY_SUFFIX}")

# png
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/png/lib")
    BuildDependency(libpng)
endif()
set(png_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/lib/png/include")
set(png_LIBDIR "${CMAKE_CURRENT_BINARY_DIR}/lib/png/lib")
# libpng's static library name differs per toolchain (libpng.a vs libpng16_static[d].lib)
find_library(png_LIBRARIES NAMES png png16 png16_static png16_staticd PATHS "${png_LIBDIR}" NO_DEFAULT_PATH REQUIRED)

# z
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/z/lib")
    BuildDependency(libz)
endif()
set(z_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/lib/z/include")
set(z_LIBDIR "${CMAKE_CURRENT_BINARY_DIR}/lib/z/lib")
# zlib's static library name differs per toolchain (libz.a vs zlibstatic[d].lib)
find_library(z_LIBRARIES NAMES z zlibstatic zlibstaticd PATHS "${z_LIBDIR}" NO_DEFAULT_PATH REQUIRED)
if(EMSCRIPTEN)
    # The wasm zlib build emits BOTH a static archive (libz.a) and a wasm side module (libz.so). Force the static
    # archive so the link stays fully static. Native builds are already static.
    set(z_LIBRARIES "${z_LIBDIR}/libz.a" CACHE FILEPATH "Static zlib archive for the wasm build" FORCE)
endif()

if(DEBUG AND NOT EMSCRIPTEN)
    # GTest
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/googletest/lib")
        BuildDependency(libgtest)
    endif()
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/googletest")
    find_package(GTest REQUIRED)
endif()

# Compiler flags
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if(EMSCRIPTEN)
    # em++ disables exception catching by default, enable it globally.
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
endif()

# Get git commit hash
find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHORT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CMakeProject.h.in" "${CMAKE_CURRENT_BINARY_DIR}/CMakeProject.h")

add_subdirectory(game)

# Print options
message("GAME_IS_DETERMINISTIC: ${GAME_IS_DETERMINISTIC}")
if(GAME_IS_DETERMINISTIC)
    add_compile_definitions(_GAME_IS_DETERMINISTIC)
endif()

message("USE_M2_PHYSICS: ${USE_M2_PHYSICS}")
if(USE_M2_PHYSICS)
    add_compile_definitions(_USE_M2_PHYSICS)
endif()

# Generate protobuf files
# Extract the filenames of game proto files
foreach(GAME_PROTO_FILE ${GAME_PROTO_FILES})
    cmake_path(GET GAME_PROTO_FILE FILENAME GAME_PROTO_FILENAME)
    list(APPEND GAME_PROTO_FILENAMES "${GAME_PROTO_FILENAME}")
endforeach()
# Gather names of the default proto files
file(GLOB PROTO_FILES_WITH_DEFAULT_FALLBACK RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/pb-default" "pb-default/*.proto")
# Add default proto filesto game proto files if an alternative isn't provided by the game
foreach(PROTO_DEFAULT_FILE ${PROTO_FILES_WITH_DEFAULT_FALLBACK})
    if (NOT PROTO_DEFAULT_FILE IN_LIST GAME_PROTO_FILENAMES)
        message("Using the default ${PROTO_DEFAULT_FILE} as it's not provided by the game")
        list(APPEND GAME_PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pb-default/${PROTO_DEFAULT_FILE}")
    endif()
endforeach()
file(GLOB PROTO_FILES "pb/*.proto")
if(GAME_IS_DETERMINISTIC)
    # Gather names of the deterministic proto files
    file(GLOB DETERMINISTIC_PROTO_FILES "pb-deterministic/*.proto")
    list(APPEND PROTO_FILES ${DETERMINISTIC_PROTO_FILES})
else()
    # Gather names of the undeterministic proto files
    file(GLOB UNDETERMINISTIC_PROTO_FILES "pb-undeterministic/*.proto")
    list(APPEND PROTO_FILES ${UNDETERMINISTIC_PROTO_FILES})
endif()
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}" "${GAME_PROTO_FILES}")

add_library(m2_pb OBJECT ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(m2_pb SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIRS})
target_link_libraries(m2_pb PUBLIC ${Protobuf_LIBRARIES})
if (MSVC)
    set_property(TARGET m2_pb PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    target_compile_options(m2_pb PRIVATE "/MP")
endif()

file(GLOB m2_common_SOURCES
        "common/src/*.cc"
        "common/src/*/*.cc"
        "common/src/*/*/*.cc"
        "common/include/m2/common/*.h"
        "common/include/m2/common/*/*.h"
        "common/include/m2/common/*/*/*.h"
)
add_library(m2_common STATIC ${m2_common_SOURCES})
target_include_directories(m2_common PUBLIC common/include)
target_compile_features(m2_common PUBLIC cxx_std_23)
target_link_libraries(m2_common PUBLIC m2_pb)
if (MSVC)
    set_property(TARGET m2_common PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    target_compile_options(m2_common PRIVATE "/MP")
endif()

file(GLOB m2_thirdparty_sdl_SOURCES
        "thirdparty/sdl/src/*.cc"
        "thirdparty/sdl/src/*/*.cc"
        "thirdparty/sdl/src/*/*/*.cc"
        "thirdparty/sdl/src/*.h"
        "thirdparty/sdl/src/*/*.h"
        "thirdparty/sdl/src/*/*/*.h"
        "thirdparty/sdl/include/m2/thirdparty/*.h"
        "thirdparty/sdl/include/m2/thirdparty/*/*.h"
        "thirdparty/sdl/include/m2/thirdparty/*/*/*.h"
)
add_library(m2_thirdparty_sdl STATIC ${m2_thirdparty_sdl_SOURCES})
target_include_directories(m2_thirdparty_sdl PUBLIC thirdparty/sdl/include)
target_compile_features(m2_thirdparty_sdl PUBLIC cxx_std_23)
target_link_libraries(m2_thirdparty_sdl PUBLIC m2_common)
# SDL3 imported targets are PRIVATE: their headers are needed to compile this wrapper but must NOT leak to
# consumers, and because this is a STATIC library CMake propagates the link requirements to consumers as
# $<LINK_ONLY:...> (the final m2 binary links them, but their include dirs are not inherited). The imported
# targets carry their own include dirs and transitive dependencies (SDL3, freetype, system frameworks).
target_link_libraries(m2_thirdparty_sdl PRIVATE SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image SDL3::SDL3-static)
if (MSVC)
    set_property(TARGET m2_thirdparty_sdl PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    target_compile_options(m2_thirdparty_sdl PRIVATE "/MP")
endif()

file(GLOB M2_PUBLIC_HEADERS "include/m2/*.h" "include/m2/*/*.h" "include/m2/*/*/*.h" "include/m2/*/*/*/*.h")
file(GLOB M2_PRIVATE_HEADERS "src/*.h" "src/*/*.h" "src/*/*/*.h")
file(GLOB M2_SOURCES "src/*.cc" "src/*/*.cc" "src/*/*/*.cc" "src/*/*/*/*.cc" "src/*/*/*/*/*.cc")
# Add platform specific source files
if (EMSCRIPTEN)
    file(GLOB M2_PLATFORM_SPECIFIC_SOURCES "platform/emscripten/*.h" "platform/emscripten/*.cc")
elseif (WIN32)
    file(GLOB M2_PLATFORM_SPECIFIC_SOURCES "platform/windows/*.h" "platform/windows/*.cc")
elseif(APPLE)
    file(GLOB M2_PLATFORM_SPECIFIC_SOURCES "platform/macos/*.h" "platform/macos/*.cc" "platform/posix/*.h" "platform/posix/*.cc")
else()
    file(GLOB M2_PLATFORM_SPECIFIC_SOURCES "platform/linux/*.h" "platform/linux/*.cc" "platform/posix/*.h" "platform/posix/*.cc")
endif()

if(APPLE)
	# Put resources into the .app
	file(GLOB_RECURSE BUNDLE_RESOURCES "${CMAKE_SOURCE_DIR}/resource/default/*" "${CMAKE_SOURCE_DIR}/resource/fonts/*" "${CMAKE_SOURCE_DIR}/resource/game/${GAME}/*")
	foreach (ABSOLUTE_PATH_TO_RESOURCE ${BUNDLE_RESOURCES})
		file(RELATIVE_PATH RELATIVE_PATH_TO_RESOURCE "${CMAKE_SOURCE_DIR}/resource" ${ABSOLUTE_PATH_TO_RESOURCE})
		get_filename_component(RELATIVE_PATH_TO_RESOURCES_DIRECTORY ${RELATIVE_PATH_TO_RESOURCE} DIRECTORY)
		# Set it's location inside the app package (under Resources)
		set_property(SOURCE ${ABSOLUTE_PATH_TO_RESOURCE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RELATIVE_PATH_TO_RESOURCES_DIRECTORY}")
		# Add the file to the 'Resources' folder group in Xcode. This also preserves folder structure.
		source_group("Resources/${RELATIVE_PATH_TO_RESOURCES_DIRECTORY}" FILES "${ABSOLUTE_PATH_TO_RESOURCE}")
	endforeach ()
endif()

set(ORM_SOURCES "${CMAKE_BINARY_DIR}/M2.orm.h;${CMAKE_BINARY_DIR}/M2.orm.cc")
add_executable(m2 WIN32 MACOSX_BUNDLE ${M2_PUBLIC_HEADERS} ${M2_PRIVATE_HEADERS} ${M2_SOURCES} ${M2_PLATFORM_SPECIFIC_SOURCES} ${GAME_SOURCES} ${ORM_SOURCES} ${BUNDLE_RESOURCES})
add_dependencies(m2 build_orm_sources)

# Name the produced binary after the game (RPG, RPG.exe, RPG.html, ...) on every platform.
# The CMake target itself stays "m2" so build scripts and generator expressions keep working.
set_target_properties(m2 PROPERTIES OUTPUT_NAME "${GAME}")

if (MSVC)
    # Enable parallel build for MSVC
    target_compile_options(m2 PRIVATE "/MP")
    # This flag build the target using the static system library, not dynamic.
    # Use MultiThreadedDebugDLL in debug build, otherwise use MultiThreadedDLL.
    set_property(TARGET box2d PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set_property(TARGET m2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else()
    # Enable precompiled headers on Posix platforms. Beyond the protobuf-generated headers, precompile the heaviest
    # standard-library headers.
    target_precompile_headers(m2 PRIVATE
            ${PROTO_HDRS}
            <algorithm> <array> <cmath> <cstdint> <filesystem> <format> <functional>
            <list> <map> <memory> <optional> <span> <string> <string_view> <tuple>
            <unordered_map> <utility> <variant> <vector>)
endif()

target_include_directories(m2 PUBLIC include ${GAME_INCLUDE_DIRS})
target_include_directories(m2 PUBLIC ${GENORM_CXX_INCLUDE_DIRS} ${png_INCLUDE_DIRS} ${z_INCLUDE_DIRS})
target_link_directories(m2 PUBLIC ${GENORM_CXX_LIBDIR} ${png_LIBDIR} ${z_LIBDIR})
target_link_libraries(m2 m2_thirdparty_sdl box2d ${GENORM_CXX_LIBRARIES} sqlite3 ${png_LIBRARIES} ${z_LIBRARIES})

if (WIN32)
    #target_compile_options(m2 PRIVATE /analyze) # Code inspections
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE") # Temporary
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT m2)
    set_target_properties(m2 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
elseif(APPLE)
    target_link_libraries(m2 "-framework Foundation")

	set_target_properties(m2 PROPERTIES
			MACOSX_BUNDLE_BUNDLE_NAME "${GAME}"
			MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0"
			MACOSX_BUNDLE_COPYRIGHT "2024 Cagri Uslu"
			MACOSX_BUNDLE_GUI_IDENTIFIER "dev.uslu.${GAME}")

	# Put the icon into the .app/Contents/Resources
    # Check if the game provides an icon, otherwise, copy the default icon
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/resource/game/${GAME}/${GAME}.icns")
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/resource/default.icns $<TARGET_FILE_DIR:m2>/../Resources
                COMMAND_EXPAND_LISTS
        )
    else()
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/resource/game/${GAME}/${GAME}.icns $<TARGET_FILE_DIR:m2>/../Resources
                COMMAND_EXPAND_LISTS
        )
    endif()
else()
    # The following packages must be install before BUILDING SDL3 in Ubuntu: libxext-dev libgl-dev libasound2-dev libpulse-dev
    target_link_libraries(m2 GL)
endif()

if(EMSCRIPTEN)
    target_link_options(m2 PRIVATE
        -sASYNCIFY                # Allow emscripten_sleep() to yield the thread to the browser, suspend/resume the stack
        -sALLOW_MEMORY_GROWTH=1   # Let the heap grow
        -sGROWABLE_ARRAYBUFFERS=0 # Recreate HEAP views on growth instead of a resizable ArrayBuffer;
                                  # Firefox's TextDecoder.decode() rejects views over resizable buffers,
                                  # which otherwise crashes UTF8ToString at boot.
        -sINITIAL_MEMORY=67108864 # 64MiB
        -sSTACK_SIZE=4194304      # 4MiB
        -sFULL_ES2=1              # Full GLES2 for the WebGL
        -sEXIT_RUNTIME=0          # Keep the runtime running
        -sSINGLE_FILE=1           # Inline the JS and base64-embed the wasm into the HTML shell
    )

    # Asset packaging + HTML shell
    # --embed-file (rather than --preload-file) stores asset contents inside the wasm/js at link
    # time instead of a separate .data bundle, which SINGLE_FILE=1 does not fold into the HTML.
    target_link_options(m2 PRIVATE
        "SHELL:--embed-file ${CMAKE_SOURCE_DIR}/resource/default@resource/default"
        "SHELL:--embed-file ${CMAKE_SOURCE_DIR}/resource/fonts@resource/fonts"
        "SHELL:--embed-file ${CMAKE_SOURCE_DIR}/resource/game/${GAME}@resource/game/${GAME}"
        "SHELL:--shell-file  ${CMAKE_SOURCE_DIR}/platform/emscripten/shell.html"
    )

    # Give the .html extension to the output
    set_target_properties(m2 PROPERTIES SUFFIX ".html")
endif()

if(GTest_FOUND)
    message("Building tests")
    # Tests (Build with Debug)
    file(GLOB TEST_SOURCES "test/*.cc")
    list(APPEND TEST_SOURCES
            "src/Log.cc"
            "src/M2.cc"
            "src/Options.cc"
            "src/physics/Particle.cc"
            "src/physics/RigidBody.cc"
            "src/VmAddScript.cc"
            "src/VmExecute.cc"
            "src/protobuf/Detail.cc"
            "src/thirdparty/compression/Inflate.cc"
            "src/thirdparty/compression/Deflate.cc"
            "src/thirdparty/protobuf/Json.cc"
            "src/ui/Layout.cc"
            "src/audio/Song.cc"
            "src/audio/synthesizer/Detail.cc"
            "src/reflect/utils/Proto.cc"
            "src/reflect/utils/Hash.cc"
    )
    add_executable(test ${TEST_SOURCES} ${ORM_SOURCES})
    target_include_directories(test PUBLIC include thirdparty/sdl/include)
    target_include_directories(test PUBLIC ${GENORM_CXX_INCLUDE_DIRS})
    target_link_directories(test PUBLIC ${GENORM_CXX_LIBDIR} ${z_LIBDIR})
    target_link_libraries(test m2_common box2d SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf SDL3::SDL3-static ${GENORM_CXX_LIBRARIES} sqlite3 ${z_LIBRARIES})
    target_link_libraries(test GTest::gtest GTest::gtest_main)
    if(WIN32)
        #target_compile_options(test PRIVATE /analyze) # Code inspections
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_link_libraries(test m)
    endif()
endif()
