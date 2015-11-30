#
# Generate an empty project in the current directory.
#
# Call with:
# > cmake -DGAME_NAME="my_game" -P generate-project.cmake
#
# Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
#

if(DEFINED GAME_NAME)
	set(ENGINE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../)
	set(GAME_PATH ${GAME_NAME})
	string(TIMESTAMP YEAR "%Y")

	# Copy template project.
	file(COPY ${ENGINE_PATH}/empty-project/ DESTINATION ${GAME_NAME})

	# Replace tokens in template project.
	file(GLOB_RECURSE GAME_FILES ${GAME_PATH}/*.c ${GAME_PATH}/*.h
		${GAME_PATH}/*.txt ${GAME_PATH}/*.json)

	# Perform variable substitution.
	foreach(game_file ${GAME_FILES})
		message("Preparing ${game_file}...")
		configure_file(${game_file} ${game_file})
	endforeach()

	message("Project ${GAME_NAME} generated")
else()
	message("GAME_NAME not set")
endif()

