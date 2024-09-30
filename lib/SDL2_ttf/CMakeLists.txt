# Point to the install location of freetype
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../freetype")

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/../SDL2/lib")
    message(WARNING "libSDL2 needs to be built first")
    return()
endif()
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../SDL2")

# ExternalProject cannot handle regular CMake lists, replace the list separator with some other separator
string (REPLACE ";" "|" MULTIPLE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

include(ExternalProject)
ExternalProject_Add(
    libSDL2_ttf
    URL ${CMAKE_CURRENT_LIST_DIR}/SDL2_ttf-2.20.2.zip
    URL_HASH SHA256=aa6256bfcffd8381a75b3a2a2384ac12109b5b148e72722a19b0ede54c4051dc
    LIST_SEPARATOR | # Specify the new separator to ExternalProject
    CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_PREFIX_PATH=${MULTIPLE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF -USDL2TTF_DEBUG_POSTFIX -DSDL2TTF_DEBUG_POSTFIX= -DSDL2TTF_SAMPLES=OFF
	DEPENDS libfreetype
)
