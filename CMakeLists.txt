cmake_minimum_required(VERSION 3.21)

# Options
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

# Load external libraries
add_subdirectory(lib)
FetchContent_MakeAvailable(libbox2d libexpected)

# protobuf
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/protobuf/bin")
    BuildDependency(libprotobuf)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/protobuf")
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
endif()
set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)
include_directories(SYSTEM ${Protobuf_INCLUDE_DIRS})
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

# SDL2
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2/lib")
    BuildDependency(libSDL2)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2")
find_package(SDL2 REQUIRED)
if (WIN32)
    set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/../include;${SDL2_DIR}/../include/SDL2") # On Windows, SDL2_INCLUDE_DIRS is not populated?!
endif()

# SDL2_image
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_image/lib")
    BuildDependency(libSDL2_image)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_image")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/z")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/png")
find_package(SDL2_image REQUIRED)
if (WIN32)
    set(SDL2_image_INCLUDE_DIRS "${SDL2_image_DIR}/../include")
    set(SDL2_image_LIBDIR "${SDL2_image_DIR}/../lib")
    set(SDL2_image_LIBNAME "SDL2_image::SDL2_image-static")
elseif(APPLE)
    set(SDL2_image_INCLUDE_DIRS "${SDL2_image_DIR}/../../include")
    set(SDL2_image_LIBDIR "${SDL2_image_DIR}/../../lib")
    set(SDL2_image_LIBNAME "SDL2_image")
else()
    set(SDL2_image_INCLUDE_DIRS "${SDL2_image_DIR}/../../../include")
    set(SDL2_image_LIBDIR "${SDL2_image_DIR}/../../../lib")
    set(SDL2_image_LIBNAME "SDL2_image")
endif()

# SDL2_ttf
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_ttf/lib")
    BuildDependency(libSDL2_ttf)
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/SDL2_ttf")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib/freetype")
find_package(SDL2_ttf REQUIRED)
if (WIN32)
    set(SDL2_ttf_INCLUDE_DIRS "${SDL2_ttf_DIR}/../include")
    set(SDL2_ttf_LIBDIR "${SDL2_ttf_DIR}/../lib")
    # Since we're statically linking to SDL2_ttf, we need to statically link to freetype as well
    set(Freetype_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lib/freetype/lib/freetype.lib")
else()
    set(SDL2_ttf_INCLUDE_DIRS "${SDL2_ttf_DIR}/../../../include")
    set(SDL2_ttf_LIBDIR "${SDL2_ttf_DIR}/../../../lib")
	# Since we're statically linking to SDL2_ttf, we need to statically link to freetype as well
	set(Freetype_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lib/freetype/lib/libfreetype.a")
endif()

if(DEBUG)
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

# Post-build actions
#add_custom_command(
#        OUTPUT ${CMAKE_SOURCE_DIR}/resource.zip
#        COMMAND ${CMAKE_COMMAND}
#        ARGS -E tar cf ${CMAKE_SOURCE_DIR}/resource.zip --format=zip ${CMAKE_SOURCE_DIR}/resource/
#        DEPENDS ${CMAKE_SOURCE_DIR}/resource    # TODO this does not check if a new directory is created inside
#)

file(GLOB M2_PUBLIC_HEADERS "include/m2/*.h" "include/m2/*/*.h")
file(GLOB M2_PRIVATE_HEADERS "src/*.h" "src/*/*.h" "src/*/*/*.h")
file(GLOB M2_SOURCES "src/*.cc" "src/*/*.cc" "src/*/*/*.cc")
# Add platform specific source files
if (WIN32)
    file(GLOB M2_PLATFORM_SPECIFIC_SOURCES "platform/windows/*.h" "platform/windows/*.cc")
elseif(APPLE)
    file(GLOB M2_PLATFORM_SPECIFIC_SOURCES "platform/macos/*.h" "platform/macos/*.cc" "platform/posix/*.h" "platform/posix/*.cc")
else()
    file(GLOB M2_PLATFORM_SPECIFIC_SOURCES "platform/linux/*.h" "platform/linux/*.cc" "platform/posix/*.h" "platform/posix/*.cc")
endif()

if(APPLE)
	# Put resources into the .app
	file(GLOB_RECURSE BUNDLE_RESOURCES "${CMAKE_SOURCE_DIR}/resource/*")
	foreach (ABSOLUTE_PATH_TO_RESOURCE ${BUNDLE_RESOURCES})
		file(RELATIVE_PATH RELATIVE_PATH_TO_RESOURCE "${CMAKE_SOURCE_DIR}/resource" ${ABSOLUTE_PATH_TO_RESOURCE})
		get_filename_component(RELATIVE_PATH_TO_RESOURCES_DIRECTORY ${RELATIVE_PATH_TO_RESOURCE} DIRECTORY)
		# Set it's location inside the app package (under Resources)
		set_property(SOURCE ${ABSOLUTE_PATH_TO_RESOURCE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RELATIVE_PATH_TO_RESOURCES_DIRECTORY}")
		# Add the file to the 'Resources' folder group in Xcode. This also preserves folder structure.
		source_group("Resources/${RELATIVE_PATH_TO_RESOURCES_DIRECTORY}" FILES "${ABSOLUTE_PATH_TO_RESOURCE}")
	endforeach ()
endif()

add_executable(m2 WIN32 MACOSX_BUNDLE ${M2_PUBLIC_HEADERS} ${M2_PRIVATE_HEADERS} ${M2_SOURCES} ${M2_PLATFORM_SPECIFIC_SOURCES} ${GAME_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS} ${BUNDLE_RESOURCES})

if (MSVC)
    # Enable parallel build for MSVC
    target_compile_options(m2 PRIVATE "/MP")
    # This flag build the target using the static system library, not dynamic.
    # Use MultiThreadedDebugDLL in debug build, otherwise use MultiThreadedDLL.
    set_property(TARGET box2d PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set_property(TARGET expected PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set_property(TARGET m2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else()
    # Enable precompiled headers on Posix platforms
    target_precompile_headers(m2 PRIVATE ${PROTO_HDRS})
endif()

target_include_directories(m2 PUBLIC include ${GAME_INCLUDE_DIRS})
target_include_directories(m2 PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS})
target_link_directories(m2 PUBLIC ${SDL2_image_LIBDIR} ${SDL2_ttf_LIBDIR})
target_link_libraries(m2 box2d ${SDL2_image_LIBNAME} SDL2_ttf ${Freetype_LIBRARIES} ${SDL2_STATIC_LIBRARIES} ${Protobuf_LIBRARIES} expected)

if (WIN32)
    #target_compile_options(m2 PRIVATE /analyze) # Code inspections
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE") # Temporary
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT m2)
    set_target_properties(m2 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
elseif(APPLE)
    target_link_libraries(m2 "-framework Foundation" m)

	set_target_properties(m2 PROPERTIES
			OUTPUT_NAME "${GAME}"
			MACOSX_BUNDLE_BUNDLE_NAME "${GAME}"
			MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0"
			MACOSX_BUNDLE_COPYRIGHT "2024 Cagri Uslu"
			MACOSX_BUNDLE_GUI_IDENTIFIER "dev.uslu.${GAME}")

	# Put the icon into the .app/Contents/Resources
	add_custom_command(
			TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/resource/game/${GAME}/${GAME}.icns $<TARGET_FILE_DIR:m2>/../Resources
			COMMAND_EXPAND_LISTS
	)
else()
    # The following packages must be install before BUILDING SDL2 in Ubuntu: libxext-dev libgl-dev libasound2-dev libpulse-dev
    target_link_libraries(m2 GL)
endif()

if(GTest_FOUND)
    message("Building tests")
    # Tests (Build with Debug)
    file(GLOB TEST_SOURCES "test/*.cc")
    list(APPEND TEST_SOURCES
            "src/Error.cc"
            "src/FileSystem.cc"
            "src/Log.cc"
            "src/Math.cc"
            "src/Meta.cc"
            "src/M2.cc"
            "src/Options.cc"
            "src/physics/Particle.cc"
            "src/String.cc"
            "src/Synth.cc"
            "src/VmAddScript.cc"
            "src/VmExecute.cc"
            "src/Vson.cc"
            "src/containers/MapF.cc"
            "src/containers/Pool.cc"
            "src/game/Graph.cc"
            "src/math/Rational.cc"
            "src/math/RectF.cc"
            "src/math/RectI.cc"
            "src/math/VecF.cc"
            "src/math/VecI.cc"
            "src/math/primitives/Fixed.cc"
            "src/math/primitives/Float.cc"
            "src/protobuf/Detail.cc"
    )
    add_executable(test ${TEST_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
    target_include_directories(test PUBLIC include)
    target_include_directories(test PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_image_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS})
    target_link_directories(test PUBLIC ${SDL2_image_LIBDIR} ${SDL2_ttf_LIBDIR})
    target_link_libraries(test box2d ${SDL2_image_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${SDL2_LIBRARIES} ${Protobuf_LIBRARIES} expected)
    target_link_libraries(test GTest::gtest GTest::gtest_main)
    if(WIN32)
        #target_compile_options(test PRIVATE /analyze) # Code inspections
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_link_libraries(test m)
    endif()
endif()
