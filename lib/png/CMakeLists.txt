# Point to the install location of z
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../z")

include(ExternalProject)

# When cross-compiling (e.g. Emscripten) forward the toolchain file into the nested ExternalProject build because
# ExternalProject does not inherit CMAKE_TOOLCHAIN_FILE automatically. Empty on native builds.
set(DEPENDENCY_TOOLCHAIN_ARGS)
if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND DEPENDENCY_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        # The Emscripten toolchain restricts find_package/find_library to its sysroot. Relax it so this
        # nested build can locate our sibling zlib installed under build/<game>-WASM/lib via CMAKE_PREFIX_PATH.
        "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH"
        "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH"
        "-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH"
        # Emscripten misdetects the target architecture as x86 and would enable SSE intrinsics that do
        # not exist in the wasm sysroot. Disable libpng's hardware optimizations for the wasm build.
        "-DPNG_HARDWARE_OPTIMIZATIONS=OFF")
endif()

ExternalProject_Add(
    libpng
    URL ${CMAKE_CURRENT_LIST_DIR}/libpng-1.6.47.tar.gz
    URL_HASH SHA256=084115c62fe023e3d88cd78764a4d8e89763985ee4b4a085825f7a00d85eafbb
    CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DPNG_SHARED=OFF ${DEPENDENCY_TOOLCHAIN_ARGS}
	DEPENDS libz
)
