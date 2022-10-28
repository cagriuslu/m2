if (WIN32)
	set(SDL2_image_IMPORTED_LOCATION
		"${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/lib/x64/SDL2_image.dll"
		"${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/lib/x64/libjpeg-9.dll"
		"${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/lib/x64/libpng16-16.dll"
		"${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/lib/x64/libtiff-5.dll"
		"${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/lib/x64/libwebp-7.dll"
		"${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/lib/x64/zlib1.dll"
	)

	add_library(SDL2_image SHARED IMPORTED GLOBAL)
	set_target_properties(SDL2_image PROPERTIES
		IMPORTED_LOCATION "${SDL2_image_IMPORTED_LOCATION}"
		IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/lib/x64/SDL2_image.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.0.5/include"
	)
else()
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
	string (REPLACE ";" ":" MULTIPLE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

	include(ExternalProject)
	ExternalProject_Add(
		libSDL2_image
		URL ${CMAKE_CURRENT_LIST_DIR}/SDL2_image-2.6.2.zip
		LIST_SEPARATOR : # Specify the new separator to ExternalProject
		CMAKE_ARGS -DCMAKE_PREFIX_PATH=${MULTIPLE_PREFIX_PATH} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSDL2IMAGE_SAMPLES=OFF -DSDL2IMAGE_JPG=OFF
		BUILD_COMMAND make -j${PROCESSOR_COUNT}
	)
endif()
