if (WIN32)
    add_library(googlemock SHARED IMPORTED GLOBAL)
    set_target_properties(googlemock PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/bin/gmock.dll"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/lib/gmock.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/include"
            )
    add_library(googlemock_main SHARED IMPORTED GLOBAL)
    set_target_properties(googlemock_main PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/bin/gmock_main.dll"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/lib/gmock_main.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/include"
            )
    add_library(googletest SHARED IMPORTED GLOBAL)
    set_target_properties(googletest PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/bin/gtest.dll"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/lib/gtest.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/include"
            )
    add_library(googletest_main SHARED IMPORTED GLOBAL)
    set_target_properties(googletest_main PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/bin/gtest_main.dll"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/lib/gtest_main.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/googletest-release-1.12.1/build/install/include"
            )
    target_link_libraries(googletest_main INTERFACE "googletest;googlemock_main;googlemock")
else()

endif()
