cmake_minimum_required(VERSION 3.20)

project(lodge VERSION 0.0.1 LANGUAGES C)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-build-flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-module.cmake)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
#set(CMAKE_MACOSX_RPATH 1)

# Check for compatibility.
include(CheckFunctionExists)
include(CheckSymbolExists)

#
# Find lodge-* modules.
#
list(APPEND ENGINE_MODULES_PATH "${CMAKE_CURRENT_LIST_DIR}/lodge-*/")
file(GLOB ENGINE_MODULES_AVAILABLE "${ENGINE_MODULES_PATH}")
foreach(module ${ENGINE_MODULES_AVAILABLE})
    get_filename_component(module_name "${module}" NAME)
    message(STATUS "Found lodge module: ${module_name}")
    add_subdirectory(${module} ${CMAKE_BINARY_DIR}/module/${module_name})
endforeach()

#
# Add a game library.
#
function(add_game_library game_name)
    # Create library.
    add_executable(${game_name} ${ARGN})

    # Compile with DEBUG flag?
    target_compile_definitions(${game_name} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

    # Link libraries.
    target_link_libraries(${game_name} lodge-build-flags)

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${game_name})

    set_target_properties(${game_name} PROPERTIES
    	VS_DEBUGGER_COMMAND_ARGUMENTS "--windowed"
    	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()
