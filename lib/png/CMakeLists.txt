if(WIN32)

else()
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/../z/lib")
        message(WARNING "libz needs to be built first")
        return()
    endif()
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../z")

    include(ExternalProject)
    ExternalProject_Add(
        libpng
        URL ${CMAKE_CURRENT_LIST_DIR}/libpng-1.6.38.tar.gz
        CMAKE_ARGS -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        BUILD_COMMAND make -j${PROCESSOR_COUNT}
    )
endif()
