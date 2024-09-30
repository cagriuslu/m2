include(FetchContent)
FetchContent_Declare(
    libexpected
    URL ${CMAKE_CURRENT_LIST_DIR}/expected-1.1.0.tar.gz
    URL_HASH SHA256=1db357f46dd2b24447156aaf970c4c40a793ef12a8a9c2ad9e096d9801368df6
)
set(EXPECTED_BUILD_PACKAGE OFF CACHE INTERNAL "Disable expected packages")
set(EXPECTED_BUILD_TESTS OFF CACHE INTERNAL "Disable expected testing")
