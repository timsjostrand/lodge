cmake_minimum_required(VERSION 3.20)

project(lodge VERSION 0.0.1 LANGUAGES C)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-build-flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-plugin.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-add-executable.cmake)

# QUIRK: Surpress warning with cmake 3.0 on OSX.
#set(CMAKE_MACOSX_RPATH 1)

# Check for compatibility.
include(CheckFunctionExists)
include(CheckSymbolExists)

#
# Find lodge-* modules.
#
list(APPEND LODGE_MODULES_PATH "${CMAKE_CURRENT_LIST_DIR}/lodge-*/")
file(GLOB LODGE_MODULES_AVAILABLE "${LODGE_MODULES_PATH}")
foreach(module ${LODGE_MODULES_AVAILABLE})
    get_filename_component(module_name "${module}" NAME)
    message(STATUS "Found lodge module: ${module_name}")
    add_subdirectory(${module} ${CMAKE_BINARY_DIR}/module/${module_name})
    list(APPEND LODGE_MODULE_NAMES "${module_name}")
endforeach()

#
# Minimal set of modules to base a typical project off.
#
add_library(lodge-modules-minimal INTERFACE)
target_link_libraries(lodge-modules-minimal
    INTERFACE
        lodge-main
        lodge-plugins
        lodge-plugin-types
        lodge-plugin-vfs
)

#
# The full set of modules found in `LODGE_MODULES_PATH`.
#
add_library(lodge-modules-full INTERFACE)
target_link_libraries(lodge-modules-full
    INTERFACE
        lodge-modules-minimal
        ${LODGE_MODULE_NAMES}
)

#
# Editor executable (minimal dependencies).
#
add_executable(lodge-editor-minimal
    "${CMAKE_CURRENT_LIST_DIR}/cmake/dummy.c"
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-editor.rc>
)
target_link_libraries(lodge-editor-minimal
    PRIVATE
        lodge-build-flags
    PUBLIC
        lodge-modules-minimal
        lodge-plugin-scenes
        lodge-plugin-files
        lodge-plugin-editor
        lodge-plugin-stdio-log
)

#
# Editor executable (all dependencies).
#
add_executable(lodge-editor-full
    "${CMAKE_CURRENT_LIST_DIR}/cmake/dummy.c"
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/cmake/lodge-editor.rc>
)
target_link_libraries(lodge-editor-full
    PRIVATE
        lodge-build-flags
    PUBLIC
        lodge-modules-full
        lodge-plugin-editor
)