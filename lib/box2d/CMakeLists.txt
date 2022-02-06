if(WIN32)
	add_library(box2d STATIC IMPORTED GLOBAL)
	set_target_properties(box2d PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1/build/bin/Release/box2d.lib"
		IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1/build/bin/Release/box2d.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1/include/box2d"
	)

	add_library(box2d-debug STATIC IMPORTED GLOBAL)
	set_target_properties(box2d-debug PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1/build/bin/Debug/box2d.lib"
		IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1/build/bin/Debug/box2d.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1/include/box2d"
	)
else()

endif()
