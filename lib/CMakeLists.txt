add_subdirectory(box2d)
add_subdirectory(expected)
add_subdirectory(googletest)
add_subdirectory(protobuf)
add_subdirectory(SDL2)

add_subdirectory(z)
add_subdirectory(png)
add_subdirectory(SDL2_image)

add_subdirectory(freetype)
add_subdirectory(SDL2_ttf)

# Define the macro that will eventually build some of these dependencies
macro(BuildDependency dependency)
    # This macro uses execute_process to run cmake again because it needs to finish a successful run to generate the
    # build script. BUILDING_DEPS is set to ON to signal to the executed cmake process that the configuration should
    # return successfully, albeit prematurely.
    if(BUILDING_DEPS)
        return()
    else()
        message("Will build ${dependency}...")
        execute_process(
                COMMAND "${CMAKE_COMMAND}" -G ${CMAKE_GENERATOR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILDING_DEPS=ON -B ${CMAKE_BINARY_DIR} -S ${CMAKE_SOURCE_DIR}
                OUTPUT_FILE ${CMAKE_BINARY_DIR}/${dependency}-cmake.out
                ERROR_FILE ${CMAKE_BINARY_DIR}/${dependency}-cmake.err
                RESULT_VARIABLE EXIT_CODE)
        if(NOT EXIT_CODE EQUAL "0")
            message(FATAL_ERROR "Failed to configure ${dependency}")
        endif()
        execute_process(
                COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${dependency}
                OUTPUT_FILE ${CMAKE_BINARY_DIR}/${dependency}-build.out
                ERROR_FILE ${CMAKE_BINARY_DIR}/${dependency}-build.err
                RESULT_VARIABLE EXIT_CODE)
        if(NOT EXIT_CODE EQUAL "0")
            message(FATAL_ERROR "Failed to build ${dependency}")
        endif()
    endif()
endmacro()
