# Point to the install location of z
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../z")
# Point to the install location of png
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../png")

include(ExternalProject)

# When cross-compiling (e.g. Emscripten) forward the toolchain file into the nested ExternalProject build because
# ExternalProject does not inherit CMAKE_TOOLCHAIN_FILE automatically. Empty on native builds.
set(DEPENDENCY_TOOLCHAIN_ARGS)
if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND DEPENDENCY_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        # The Emscripten toolchain restricts find_package/find_library to its sysroot. Use `BOTH` so the nested build
        # can locate our zlib/png installed under build/<game>-WASM/lib via CMAKE_PREFIX_PATH.
        "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH"
        "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH"
        "-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH")
endif()

ExternalProject_Add(
    libfreetype
    URL ${CMAKE_CURRENT_LIST_DIR}/freetype-2.12.1.tar.gz
    URL_HASH SHA256=efe71fd4b8246f1b0b1b9bfca13cfff1c9ad85930340c27df469733bbb620938
    CMAKE_ARGS -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${DEPENDENCY_TOOLCHAIN_ARGS} -DBUILD_SHARED_LIBS=OFF -DDISABLE_FORCE_DEBUG_POSTFIX=ON -DFT_DISABLE_ZLIB=OFF -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_PNG=OFF -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON
    DEPENDS libz libpng
)
