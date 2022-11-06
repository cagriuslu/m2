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
	include(ExternalProject)
	ExternalProject_Add(
		libbox2d
		URL ${CMAKE_CURRENT_LIST_DIR}/box2d-2.4.1.zip
		URL_HASH SHA256=0cb512dfa5be79ca227cd881b279adee61249c85c8b51caf5aa036b71e943002
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBOX2D_BUILD_UNIT_TESTS=OFF -DBOX2D_BUILD_TESTBED=OFF
		BUILD_COMMAND make -j${PROCESSOR_COUNT}
	)

	add_library(box2d STATIC IMPORTED GLOBAL)
	add_dependencies(box2d libbox2d)
	set_target_properties(box2d PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/lib/libbox2d.a"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/include"
	)
endif()
