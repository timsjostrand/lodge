# Where to store generated assets files.
set(ASSETS_STAMP_FILE "${CMAKE_BINARY_DIR}/assets.stamp")

# Asset generator executable
add_executable(generate_assets
    "${CMAKE_CURRENT_LIST_DIR}/generate_assets.c"
    "${ENGINE_PATH}/lodge-vfs/vfs.h"
    "${ENGINE_PATH}/lodge-vfs/vfs.c"
    "${ENGINE_PATH}/lodge-lib/strview.c"
    "${ENGINE_PATH}/lodge-lib/str.c"
    "${ENGINE_PATH}/lodge-lib/strbuf.c"
    "${ENGINE_PATH}/lodge-lib/lodge.c"
    "${ENGINE_PATH}/lodge-lib/lodge_platform.h"
)

target_include_directories(generate_assets
    PRIVATE
        "${ENGINE_PATH}"
        ${ENGINE_INCLUDES}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

# FIXME(TS): dependencies should be handled in another way.
target_link_libraries(generate_assets
    glfw
    lodge-lib
    lodge-sound
    lodge-collections
    lodge-plugin-vfs
    lodge-vfs
)

# Compare assets directory, maybe regenerate assets.c
add_custom_target(assets_check_update ALL
    COMMAND ${CMAKE_COMMAND} -DSRC_DIR=${ASSET_PATH} -DSTAMP_FILE=${ASSETS_STAMP_FILE} -P ${ENGINE_PATH}/cmake/dir_checksum.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Calculating assets checksum...")

# Mark asset files as "generated" so cmake doesn't complain.
set_source_files_properties("${ASSETS_C}" PROPERTIES GENERATED TRUE)
set_source_files_properties("${ASSETS_H}" PROPERTIES GENERATED TRUE)

# Creates assets.{c,h} if ASSETS_STAMP_FILE was changed.
add_custom_command(OUTPUT "${ASSETS_C}" "${ASSETS_H}"
    COMMAND generate_assets ${ASSET_PATH}
    DEPENDS generate_assets
    DEPENDS assets_check_update
    DEPENDS ${ASSETS_STAMP_FILE}
    COMMENT "Generating assets.c")

# Targets can depend on this target to generate assets.{c,h}.
add_custom_target(generate_assets_do DEPENDS ${ASSETS_C} ${ASSETS_H})

# lodge-assets
add_library(lodge-assets STATIC ${ASSETS_C} ${ASSETS_H})
set_target_properties(lodge-assets PROPERTIES LINKER_LANGUAGE C)

target_sources(lodge-assets
    PRIVATE
        "${ENGINE_PATH}"
    PUBLIC
)

target_include_directories(lodge-assets
    PRIVATE
        "${ENGINE_PATH}"
        ${ENGINE_INCLUDES}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

# FIXME(TS): link libraries should be inherited from engine libs (ie its not nice
# to have to depend on lodge-sound if its not being used)
target_link_libraries(lodge-assets
    lodge-lib
    lodge-sound
    lodge-collections
    lodge-renderer
    lodge-asset-pyxel   # FIXME(TS): these assets should be added with fragments and an asset system instead
    lodge-asset-atlas
    lodge-vfs-reload
    glfw
    ${ENGINE_EXTRA_LIBS}
)

add_dependencies(lodge-assets generate_assets_do)