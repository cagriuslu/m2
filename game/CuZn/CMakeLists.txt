file(GLOB GAME_PROTO_FILES "pb/*.proto")

file(GLOB GAME_SOURCES
        "include/m2g/*.h"
        "include/cuzn/*.h"
        "src/*.h"   "src/*.cc"
        "src/*/*.h" "src/*/*.cc"
        "src/*/*/*.h" "src/*/*/*.cc")

set(GAME_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

# Export vars to parent
set(GAME_PROTO_FILES "${GAME_PROTO_FILES}" PARENT_SCOPE)
set(GAME_SOURCES "${GAME_SOURCES}" PARENT_SCOPE)
set(GAME_INCLUDE_DIRS "${GAME_INCLUDE_DIRS}" PARENT_SCOPE)
