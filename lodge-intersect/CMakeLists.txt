# lodge-intersect

add_library(lodge-intersect STATIC "")

target_sources(lodge-intersect
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/intersect.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/intersect.h"
)

target_include_directories(lodge-intersect
    PRIVATE
        "${ENGINE_PATH}"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-intersect
    lodge-lib
)
