# Build the C++ runtime library
include(ExternalProject)

# When cross-compiling (e.g. Emscripten) forward the toolchain file into the nested ExternalProject build because
# ExternalProject does not inherit CMAKE_TOOLCHAIN_FILE automatically. Empty on native builds.
# The genORM *tool* deliberately stays host-native: it only generates portable C++ (M2.orm.{h,cc}).
set(DEPENDENCY_TOOLCHAIN_ARGS)
if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND DEPENDENCY_TOOLCHAIN_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        # The Emscripten toolchain restricts find_package to its sysroot. Relax it so any sibling
        # dependency installed under build/<game>-WASM/lib remains discoverable via CMAKE_PREFIX_PATH.
        "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH"
        "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH"
        "-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH")
endif()

ExternalProject_Add(
    genORM-cxx
    URL ${CMAKE_CURRENT_LIST_DIR}/genORM-adf3160.zip
    CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${DEPENDENCY_TOOLCHAIN_ARGS} -DSQLITE_HEADER_DIR=${sqlite3_HEADER_DIR}
)

# Build the genORM tool
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/target/release/genORM
    COMMAND cargo build -r --target-dir ${CMAKE_CURRENT_BINARY_DIR}/target
    DEPENDS genORM-cxx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genORM-cxx-prefix/src/genORM-cxx
)
add_custom_target(build_genORM DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/target/release/genORM)

# Generate the ORM sources
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/M2.orm.h ${CMAKE_BINARY_DIR}/M2.orm.cc
    COMMAND cargo run -r --target-dir ${CMAKE_CURRENT_BINARY_DIR}/target -- ${CMAKE_SOURCE_DIR}/orm/M2.orm.json
    DEPENDS build_genORM ${CMAKE_SOURCE_DIR}/orm/M2.orm.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genORM-cxx-prefix/src/genORM-cxx
)
add_custom_target(build_orm_sources DEPENDS ${CMAKE_BINARY_DIR}/M2.orm.h ${CMAKE_BINARY_DIR}/M2.orm.cc)
