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
	# Not yet implemented
endif()
