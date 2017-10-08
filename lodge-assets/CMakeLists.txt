# Where to store generated assets files.
set(ASSETS_STAMP_FILE "${CMAKE_BINARY_DIR}/assets.stamp")

# Asset generator executable
add_executable(generate_assets
    "${CMAKE_CURRENT_LIST_DIR}/generate_assets.c"
    "${ENGINE_PATH}/vfs.c"
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
    lodge-sound
    lodge-collections
)

# Compare assets directory, maybe regenerate assets.c
add_custom_target(assets_check_update ALL
    COMMAND ${CMAKE_COMMAND} -DSRC_DIR=${ASSET_PATH} -DSTAMP_FILE=${ASSETS_STAMP_FILE} -P ${ENGINE_PATH}/cmake/dir_checksum.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Calculating assets checksum...")

# Creates assets.{c,h} if ASSETS_STAMP_FILE was changed.
add_custom_command(OUTPUT ${ASSETS_C} ${ASSETS_H}
    COMMAND generate_assets ${ASSET_PATH}
    DEPENDS generate_assets
    DEPENDS assets_check_update
    DEPENDS ${ASSETS_STAMP_FILE}
    COMMENT "Generating assets.c")

# Targets can depend on this target to generate assets.{c,h}.
add_custom_target(generate_assets_do DEPENDS ${ASSETS_C} ${ASSETS_H})

# lodge-assets
add_library(lodge-assets STATIC "")

target_sources(lodge-assets
    PRIVATE
        "${CMAKE_BINARY_DIR}/${ASSETS_C}"
        "${ENGINE_PATH}"
    PUBLIC
        "${CMAKE_BINARY_DIR}/${ASSETS_H}"
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
    glfw
    lodge-sound
    lodge-collections
    lodge-asset-pyxel   # FIXME(TS): these assets should be added with fragments and an asset system instead
    ${ENGINE_EXTRA_LIBS}
)

add_dependencies(lodge-assets generate_assets_do)