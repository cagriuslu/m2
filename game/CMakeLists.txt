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
set(IMPL_PROTO_FILES "${IMPL_PROTO_FILES}" PARENT_SCOPE)
set(IMPL_SOURCES "${IMPL_SOURCES}" PARENT_SCOPE)
set(IMPL_INCLUDE_DIRS "${IMPL_INCLUDE_DIRS}" PARENT_SCOPE)
