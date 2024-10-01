include(FetchContent)
FetchContent_Declare(
    libbox2d
    URL ${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.2.zip
    URL_HASH SHA256=593f165015fdd07ea521a851105f1c86ae313c5af0a15968ed95f864417fa8a7
)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE INTERNAL "Disable box2d unit tests")
set(BOX2D_BUILD_TESTBED OFF CACHE INTERNAL "Disable box2d testbed")
