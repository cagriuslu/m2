if(NOT DEFINED GAME)
    # Set default game
    set(GAME "RPG")
endif()

# Descend into GAME's directory
add_subdirectory(${GAME})

# Export variables to parent
set(GAME_PROTO_FILES "${GAME_PROTO_FILES}" PARENT_SCOPE)
set(GAME_SOURCES "${GAME_SOURCES}" PARENT_SCOPE)
set(GAME_INCLUDE_DIRS "${GAME_INCLUDE_DIRS}" PARENT_SCOPE)
