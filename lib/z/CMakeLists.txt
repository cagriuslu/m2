if(WIN32)

else()
    include(ExternalProject)
    ExternalProject_Add(
        libz
        URL ${CMAKE_CURRENT_LIST_DIR}/zlib-1.2.13.tar.gz
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        BUILD_COMMAND make -j${PROCESSOR_COUNT}
    )
endif()
