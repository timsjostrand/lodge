#
# Add a game library.
#
function(lodge_add_executable game_name)
    # Create library.
    add_executable(${game_name} ${ARGN})

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${game_name})

    set_target_properties(${game_name} PROPERTIES
    	VS_DEBUGGER_COMMAND_ARGUMENTS "--windowed"
    	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()
