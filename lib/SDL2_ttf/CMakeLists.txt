if (WIN32)
	add_library(SDL2_ttf SHARED IMPORTED GLOBAL)
	set_target_properties(SDL2_ttf PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/SDL2_ttf-2.0.18/lib/x64/SDL2_ttf.dll"
		IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/SDL2_ttf-2.0.18/lib/x64/SDL2_ttf.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/SDL2_ttf-2.0.18/include"
	)
else()
	# Not yet implemented
endif()
