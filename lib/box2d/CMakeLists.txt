include(FetchContent)
FetchContent_Declare(
    libbox2d
    URL ${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1.zip
    URL_HASH SHA256=0cb512dfa5be79ca227cd881b279adee61249c85c8b51caf5aa036b71e943002
)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE INTERNAL "Disable box2d unit tests")
set(BOX2D_BUILD_TESTBED OFF CACHE INTERNAL "Disable box2d testbed")
