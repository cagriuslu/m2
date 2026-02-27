# Build the C++ runtime library
include(ExternalProject)
ExternalProject_Add(
    genORM-cxx
    URL ${CMAKE_CURRENT_LIST_DIR}/genORM-adf3160.zip
    CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSQLITE_HEADER_DIR=${sqlite3_HEADER_DIR}
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
    OUTPUT ${CMAKE_BINARY_DIR}/m2.orm.h ${CMAKE_BINARY_DIR}/m2.orm.cc
    COMMAND cargo run -r --target-dir ${CMAKE_CURRENT_BINARY_DIR}/target -- ${CMAKE_SOURCE_DIR}/orm/M2.orm.json
    DEPENDS build_genORM ${CMAKE_SOURCE_DIR}/orm/M2.orm.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genORM-cxx-prefix/src/genORM-cxx
)
add_custom_target(build_orm_sources DEPENDS ${CMAKE_BINARY_DIR}/m2.orm.h ${CMAKE_BINARY_DIR}/m2.orm.cc)
