# lodge-collections

add_library(lodge-collections "")

target_sources(lodge-collections
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/alist.c"
        "${CMAKE_CURRENT_LIST_DIR}/array.c"
        "${CMAKE_CURRENT_LIST_DIR}/list.c"
        "${CMAKE_CURRENT_LIST_DIR}/rqueue.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/alist.h"
        "${CMAKE_CURRENT_LIST_DIR}/array.h"
        "${CMAKE_CURRENT_LIST_DIR}/list.h"
        "${CMAKE_CURRENT_LIST_DIR}/rqueue.h"
)

target_include_directories(lodge-collections
    PRIVATE
        "${ENGINE_PATH}"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
