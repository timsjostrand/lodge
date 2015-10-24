project(glpong)

include(cmake_user.txt)
cmake_minimum_required(VERSION 3.1)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# Compile options
option(ENABLE_CONSOLE "Compile with console support" ON)

# Add debug options of debug build type
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DDEBUG)
endif()

# QUIRK: Define M_PI on Windows.
add_definitions(-D_USE_MATH_DEFINES)

# Enable asset hotswap.
add_definitions(-DVFS_ENABLE_FILEWATCH)

# Check for compatibility.
include(CheckFunctionExists)
check_function_exists(strnlen HAVE_STRNLEN)

if(NOT EMSCRIPTEN)
    # Build GLFW.
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
    add_subdirectory(lib/glfw)
    include_directories(lib/glfw/include)
    list(APPEND EXTRA_LIBS glfw ${GLFW_LIBRARIES})

    # Build glew.
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "Build the shared glew library")
    add_subdirectory(lib/glew)
    include_directories(lib/glew/include)
    add_definitions(-DGLEW_STATIC)
    list(APPEND EXTRA_LIBS libglew_static ${GL_LIBRARY}
        ${PLATFORM_SPECIFIC_LIBRARIES})

    # Build cJSON.
    add_subdirectory(lib/cjson)
    list(APPEND EXTRA_LIBS cJSON)

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
        include_directories(lib/openal/include/AL)

        # OpenAL has different names on different platforms.
        if(WIN32)
            list(APPEND EXTRA_LIBS OpenAL32)
        else()
            list(APPEND EXTRA_LIBS openal)
        endif()
    else()
        # Use the OS X OpenAL framework.
        find_library(OPENAL_LIBRARY OpenAL)
        find_path(OPENAL_INCLUDE_DIR al.h)
        include_directories(${OPENAL_INCLUDE_DIR})
        list(APPEND EXTRA_LIBS ${OPENAL_LIBRARY})
    endif()
else()
    # glfw and glew is provided by emscripten
    option(USE_GLFW_3 "Use glfw3" ON)
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} --preload-file test_assets/test.png")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} --preload-file test_assets/vivaldi.ogg")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} --preload-file test_assets/paddle.png")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=100000000")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-warn-absolute-paths")
endif()

# Include stb.h
include_directories(lib/)

set(SOURCE math4.c core_console.c graphics.c shader.c input.c texture.c
    color.c sound.c vfs.c atlas.c monotext.c str.c list.c console.c
    spritebatch.c animatedsprites.c alist.c core.c core_argv.c core_reload.c assets.c )
set(HEADERS math4.h core_console.h graphics.h shader.h input.h texture.h
    color.h sound.h vfs.h atlas.h monotext.h str.h list.h console.h
	spritebatch.h animatedsprites.h alist.h core.h core_argv.h core_reload.h assets.h )

# Asset generator executable
add_executable(generate_assets generate_assets.c vfs.c vfs.h alist.c alist.h sound.h)
set_property(TARGET generate_assets PROPERTY C_STANDARD 99)
set_property(TARGET generate_assets APPEND_STRING PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})
target_link_libraries(generate_assets ${EXTRA_LIBS})

add_custom_command(OUTPUT assets.c assets.h
				   COMMAND generate_assets ARGS ${ASSET_PATH}
				   COMMENT "Generating assets.c"
				   DEPENDS generate_assets)
				   #DEPENDS check_need_update_assets)

# Main executable.
add_executable(glpong main.c ${SOURCE} ${HEADERS})
set_property(TARGET glpong APPEND_STRING PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})
set_property(TARGET glpong APPEND_STRING PROPERTY INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR})

# Console capture stdout support
if(ENABLE_CONSOLE)
	set_property(TARGET glpong PROPERTY COMPILE_DEFINITIONS CONSOLE_ENABLE)
endif()

# GCC compiler settings.
if(CMAKE_COMPILER_IS_GNUCC)
    # More warnings.
    set_property(TARGET glpong APPEND_STRING PROPERTY COMPILE_FLAGS "-Wall -Wpedantic -fdiagnostics-show-option ")
    # Do not allow declaring functions without implementing them.
    set_property(TARGET glpong APPEND_STRING PROPERTY COMPILE_FLAGS
        "-Werror-implicit-function-declaration ")
    # Force casting of pointer types.
    set_property(TARGET glpong APPEND_STRING PROPERTY COMPILE_FLAGS
        "-Werror=strict-aliasing ")
	# DEBUG: Lightweight buffer overflow detection.
	set_property(TARGET glpong APPEND_STRING PROPERTY COMPILE_FLAGS_DEBUG
        "-D_FORTIFY_SOURCE=2 ")

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
    set_property(TARGET glpong APPEND_STRING PROPERTY COMPILE_FLAGS "/W3 ")
    # Disable "unsafe" functions that redirects to Microsoft-specific
    # implementations.
    set_property(TARGET glpong APPEND_STRING PROPERTY COMPILE_FLAGS
        "/D_CRT_SECURE_NO_WARNINGS ")
endif()

# Compiling with Clang?
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
	set(CLANG 1)
endif()

# Clang compiler settings.
if(CLANG)
	# DEBUG: Bounds checking.
	set_property(TARGET glpong APPEND_STRING PROPERTY COMPILE_FLAGS_DEBUG "-fsanitize=address ")
	set_property(TARGET glpong APPEND_STRING PROPERTY LINK_FLAGS_DEBUG "-fsanitize=address ")
endif()

# Link.
if(NOT EMSCRIPTEN)
    # Build glpong using the C99 standard.
    set_property(TARGET glpong PROPERTY C_STANDARD 99)
    target_link_libraries(glpong ${EXTRA_LIBS})
else()
    target_link_libraries(glpong)
endif()
