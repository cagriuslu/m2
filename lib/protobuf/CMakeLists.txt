include(ExternalProject)

# When cross-compiling (e.g. Emscripten) forward the toolchain file into the nested ExternalProject build because
# ExternalProject does not inherit CMAKE_TOOLCHAIN_FILE automatically. Empty on native builds. For wasm we additionally
# turn off protobuf_BUILD_PROTOC_BINARIES: a wasm protoc is useless, and the host protoc from a prior native build is
# reused instead.
set(LIBPROTOBUF_TOOLCHAIN_ARGS)
if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND LIBPROTOBUF_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}" "-Dprotobuf_BUILD_PROTOC_BINARIES=OFF")
endif()

ExternalProject_Add(
        libprotobuf
        URL ${CMAKE_CURRENT_LIST_DIR}/protobuf-cpp-3.21.5.zip
        URL_HASH SHA256=5160e1ea2485cba6329d1350d560d3dbb9246aba91a46d076ddd971687df253f
        CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${LIBPROTOBUF_TOOLCHAIN_ARGS} -DBUILD_SHARED_LIBS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_MSVC_STATIC_RUNTIME=OFF -Dprotobuf_DEBUG_POSTFIX= -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_WITH_ZLIB=OFF
)
