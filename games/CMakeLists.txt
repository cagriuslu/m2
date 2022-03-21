if(NOT DEFINED GAME)
    # Default game
    set(GAME "ARPG")
endif()

if(${GAME} MATCHES "APLAT")
    message("Game not implemented")
elseif(${GAME} MATCHES "ARPG")
    file(GLOB GAME_SOURCES
            "ARPG/*.hh" "ARPG/*.cc"
            "ARPG/*/*.hh" "ARPG/*/*.cc")
    set(GAME_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/ARPG/public")
elseif(${GAME} MATCHES "PLAT")
    message("Game not implemented")
elseif(${GAME} MATCHES "RPG")
    message("Game not implemented")
elseif(${GAME} MATCHES "RTS")
    message("Game not implemented")
else()
    message("Game not implemented")
endif()

# Export vars to parent
set(GAME_SOURCES "${GAME_SOURCES}" PARENT_SCOPE)
set(GAME_INCLUDE_DIRS "${GAME_INCLUDE_DIRS}" PARENT_SCOPE)
