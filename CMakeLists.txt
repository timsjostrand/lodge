project(engine C)

cmake_minimum_required(VERSION 3.6)

# Import compiler flags defined in another file.
include(${ENGINE_PATH}/cmake/engine-compiler-flags.cmake)

# Default to C99.
set(CMAKE_C_STANDARD 99)

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
include(CheckSymbolExists)

check_function_exists(strnlen HAVE_STRNLEN)
if(HAVE_STRNLEN)
    add_definitions(-DHAVE_STRNLEN)
endif()

check_function_exists(vsnprintf HAVE_VSNPRINTF)
if(HAVE_VSNPRINTF)
    add_definitions(-DHAVE_VSNPRINTF)
endif()

if(MSVC14)
	check_symbol_exists(snprintf stdio.h HAVE_SNPRINTF)
else()
	check_function_exists(snprintf HAVE_SNPRINTF)
endif()

if(HAVE_SNPRINTF)
    add_definitions(-DHAVE_SNPRINTF)
endif()

# Include engine sources.
set(ENGINE_INCLUDES ${ENGINE_PATH})

# Store DLLs alongside engine.exe.
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
set_source_files_properties("${ASSETS_C}" PROPERTIES GENERATED TRUE)
set_source_files_properties("${ASSETS_H}" PROPERTIES GENERATED TRUE)

if(NOT EMSCRIPTEN)
    # Library: GLFW.
    set(BUILD_SHARED_LIBS ${ENABLE_SHARED} CACHE BOOL "Build shared libraries" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
    set(GLFW_USE_RETINA OFF CACHE BOOL "Disable GLFW retina scaling" FORCE)
    add_subdirectory(${ENGINE_PATH}/lib/glfw ${CMAKE_BINARY_DIR}/lib/glfw)
    list(APPEND ENGINE_INCLUDES ${ENGINE_PATH}/lib/glfw/include)
    list(APPEND ENGINE_EXTRA_LIBS glfw ${GLFW_LIBRARIES})

    # Library: glew.
    if(ENABLE_SHARED)
        set(GLEW_LIBRARY glew)
    else()
        set(GLEW_LIBRARY glew_s)
    endif()
    list(APPEND ENGINE_EXTRA_LIBS ${GLEW_LIBRARY} ${GLEW_LIBRARIES})
    set(BUILD_UTILS OFF CACHE BOOL "Skip glew executables" FORCE)
    add_subdirectory(${ENGINE_PATH}/lib/glew/build/cmake ${CMAKE_BINARY_DIR}/lib/glew)
    target_include_directories(${GLEW_LIBRARY}
        PUBLIC
            "${ENGINE_PATH}/lib/glew/include"
    )

    # Library: cJSON.
    add_subdirectory(${ENGINE_PATH}/lib/cjson ${CMAKE_BINARY_DIR}/lib/cjson)
    set(ENABLE_CJSON_TEST OFF CACHE BOOL "Enable building cJSON test" FORCE)
    set(CJSON_LIBRARY cjson)
    target_include_directories(${CJSON_LIBRARY} PUBLIC "${ENGINE_PATH}/lib")
    list(APPEND ENGINE_EXTRA_LIBS ${CJSON_LIBRARY}) # NOTE(TS): needed or atlas support, but should be moved to a module

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

# Load engine modules
list(APPEND ENGINE_MODULES_PATH "${ENGINE_PATH}/lodge-*/")
file(GLOB ENGINE_MODULES_AVAILABLE "${ENGINE_MODULES_PATH}")
foreach(module ${ENGINE_MODULES_AVAILABLE})
    get_filename_component(module_name "${module}" NAME)
    message(STATUS "Found lodge module: ${module_name}")
    add_subdirectory(${module} ${CMAKE_BINARY_DIR}/module/${module_name})
endforeach()

# Engine sources
list(APPEND ENGINE_SOURCES_LOCAL
    math4.c
    lodge_plugins.c
    core_console.c
    events.c
    vertex.c
    shader.c
    input.c
    texture.c
    color.c
    vfs.c
    atlas.c
    str.c
    strview.c
    strbuf.c
    txt.c
    lodge.c
    console.c
    spritebatch.c
    animatedsprites.c
    core_argv.c
    util_reload.c
    particles.c
    collide.c
    sprite_group.c
    util_graphics.c
    env.c
    lodge_window_glfw.c
    blob.c
)

list(APPEND ENGINE_HEADERS_LOCAL
    math4.h
    lodge_platform.h
    lodge_assert.h
    lodge_plugin.h
    lodge_plugins.h
    core_console.h
    events.h
    shader.h
    input.h
    texture.h
    color.h
    vfs.h
    atlas.h
    str.h
    strview.h
    strbuf.h
    txt.h
    lodge.h
    console.h
    spritebatch.h
    animatedsprites.h
    core_argv.h
    util_reload.h
    particles.h
    game.h
    collide.h
    geometry.h
    sprite_group.h
    util_graphics.h
    vertex.h
    env.h
    lodge_window.h
    blob.h
)

# Prepend all sources with absolute engine path (because of reasons...)
foreach(src ${ENGINE_SOURCES_LOCAL})
    list(APPEND ENGINE_SOURCES ${ENGINE_PATH}/${src})
endforeach()
foreach(src ${ENGINE_HEADERS_LOCAL})
    list(APPEND ENGINE_HEADERS ${ENGINE_PATH}/${src})
endforeach()

# Lodge Interface sources.
list(APPEND LODGEI_SOURCES lodge_interface.c)
list(APPEND LODGEI_HEADERS lodge_interface.h)

# Prepend all sources with absolute engine path (because of reasons...)
foreach(src ${LODGEI_SOURCES})
    list(APPEND LODGEI_SOURCES_HEADERS ${ENGINE_PATH}/${src})
endforeach()
foreach(src ${LODGEI_HEADERS})
    list(APPEND LODGEI_SOURCES_HEADERS ${ENGINE_PATH}/${src})
endforeach()

# Include all source directories
include_directories(${ENGINE_INCLUDES})

# Add the lodge engine library.
function(add_lodgei_library)
    # Create library.
    add_library(lodgei SHARED ${ARGN} ${ASSETS_C}
        ${ENGINE_SOURCES} ${ENGINE_HEADERS}
        ${LODGEI_SOURCES_HEADERS}
    )
    
    # Compile with console output support?
    if(ENABLE_CONSOLE)
        target_compile_definitions(lodgei PRIVATE CONSOLE_ENABLE)
    endif()

    # Compile as shared library?
    if(ENABLE_SHARED)
        target_compile_definitions(lodgei PRIVATE ENABLE_SHARED)
    endif()

    # Compile with DEBUG flag?
    target_compile_definitions(lodgei PRIVATE $<$<CONFIG:Debug>:DEBUG>)

    # Compile with correct flags.
    set_property(TARGET lodgei PROPERTY COMPILE_FLAGS "${ENGINE_COMPILER_FLAGS}")
    set_property(TARGET lodgei PROPERTY LINK_FLAGS "${ENGINE_LINKER_FLAGS}")

    # GCC compiler settings.
    if(CMAKE_COMPILER_IS_GNUCC)
        # DEBUG: Lightweight buffer overflow detection.
        target_compile_options(lodgei PRIVATE $<$<CONFIG:Debug>:-O2>)
        target_compile_definitions(lodgei PRIVATE $<$<CONFIG:Debug>:_FORTIFY_SOURCE=2>)

        # Static STD libs.
        #set(ENGINE_LINKER_FLAGS "-static-libgcc ${ENGINE_LINKER_FLAGS} ")
    endif()

    # Set compiler flags.
    set_property(TARGET lodgei PROPERTY COMPILE_FLAGS "${ENGINE_COMPILER_FLAGS}")
    set_property(TARGET lodgei PROPERTY LINK_FLAGS "${ENGINE_LINKER_FLAGS}")

    # Link libraries.
    target_link_libraries(lodgei ${ENGINE_EXTRA_LIBS})
endfunction()

# Add a game library.
function(add_game_library game_name)
    # Create library.
    add_library(${game_name} ${ARGN} ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    
    # Compile with correct flags.
    set_property(TARGET ${game_name} PROPERTY COMPILE_FLAGS "${ENGINE_COMPILER_FLAGS}")
    set_property(TARGET ${game_name} PROPERTY LINK_FLAGS "${ENGINE_LINKER_FLAGS}")
    
    # TODO(TS): The engine depends on these modules, so they are forced to be
    #           part of the build. Mainly because core.c pulls depends on a lot
    #           of things, so the proper fix is probably to make core itself into
    #           a module.
    list(APPEND ENGINE_MODULES lodge-assets)
    list(APPEND ENGINE_MODULES lodge-sound)
    list(APPEND ENGINE_MODULES lodge-collections)
    list(APPEND ENGINE_MODULES lodge-sprite) # NOTE(TS): needed by console.h
    list(APPEND ENGINE_MODULES lodge-stb) # NOTE(TS): needed for texture.c, vfs.c
    list(APPEND ENGINE_MODULES lodge-image) # NOTE(TS): needed for texture.c
    list(APPEND ENGINE_MODULES lodge-monotext) # NOTE(TS): needed for console

    # Engine modules
    list(REMOVE_DUPLICATES ENGINE_MODULES)
    foreach(module ${ENGINE_MODULES})
        list(APPEND ENGINE_EXTRA_LIBS ${module})
        string(TOUPPER "ENABLE_${module}" module_define)
        string(REPLACE "-" "_" module_define "${module_define}")
        message(STATUS "Adding lodge module: ${module} (${module_define})")
        list(APPEND ENGINE_DEFINES ${module_define})
    endforeach()
        
    # Update defines for modules
    target_compile_definitions(${game_name} PUBLIC ${ENGINE_DEFINES})
    foreach(module ${ENGINE_MODULES})
        target_compile_definitions(${module} PUBLIC ${ENGINE_DEFINES})
        set_property(TARGET ${module} PROPERTY COMPILE_FLAGS "${ENGINE_COMPILER_FLAGS}")
        set_property(TARGET ${module} PROPERTY LINK_FLAGS "${ENGINE_LINKER_FLAGS}")
    endforeach()

    # Store as dependencies that will be applied to the 'engine' target later.
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
    target_link_libraries(${game_name} ${ENGINE_EXTRA_LIBS})
    
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
    add_executable(engine ${ENGINE_PATH}/main.c ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    set_property(TARGET engine PROPERTY RUNTIME_OUTPUT_NAME "${game_name}")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)

    # Engine depends on game library.
    #add_dependencies(engine ${ENGINE_DEPS})

    # Add DEBUG define when building debug executables.
    if(ENABLE_SHARED)
        target_compile_definitions(engine PRIVATE LOAD_SHARED)
    endif()
    target_compile_definitions(engine PRIVATE $<$<CONFIG:Debug>:DEBUG>)

    # Console capture stdout support
    if(ENABLE_CONSOLE)
        target_compile_definitions(engine PRIVATE CONSOLE_ENABLE)
    endif()

    # GCC compiler settings.
    if(CMAKE_COMPILER_IS_GNUCC)
        # DEBUG: Lightweight buffer overflow detection.
        target_compile_options(engine PRIVATE $<$<CONFIG:Debug>:-O2>)
        target_compile_definitions(engine PRIVATE $<$<CONFIG:Debug>:_FORTIFY_SOURCE=2>)
    endif()

    # Set compiler flags.
    set_property(TARGET engine PROPERTY COMPILE_FLAGS "${ENGINE_COMPILER_FLAGS}")
    set_property(TARGET engine PROPERTY LINK_FLAGS "${ENGINE_LINKER_FLAGS}")

    # Link.
    if(NOT EMSCRIPTEN)
        target_link_libraries(engine ${ENGINE_GAME_LIBRARY} ${ENGINE_EXTRA_LIBS})
    else()
        target_link_libraries(engine)
    endif()
endfunction()
