project(engine)

cmake_minimum_required(VERSION 3.1)

# Include local settings.
include(cmake_user.txt)
message(STATUS "GAME_PATH=${GAME_PATH}")
message(STATUS "ASSET_PATH=${ASSET_PATH}")

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# Compile options
option(ENABLE_CONSOLE "Compile with console support" ON)
option(ENABLE_SHARED "Enable game hotswapping" ON)

# QUIRK: Define M_PI on Windows.
add_definitions(-D_USE_MATH_DEFINES)

# Enable asset hotswap.
add_definitions(-DVFS_ENABLE_FILEWATCH)

# Check for compatibility.
include(CheckFunctionExists)
check_function_exists(strnlen HAVE_STRNLEN)

set(ENGINE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(ENGINE_INCLUDES ${ENGINE_PATH})

if(NOT EMSCRIPTEN)
    # Build GLFW.
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
    add_subdirectory(lib/glfw)
    list(APPEND ENGINE_INCLUDES lib/glfw/include)
    list(APPEND ENGINE_EXTRA_LIBS glfw ${GLFW_LIBRARIES})

    # Build glew.
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "Build the shared glew library")
    add_subdirectory(lib/glew)
    list(APPEND ENGINE_INCLUDES lib/glew/include)
    add_definitions(-DGLEW_STATIC)
    list(APPEND ENGINE_EXTRA_LIBS libglew_static ${GL_LIBRARY}
        ${PLATFORM_SPECIFIC_LIBRARIES})

    # Build cJSON.
    add_subdirectory(lib/cjson)
    list(APPEND ENGINE_EXTRA_LIBS cJSON)

    if(NOT APPLE)
        # Build OpenAL-Soft.
        set(LIBTYPE "STATIC" CACHE STRING "Build static library")
        set(ALSOFT_UTILS OFF CACHE BOOL "Build and install utility programs")
        set(ALSOFT_NO_CONFIG_UTIL OFF CACHE BOOL "Disable building the alsoft-config utility")
        set(ALSOFT_EXAMPLES OFF CACHE BOOL "Build and install example programs")
        set(ALSOFT_CONFIG OFF CACHE BOOL "Install alsoft.conf sample configuration file")
        set(ALSOFT_HRTF_DEFS OFF CACHE BOOL "Install HRTF definition files")
        set(ALSOFT_INSTALL OFF CACHE BOOL "Install headers and libraries")
        add_subdirectory(lib/openal)
        list(APPEND ENGINE_INCLUDES lib/openal/include/AL)

        # OpenAL has different names on different platforms.
        if(WIN32)
            list(APPEND ENGINE_EXTRA_LIBS OpenAL32)
        else()
            list(APPEND ENGINE_EXTRA_LIBS openal)
        endif()
    else()
        # Use the OS X OpenAL framework.
        find_library(OPENAL_LIBRARY OpenAL)
        find_path(OPENAL_INCLUDE_DIR al.h)
        list(APPEND ENGINE_INCLUDES ${OPENAL_INCLUDE_DIR})
        list(APPEND ENGINE_EXTRA_LIBS ${OPENAL_LIBRARY})
    endif()
else()
    # glfw and glew is provided by emscripten
    option(USE_GLFW_3 "Use glfw3" ON)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file test_assets/test.png")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file test_assets/vivaldi.ogg")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file test_assets/paddle.png")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=100000000")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-warn-absolute-paths")
endif()

# Include stb.h
list(APPEND ENGINE_INCLUDES lib/)

set(ENGINE_SOURCES_LOCAL math4.c core_console.c graphics.c shader.c input.c texture.c
        color.c sound.c vfs.c atlas.c monotext.c str.c list.c console.c
        spritebatch.c animatedsprites.c alist.c core.c core_argv.c core_reload.c
        particles.c)
set(ENGINE_HEADERS_LOCAL math4.h core_console.h graphics.h shader.h input.h texture.h
        color.h sound.h vfs.h atlas.h monotext.h str.h list.h console.h
        spritebatch.h animatedsprites.h alist.h core.h core_argv.h core_reload.h
        particles.h game.h)

# Top-down sources
set(ENGINE_SOURCES_TOP_DOWN top-down/tiles.c)
set(ENGINE_HEADERS_TOP_DOWN top-down/tiles.h)

list(APPEND ENGINE_SOURCES_LOCAL ${ENGINE_SOURCES_TOP_DOWN})
list(APPEND ENGINE_HEADERS_LOCAL ${ENGINE_HEADERS_TOP_DOWN})

# Prepend all sources with absolute engine path (because of reasons...)
set(ENGINE_SOURCES "")
foreach(src ${ENGINE_SOURCES_LOCAL})
    list(APPEND ENGINE_SOURCES ${ENGINE_PATH}/${src})
endforeach()
set(ENGINE_HEADERS "")
foreach(src ${ENGINE_HEADERS_LOCAL})
    list(APPEND ENGINE_HEADERS ${ENGINE_PATH}/${src})
endforeach()

# Include all source directories
include_directories(${ENGINE_INCLUDES})

# Asset generator executable
add_executable(generate_assets ${ENGINE_PATH}/generate_assets.c ${ENGINE_PATH}/vfs.c ${ENGINE_PATH}/vfs.h ${ENGINE_PATH}/alist.c ${ENGINE_PATH}/alist.h ${ENGINE_PATH}/sound.h)
set_property(TARGET generate_assets PROPERTY C_STANDARD 99)
set_property(TARGET generate_assets APPEND_STRING PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})
target_link_libraries(generate_assets ${ENGINE_EXTRA_LIBS})

# Where to store generated assets files.
set(ASSETS_STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/assets.stamp")
set(ASSETS_C ${CMAKE_BINARY_DIR}/assets.c)
set(ASSETS_H ${CMAKE_BINARY_DIR}/assets.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Compare assets directory, maybe regenerate assets.c
add_custom_target(assets_check_update ALL
    COMMAND ${CMAKE_COMMAND} -DSRC_DIR=${ASSET_PATH} -DSTAMP_FILE=${ASSETS_STAMP_FILE} -P ${ENGINE_PATH}/build/cmake/dir_checksum.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Calculating assets checksum...")

# Creates assets.{c,h} if ASSETS_STAMP_FILE was changed.
add_custom_command(OUTPUT ${ASSETS_C} ${ASSETS_H}
    COMMAND generate_assets ${ASSET_PATH}
    DEPENDS generate_assets
    DEPENDS assets_check_update
    DEPENDS ${ASSETS_STAMP_FILE}
    COMMENT "Generating assets.c")

# Mark asset files as "generated" so cmake doesn't complain.
set_source_files_properties(${ASSETS_C} ${ASSETS_H} PROPERTIES GENERATED TRUE)

# Add a game library.
function(add_game_library game_name)
    # Mark asset files as "generated".
    set_source_files_properties(${ASSETS_C} ${ASSETS_H} PROPERTIES GENERATED TRUE)

    # Create library.
    add_library(${game_name}_obj OBJECT ${ARGN} ${ASSETS_C} ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    add_library(${game_name} SHARED $<TARGET_OBJECTS:${game_name}_obj>)
    add_library(${game_name}_static STATIC $<TARGET_OBJECTS:${game_name}_obj>)

    target_compile_definitions(${game_name}_obj PRIVATE ENABLE_SHARED)
    target_compile_definitions(${game_name} PRIVATE ENABLE_SHARED)

    # Compile with C99.
    set_property(TARGET ${game_name}_obj PROPERTY C_STANDARD 99)

    # Output shared library into asset path.
    set_target_properties(${game_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${ASSET_PATH}"
        LIBRARY_OUTPUT_DIRECTORY "${ASSET_PATH}")

    # Link libraries.
    target_link_libraries(${game_name} ${ENGINE_EXTRA_LIBS})
    target_link_libraries(${game_name}_static ${ENGINE_EXTRA_LIBS})

    if(ENABLE_SHARED)
        list(APPEND ENGINE_EXTRA_LIBS ${game_name})
    elseif()
        list(APPEND ENGINE_EXTRA_LIBS ${game_name}_static)
    endif()

    # Convenience target to compile and run the engine executable with the same
    # asset path as in cmake_user.
    add_custom_target(run_${game_name}
        COMMAND engine --windowed --mount "${ASSET_PATH}" --game "$<TARGET_FILE_NAME:${game_name}>"
        DEPENDS ${game_name} engine)
endfunction()

# Build game library.
add_subdirectory(${GAME_PATH})

# Main executable.
include_directories(${GAME_PATH})
#add_executable(engine main.c ${GAME_PATH}/glpong.c ${ASSETS_C} ${ASSETS_H} ${ENGINE_SOURCES} ${ENGINE_HEADERS})
add_executable(engine main.c ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Add DEBUG define when building debug executables.
target_compile_definitions(engine PRIVATE LOAD_SHARED)
target_compile_definitions(engine PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions(generate_assets PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# Console capture stdout support
if(ENABLE_CONSOLE)
	target_compile_definitions(engine PRIVATE CONSOLE_ENABLE)
endif()

# GCC compiler settings.
if(CMAKE_COMPILER_IS_GNUCC)
    # More warnings.
    set_property(TARGET engine APPEND_STRING PROPERTY COMPILE_FLAGS "-Wall -Wpedantic -fdiagnostics-show-option ")
    # Do not allow declaring functions without implementing them.
    set_property(TARGET engine APPEND_STRING PROPERTY COMPILE_FLAGS "-Werror-implicit-function-declaration ")
    # Force casting of pointer types.
    set_property(TARGET engine APPEND_STRING PROPERTY COMPILE_FLAGS "-Werror=strict-aliasing ")
    # Force correct return type.
    set_property(TARGET engine APPEND_STRING PROPERTY COMPILE_FLAGS "-Werror=return-type ")
    # DEBUG: Lightweight buffer overflow detection.
    target_compile_options(engine PRIVATE $<$<CONFIG:Debug>:-O2>)
    target_compile_definitions(engine PRIVATE $<$<CONFIG:Debug>:_FORTIFY_SOURCE=2>)
    target_compile_definitions(generate_assets PRIVATE $<$<CONFIG:Debug>:_FORTIFY_SOURCE=2>)

    # QUIRK: stb.h requires all these warnings to be silenced, but it seems
    # warnings have to be set for the source file including the header...
    set(STB_HEADER vfs.c)
    set_property(SOURCE ${STB_HEADER} APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-maybe-uninitialized ")
    set_property(SOURCE ${STB_HEADER} APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-strict-aliasing ")
    set_property(SOURCE ${STB_HEADER} APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-unused-but-set-variable ")
    set_property(SOURCE ${STB_HEADER} APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-unused-value ")
    set_property(SOURCE ${STB_HEADER} APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-unused-function ")
endif()

# MSVC compiler settings.
if(MSVC)
    # More warnings.
    set_property(TARGET engine APPEND_STRING PROPERTY COMPILE_FLAGS "/W3 ")
    # Disable "unsafe" functions that redirects to Microsoft-specific
    # implementations.
    set_property(TARGET engine APPEND_STRING PROPERTY COMPILE_FLAGS "/D_CRT_SECURE_NO_WARNINGS ")
endif()

# Compiling with Clang?
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    set(CLANG 1)
endif()

# Clang compiler settings.
if(CLANG)
    # DEBUG: Bounds checking.
    set_property(TARGET engine APPEND_STRING PROPERTY COMPILE_FLAGS_DEBUG "-fsanitize=address ")
    set_property(TARGET engine APPEND_STRING PROPERTY LINK_FLAGS_DEBUG "-fsanitize=address ")
endif()

# Link.
if(NOT EMSCRIPTEN)
    # Build the engine using the C99 standard.
    set_property(TARGET engine PROPERTY C_STANDARD 99)
    target_link_libraries(engine ${ENGINE_EXTRA_LIBS})
else()
    target_link_libraries(engine)
endif()
