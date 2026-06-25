# Point to the install locations of freetype and SDL3
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../freetype")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../SDL3")

# ExternalProject cannot handle regular CMake lists, replace the list separator with some other separator
string (REPLACE ";" "|" MULTIPLE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

include(ExternalProject)
# Freetype is consumed from our own static build via find_package (SDLTTF_VENDORED=OFF). Harfbuzz and plutosvg
# are disabled: we don't vendor them, and the previous SDL2_ttf build didn't depend on them either.
ExternalProject_Add(
    libSDL3_ttf
    URL ${CMAKE_CURRENT_LIST_DIR}/SDL3_ttf-3.2.2.zip
    URL_HASH SHA256=d38c2078630e015777aafa1a1ce627df4323114a920c313274346c372ba0d19d
    LIST_SEPARATOR | # Specify the new separator to ExternalProject
    CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_PREFIX_PATH=${MULTIPLE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF -DSDLTTF_VENDORED=OFF -DSDLTTF_SAMPLES=OFF -DSDLTTF_HARFBUZZ=OFF -DSDLTTF_PLUTOSVG=OFF
	DEPENDS libfreetype libSDL3
)
