file(GLOB IMPL_PROTO_FILES "pb/*.proto")

file(GLOB IMPL_SOURCES
        "include/m2g/*.h"
        "include/m2g/*/*.h"
        "include/rpg/*.h"
        "include/rpg/*/*.h"
        "include/rpg/*/*/*.h"
        "src/*.h"   "src/*.cc"
        "src/*/*.h" "src/*/*.cc"
        "src/*/*/*.h" "src/*/*/*.cc")

set(IMPL_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

# Export vars to parent
set(IMPL_PROTO_FILES "${IMPL_PROTO_FILES}" PARENT_SCOPE)
set(IMPL_SOURCES "${IMPL_SOURCES}" PARENT_SCOPE)
set(IMPL_INCLUDE_DIRS "${IMPL_INCLUDE_DIRS}" PARENT_SCOPE)
