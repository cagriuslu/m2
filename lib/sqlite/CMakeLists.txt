include(FetchContent)
FetchContent_Declare(
    sqlite3_source
    URL ${CMAKE_CURRENT_LIST_DIR}/sqlite-amalgamation-3510200.zip
    URL_HASH SHA256=6e2a845a493026bdbad0618b2b5a0cf48584faab47384480ed9f592d912f23ec
)

# Sqlite is not a proper CMake project. We need to build the library ourselves.

# Query the properties of the content
FetchContent_GetProperties(sqlite3_source)
# If the content isn't fetched
if(NOT sqlite3_source_POPULATED)
    # Fetch the content
    FetchContent_MakeAvailable(sqlite3_source)
    # Create a library using the only source file
    add_library(sqlite3 STATIC ${sqlite3_source_SOURCE_DIR}/sqlite3.c)
    # The headers reside in the same directory
    target_include_directories(sqlite3 PUBLIC ${sqlite3_source_SOURCE_DIR})
endif()

# List the header location one level up
set(sqlite3_HEADER_DIR "${sqlite3_source_SOURCE_DIR}" PARENT_SCOPE)
