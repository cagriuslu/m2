file(GLOB IMPL_SOURCES
        "include/impl/*.h"
        "include/impl/*/*.h"
        "include/impl/*/*/*.h"
        "src/*.h"   "src/*.cc"
        "src/*/*.h" "src/*/*.cc"
        "src/*/*/*.h" "src/*/*/*.cc")
set(IMPL_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

# Export vars to parent
set(IMPL_SOURCES "${IMPL_SOURCES}" PARENT_SCOPE)
set(IMPL_INCLUDE_DIRS "${IMPL_INCLUDE_DIRS}" PARENT_SCOPE)
