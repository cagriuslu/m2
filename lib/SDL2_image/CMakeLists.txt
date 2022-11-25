if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/../png/lib")
    message(WARNING "libpng needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../png")

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/../SDL2/lib")
    message(WARNING "libSDL2 needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../SDL2")

# ExternalProject cannot handle regular CMake lists, replace the list separator with some other separator
string (REPLACE ";" "|" MULTIPLE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

include(ExternalProject)
ExternalProject_Add(
    libSDL2_image
    URL ${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.6.2.zip
    URL_HASH SHA256=efe3c229853d0d40c35e5a34c3f532d5d9728f0abc623bc62c962bcef8754205
    LIST_SEPARATOR | # Specify the new separator to ExternalProject
    CMAKE_ARGS -DCMAKE_PREFIX_PATH=${MULTIPLE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -USDL2IMAGE_DEBUG_POSTFIX -DSDL2IMAGE_DEBUG_POSTFIX= -DSDL2IMAGE_SAMPLES=OFF -DSDL2IMAGE_JPG=OFF
)
