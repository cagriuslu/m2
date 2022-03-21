if(NOT DEFINED GAME)
    # Set default game
    set(GAME "RPG")
endif()

# Descend into GAME's directory
if(${GAME} MATCHES "PLAT")
    add_subdirectory(PLAT)
elseif(${GAME} MATCHES "RPG")
    add_subdirectory(RPG)
elseif(${GAME} MATCHES "RTS")
    add_subdirectory(RTS)
else()
    message("${GAME} not implemented")
endif()

# Export variables to parent
set(GAME_SOURCES "${GAME_SOURCES}" PARENT_SCOPE)
set(GAME_INCLUDE_DIRS "${GAME_INCLUDE_DIRS}" PARENT_SCOPE)
