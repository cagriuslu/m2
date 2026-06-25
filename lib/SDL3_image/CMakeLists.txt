# Point to the install locations of png, z and SDL3
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../png")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../z")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../SDL3")

# ExternalProject cannot handle regular CMake lists, replace the list separator with some other separator
string (REPLACE ";" "|" MULTIPLE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

include(ExternalProject)
# SDL3_image loads PNG through its bundled stb_image / ImageIO backend (no external libpng linkage), so the
# formats that would require external libraries (AVIF, WEBP, TIFF, JPEG, JXL) are disabled. This keeps the
# build self-contained and avoids duplicate libpng symbols with the engine's own libpng.
ExternalProject_Add(
    libSDL3_image
    URL ${CMAKE_CURRENT_LIST_DIR}/SDL3_image-3.2.6.zip
    URL_HASH SHA256=4108ac9bdea527dbf39f104646854ec20b0b38ccab7b2015e610cd3bb6a1ba2d
    LIST_SEPARATOR | # Specify the new separator to ExternalProject
    CMAKE_ARGS -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_PREFIX_PATH=${MULTIPLE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF -DSDLIMAGE_VENDORED=OFF -DSDLIMAGE_SAMPLES=OFF -DSDLIMAGE_TESTS=OFF -DSDLIMAGE_JPG=OFF -DSDLIMAGE_AVIF=OFF -DSDLIMAGE_WEBP=OFF -DSDLIMAGE_TIF=OFF -DSDLIMAGE_JXL=OFF
	DEPENDS libpng libSDL3
)
