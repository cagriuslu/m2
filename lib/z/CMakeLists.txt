include(ExternalProject)

# When cross-compiling (e.g. Emscripten) forward the toolchain file into the nested ExternalProject build because
# ExternalProject does not inherit CMAKE_TOOLCHAIN_FILE automatically. Empty on native builds.
set(DEPENDENCY_TOOLCHAIN_ARGS)
if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND DEPENDENCY_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
endif()

ExternalProject_Add(
    libz
    URL ${CMAKE_CURRENT_LIST_DIR}/zlib-1.3.1.tar.gz
    URL_HASH SHA256=9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23
    CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${DEPENDENCY_TOOLCHAIN_ARGS} -DBUILD_SHARED_LIBS=OFF
)
