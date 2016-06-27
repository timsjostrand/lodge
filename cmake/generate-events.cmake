#
# Reads the file "events-to-generate.cmake" and generates
# "events_generated.{c,h}".
#
# Must be called with -DHEADERS="header1.h header2.h" for all headers in the
# project
#
# Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
#

#
# Any symbol type matching this regex will use memcpy instead of regular
# assignment when copying values to the event data struct.
#
set(MEMCPY_TYPES "(^vec[0-9]$)|(^struct[a-zA-Z0-9_ ]*$)")

# Sanity check.
if("${HEADERS}" STREQUAL "")
	message(FATAL_ERROR "generate-events.cmake called without -DHEADERS")
endif()

# Convert space separated list to cmake list.
string(REPLACE " " ";" HEADERS "${HEADERS}")

#
# Takes a declaration of a variable and returns the type and name.
#
# Example:
# get_type_and_name("struct example *ex" TYPE NAME)
# => TYPE="struct example *" and NAME="ex"
#
function(get_type_and_name arg out_type out_name)
	string(REPLACE "*" "* " TMP "${arg}")
	string(REPLACE " " ";" TMP "${TMP}")
	list(REVERSE TMP)
	# Name
    list(GET TMP "0" TMP_LAST)
    set("${out_name}" "${TMP_LAST}" PARENT_SCOPE)
	# Type
	list(REMOVE_AT TMP 0)
	list(REVERSE TMP)
	string(REPLACE ";" " " TMP "${TMP}")
	set("${out_type}" "${TMP}" PARENT_SCOPE)
endfunction()

#
# Write the preamble to 'events_generated.c'.
#
function(write_pre events)
	# ID_STRUCT
	foreach(event ${events})
		set(ID_STRUCT "${ID_STRUCT}\tunsigned int ${event};\n")
	endforeach()

	# REGISTER_GENERATED
	foreach(event ${events})
		set(REGISTER_GENERATED "${REGISTER_GENERATED}\tevents_register_${event}(events);\n")
	endforeach()

	configure_file("${CMAKE_CURRENT_LIST_DIR}/event-pre.template" ${CMAKE_BINARY_DIR}/event-pre.tmp @ONLY)
endfunction()

#
# Write the glue C code for a function with known arguments.
#
function(write_event event_name event_func)
	string(REPLACE "\n" "" event_name "${event_name}")
	string(REPLACE "\n" "" event_func "${event_func}")

	get_func_args(${event_func} header_file event_args)

	if("${header_file}" STREQUAL "")
		message(FATAL_ERROR "Could not find header for C-function '${event_func}' (looked in: ${HEADERS})")
	endif()

    # ARGS_TO_SEND
    set(ARGS_TO_SEND "${event_args}")
	list(INSERT ARGS_TO_SEND 0 "struct events *events")
	string(REPLACE "const " "" ARGS_TO_SEND "${ARGS_TO_SEND}")
    string(REPLACE ";" ", " ARGS_TO_SEND "${ARGS_TO_SEND}")

    # STRUCT_MEMBERS
    foreach(arg ${event_args})
        string(REPLACE "const " "" arg "${arg}")
        string(CONCAT STRUCT_MEMBERS "${STRUCT_MEMBERS}" "\t${arg};\n")
    endforeach()

    # ARGS_FROM_STRUCT
    set(ARGS_FROM_STRUCT "")
    list(LENGTH event_args event_args_length)
    foreach(arg ${event_args})
		get_type_and_name("${arg}" SYMTYPE SYMNAME)
        string(CONCAT ARGS_FROM_STRUCT "${ARGS_FROM_STRUCT}" "d->${SYMNAME}")

        math(EXPR event_args_length "${event_args_length} - 1")
        if("${event_args_length}" GREATER "0")
            string(CONCAT ARGS_FROM_STRUCT "${ARGS_FROM_STRUCT}" ", ")
        endif()
    endforeach()

    # ARGS_TO_STRUCT
    set(ARGS_TO_STRUCT "")
    foreach(arg ${event_args})
		get_type_and_name("${arg}" SYMTYPE SYMNAME)

		if(SYMTYPE MATCHES "${MEMCPY_TYPES}")
			string(CONCAT ARGS_TO_STRUCT "${ARGS_TO_STRUCT}" "\tmemcpy(d->${SYMNAME}, ${SYMNAME}, sizeof(${SYMTYPE}));\n")
		else()
	        string(CONCAT ARGS_TO_STRUCT "${ARGS_TO_STRUCT}" "\td->${SYMNAME} = ${SYMNAME};\n")
		endif()
    endforeach()

	set(HEADER_DECLARATIONS "${HEADER_DECLARATIONS}\
/* Event: ${event_name} */\n\
void events_send_${event_name}(${ARGS_TO_SEND});\n\
void events_register_${event_name}(struct events *events);\n\n" PARENT_SCOPE)

	# Used for REQUIRED_INCLUDES
	list(APPEND REQUIRED_INCLUDES_LIST "${header_file}")
	set(REQUIRED_INCLUDES_LIST "${REQUIRED_INCLUDES_LIST}" PARENT_SCOPE)

    set(NAME "${event_name}")
    set(EVENT_FUNCTION "${event_func}")

	#message(STATUS "EVENT_FUNCTION=${EVENT_FUNCTION}")
	#message(STATUS "ARGS_TO_SEND=${ARGS_TO_SEND}")
	#message(STATUS "ARGS_FROM_STRUCT=${ARGS_FROM_STRUCT}")
	#message(STATUS "STRUCT_MEMBERS=${STRUCT_MEMBERS}")
	#message(STATUS "ARGS_TO_STRUCT=${ARGS_TO_STRUCT}")

	# Write from template and append to "events.tmp"
	configure_file("${CMAKE_CURRENT_LIST_DIR}/event.template" ${CMAKE_BINARY_DIR}/event.tmp @ONLY)
	file(READ ${CMAKE_BINARY_DIR}/event.tmp EVENT_TMP)
	file(APPEND ${CMAKE_BINARY_DIR}/events.tmp "${EVENT_TMP}")
	file(REMOVE ${CMAKE_BINARY_DIR}/event.tmp)
endfunction()

#
# Find the header file and arguments for the specified C-function and store
# them in the variables named by out_header and out_args, respectively.
#
function(get_func_args func out_header out_args)
	# Find the function declaration in some header.
	foreach(header ${HEADERS})
        file(READ "${header}" HEADER_TXT)
        string(REGEX MATCH "^.*${event_func}\\(([^;]*)\\);.*" event_func_args "${HEADER_TXT}")
        set(event_func_args "${CMAKE_MATCH_1}")

        # Break if we found the function declaration.
		if(NOT "${CMAKE_MATCH_0}" STREQUAL "")
			# Strip of newlines
			string(REPLACE "\n" "" event_func_args "${event_func_args}")
			# Strip of whitespace
			string(REGEX REPLACE ",[ \t]+" "," event_func_args "${event_func_args}")
			# Make into a cmake list
			string(REPLACE "," ";" event_func_args "${event_func_args}")

			# Export to parent scope
			set("${out_header}" "${header}" PARENT_SCOPE)
			set("${out_args}" "${event_func_args}" PARENT_SCOPE)
   			break()
		endif()
	endforeach()
endfunction()

#
# Combine "event-pre.tmp", "events.tmp" into "events_generated.c"
#
function(combine_pre_and_events)
	file(READ "${CMAKE_BINARY_DIR}/event-pre.tmp" PRE_TMP)
	file(READ "${CMAKE_BINARY_DIR}/events.tmp" EVENTS_TMP)
	file(WRITE "${CMAKE_BINARY_DIR}/events_generated.c" "${PRE_TMP}${EVENTS_TMP}")
	file(REMOVE "${CMAKE_BINARY_DIR}/event-pre.tmp")
	file(REMOVE "${CMAKE_BINARY_DIR}/events.tmp")
endfunction()

function(write_header)
	# REQUIRED_INCLUDES
	list(REMOVE_DUPLICATES REQUIRED_INCLUDES_LIST)
	foreach(include_file ${REQUIRED_INCLUDES_LIST})
		set(REQUIRED_INCLUDES "${REQUIRED_INCLUDES}#include \"${include_file}\"\n")
	endforeach()

	configure_file("${CMAKE_CURRENT_LIST_DIR}/event-header.template" ${CMAKE_BINARY_DIR}/events_generated.h @ONLY)
endfunction()

function(generate_events)
	file(READ "${CMAKE_BINARY_DIR}/events-to-generate.cmake" EVENTS_TO_GEN)

	# Read all event names to generate into "EVENT_NAMES".
	set(IS_NAME true)
	foreach(tmp ${EVENTS_TO_GEN})
		if(IS_NAME)
			set(IS_NAME false)
			string(REPLACE "\n" "" tmp "${tmp}")
			list(APPEND EVENT_NAMES ${tmp})
		else()
			set(IS_NAME true)
		endif()
	endforeach()

	# Read tuples of (event_name, event_func) from the events-to-generate file.
	# Because cmake is a silly language there is no such built-in functionality.
	set(IS_NAME true)
	foreach(tmp ${EVENTS_TO_GEN})
		if(IS_NAME)
			set(IS_NAME false)
			set(EVENT_NAME ${tmp})
		else()
			set(IS_NAME true)
			set(EVENT_FUNC ${tmp})
			message(STATUS "Generating event \"${EVENT_NAME}\" for function \"${EVENT_FUNC}\"...")
			write_event(${EVENT_NAME} ${EVENT_FUNC})
		endif()
	endforeach()

	write_pre("${EVENT_NAMES}")

	# Generate "events_generated.h"
	write_header()

	combine_pre_and_events()
endfunction()

generate_events()
