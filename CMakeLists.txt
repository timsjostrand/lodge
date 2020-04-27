project(lodge C)

cmake_minimum_required(VERSION 3.6)

include(${ENGINE_PATH}/cmake/lodge-build-flags.cmake)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
set(CMAKE_MACOSX_RPATH 1)

# Compile options
option(ENABLE_CONSOLE "Compile with console support" ON)
option(ENABLE_SHARED "Enable game hotswapping" ON)

# Enable asset hotswap.
add_definitions(-DVFS_ENABLE_FILEWATCH)

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

# Inform all modules that assets files are generated later.
set(ASSETS_C assets.c)
set(ASSETS_H assets.h)
# Mark asset files as "generated" so cmake doesn't complain.
#set_source_files_properties("${ASSETS_C}" PROPERTIES GENERATED TRUE)
#set_source_files_properties("${ASSETS_H}" PROPERTIES GENERATED TRUE)

# Library: GLFW.
set(BUILD_SHARED_LIBS ${ENABLE_SHARED} CACHE BOOL "Build shared libraries" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
set(GLFW_USE_RETINA OFF CACHE BOOL "Disable GLFW retina scaling" FORCE)
add_subdirectory(${ENGINE_PATH}/lib/glfw ${CMAKE_BINARY_DIR}/lib/glfw)

# Library: glew.
if(ENABLE_SHARED)
    set(GLEW_LIBRARY glew)
else()
    set(GLEW_LIBRARY glew_s)
endif()
set(BUILD_UTILS OFF CACHE BOOL "Skip glew executables" FORCE)
add_subdirectory(${ENGINE_PATH}/lib/glew/build/cmake ${CMAKE_BINARY_DIR}/lib/glew)
target_include_directories(${GLEW_LIBRARY}
    PUBLIC
        "${ENGINE_PATH}/lib/glew/include"
)

# Library: cJSON.
set(ENABLE_CJSON_TEST OFF CACHE BOOL "Enable building cJSON test" FORCE)
add_subdirectory(${ENGINE_PATH}/lib/cjson ${CMAKE_BINARY_DIR}/lib/cjson)
set(CJSON_LIBRARY cjson)
target_include_directories(${CJSON_LIBRARY} PUBLIC "${ENGINE_PATH}/lib")

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

# Library: libarchive.
set(ENABLE_NETTLE OFF CACHE BOOL "Enable use of Nettle" FORCE)
set(ENABLE_OPENSSL OFF CACHE BOOL "Enable use of OpenSSL" FORCE)
set(ENABLE_TAR OFF CACHE BOOL "Enable tar building" FORCE)
set(ENABLE_CPIO OFF CACHE BOOL "Enable cpio building" FORCE)
set(ENABLE_XATTR OFF CACHE BOOL "Enable extended attribute support" FORCE)
set(ENABLE_ACL OFF CACHE BOOL "Enable ACL support" FORCE)
set(ENABLE_ICONV OFF CACHE BOOL "Enable iconv support" FORCE)
set(ENABLE_TEST OFF CACHE BOOL "Enable unit and regression tests" FORCE)
add_subdirectory(${ENGINE_PATH}/lib/libarchive ${CMAKE_BINARY_DIR}/lib/libarchive)
if(ENABLE_SHARED)
    set(LIBARCHIVE_LIBRARY archive)
else()
    # NOTE: Bug where LIBARCHIVE_STATIC is not applied for the
    # archive_static target in Visual Studio. Force define here.
    add_definitions(-DLIBARCHIVE_STATIC)
    set(LIBARCHIVE_LIBRARY archive_static)
endif()
set(LIBARCHIVE_INCLUDE_DIR ${ENGINE_PATH}/lib/libarchive)
target_include_directories(${LIBARCHIVE_LIBRARY} PUBLIC "${LIBARCHIVE_INCLUDE_DIR}")

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
