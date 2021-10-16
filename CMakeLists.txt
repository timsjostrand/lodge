project(lodge C)

cmake_minimum_required(VERSION 3.20)

include(${ENGINE_PATH}/cmake/lodge-build-flags.cmake)
include(${ENGINE_PATH}/cmake/lodge-module.cmake)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# Compile options
option(ENABLE_CONSOLE "Compile with console support" ON)
option(ENABLE_SHARED "Enable game hotswapping" ON)

# Check for compatibility.
include(CheckFunctionExists)
include(CheckSymbolExists)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#    CACHE STRING "Static Library output directory" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    CACHE STRING "Shared Library output directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    CACHE STRING "Executable output directory" FORCE)

if(ENABLE_SHARED)
    set(BUILD_SHARED_LIBS On)
endif()

# Library: zlib.
set(SKIP_INSTALL_ALL ON)
set(ZLIB_INCLUDE_DIR "${ENGINE_PATH}/lib/zlib" "${CMAKE_BINARY_DIR}/lib/zlib")
if(ENABLE_SHARED)
    set(ZLIB_LIBRARY zlib)
else()
    set(ZLIB_LIBRARY zlibstatic)
endif()
add_subdirectory(${ENGINE_PATH}/lib/zlib ${CMAKE_BINARY_DIR}/lib/zlib)
target_include_directories(${ZLIB_LIBRARY} PUBLIC "${ZLIB_INCLUDE_DIR}")

#
# Find lodge-* modules.
#
list(APPEND ENGINE_MODULES_PATH "${ENGINE_PATH}/lodge-*/")
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

    if(ENABLE_SHARED)
        # Need HACK_POST_BUILD for Visual Studio 2013.
        if(MSVC12)
            set(HACK_POST_BUILD 1)
        endif()

        # Output shared library into asset path.
        if(HACK_POST_BUILD)
            message("With HACK_POST_BUILD")
            # HACK: On some platforms, this method works best for hotswapping the
            # shared library.
            set_target_properties(${game_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${ASSET_PATH}"
                LIBRARY_OUTPUT_DIRECTORY "${ASSET_PATH}")
        else()
            add_custom_command(TARGET ${game_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${game_name}>" ${ASSET_PATH}
                COMMENT "Copying ${game_name} shared library to ASSET_PATH")
        endif()
    endif()

    # FIXME(TS): remove `ENABLE_CONSOLE` in favor of module enable definition
    # Compile with console output support?
    if(ENABLE_CONSOLE)
        target_compile_definitions(${game_name} PRIVATE CONSOLE_ENABLE)
    endif()

    # Compile as shared library?
    if(ENABLE_SHARED)
        target_compile_definitions(${game_name} PRIVATE ENABLE_SHARED)
    endif()

    # Compile with DEBUG flag?
    target_compile_definitions(${game_name} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

    # Link libraries.
    target_link_libraries(${game_name} lodge-build-flags)

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${game_name})

    set_target_properties(${game_name} PROPERTIES
    	VS_DEBUGGER_COMMAND_ARGUMENTS "--windowed --mount=${ASSET_PATH}"
    	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    
#[==[
    if(ENABLE_SHARED)
        set(ENGINE_GAME_LIBRARY "")

        # Convenience target to compile and run the engine executable with the same
        # asset path as in cmake_user.
        add_custom_target(run
            COMMAND engine --windowed --mount "${ASSET_PATH}" --game "$<TARGET_FILE_NAME:${game_name}>"
            DEPENDS ${game_name} engine)
    else()
        set(ENGINE_GAME_LIBRARY "${game_name}")

        # Convenience target to compile and run the engine executable with the same
        # asset path as in cmake_user.
        add_custom_target(run
            COMMAND engine --windowed --mount "${ASSET_PATH}"
            DEPENDS ${game_name} engine)
    endif()

    # Main executable.
    add_executable(engine)
    set_property(TARGET engine PROPERTY RUNTIME_OUTPUT_NAME "${game_name}")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)

    # Add DEBUG define when building debug executables.
    if(ENABLE_SHARED)
        target_compile_definitions(engine PRIVATE LOAD_SHARED)
    endif()
    target_compile_definitions(engine PRIVATE $<$<CONFIG:Debug>:DEBUG>)

    # Console capture stdout support
    # FIXME(TS): remove this define
    if(ENABLE_CONSOLE)
        target_compile_definitions(engine PRIVATE CONSOLE_ENABLE)
    endif()

    # GCC compiler settings.
    if(CMAKE_COMPILER_IS_GNUCC)
        # DEBUG: Lightweight buffer overflow detection.
        target_compile_options(engine PRIVATE $<$<CONFIG:Debug>:-O2>)
        target_compile_definitions(engine PRIVATE $<$<CONFIG:Debug>:_FORTIFY_SOURCE=2>)
    endif()

    # Link.
    target_link_libraries(engine ${ENGINE_GAME_LIBRARY})
#]==]
endfunction()
