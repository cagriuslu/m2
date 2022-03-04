if(WIN32)
	add_library(SDL_gpu STATIC IMPORTED GLOBAL)
	set_target_properties(SDL_gpu PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/sdl-gpu-b21d5be/build/SDL_gpu-VS/lib/Release/SDL2_gpu_s.lib"
		IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/sdl-gpu-b21d5be/build/SDL_gpu-VS/lib/Release/SDL2_gpu_s.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/sdl-gpu-b21d5be/build/SDL_gpu-VS/include"
	)

	add_library(SDL_gpu-debug STATIC IMPORTED GLOBAL)
	set_target_properties(SDL_gpu-debug PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/sdl-gpu-b21d5be/build/SDL_gpu-VS/lib/Debug/SDL2_gpu_s.lib"
		IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/sdl-gpu-b21d5be/build/SDL_gpu-VS/lib/Debug/SDL2_gpu_s.lib"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/sdl-gpu-b21d5be/build/SDL_gpu-VS/include"
	)

	find_package(OpenGL REQUIRED)
	target_include_directories(SDL_gpu PUBLIC ${OPENGL_INCLUDE_DIR})
	target_include_directories(SDL_gpu-debug PUBLIC ${OPENGL_INCLUDE_DIR})
	target_link_libraries(SDL_gpu INTERFACE ${OPENGL_LIBRARIES})
	target_link_libraries(SDL_gpu-debug INTERFACE ${OPENGL_LIBRARIES})
else()

endif()
