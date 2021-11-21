# lodge-collections

add_library(lodge-collections STATIC "")

target_sources(lodge-collections
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/alist.c"
        "${CMAKE_CURRENT_LIST_DIR}/rqueue.c"
        "${CMAKE_CURRENT_LIST_DIR}/sparse_set.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/alist.h"
        "${CMAKE_CURRENT_LIST_DIR}/rqueue.h"
        "${CMAKE_CURRENT_LIST_DIR}/sparse_set.h"
)

target_include_directories(lodge-collections
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-collections
    PRIVATE
        lodge-build-flags
        lodge-lib # FIXME(TS): only for lodge_assert
)
