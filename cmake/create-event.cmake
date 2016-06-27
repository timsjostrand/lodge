#
# Helper for doing the glue code involved in binding events to a specific
# C-function for use in 'events.c'.
#
# Example:
#
# create_event("sound_play", "sound_play_buf_detailed")
#
# Will look up the function "sound_play_buf_detailed" in "sound.h" and produce
# the necessary glue code to bind it to an event called "sound_play" in
# "events_generated.c"
#
# Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
#

file(WRITE ${CMAKE_BINARY_DIR}/events-to-generate.cmake "")

#
# Queues an event with the specified 'event_name' bound to the C-function
# 'event_func' to be created once "events_generated.c" is built.
#
function(create_event event_name event_func)
	file(APPEND ${CMAKE_BINARY_DIR}/events-to-generate.cmake "${event_name};${event_func};")
endfunction()

add_custom_command(
	OUTPUT "events_generated.c" "events_generated.h"
	COMMAND ${CMAKE_COMMAND} -DHEADERS="${ENGINE_HEADERS}" -P ${CMAKE_CURRENT_LIST_DIR}/generate-events.cmake
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/generate-events.cmake ${CMAKE_BINARY_DIR}/events-to-generate.cmake
)

list(APPEND ENGINE_SOURCES "events_generated.c")
list(APPEND ENGINE_HEADERS "events_generated.h")
