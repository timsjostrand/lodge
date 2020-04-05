# lodge-lib

add_library(lodge-lib STATIC "")

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

target_sources(lodge-lib
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/lodge.c"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_platform.c"
        "${CMAKE_CURRENT_LIST_DIR}/txt.c"
        "${CMAKE_CURRENT_LIST_DIR}/strview.c"
        "${CMAKE_CURRENT_LIST_DIR}/strbuf.c"
        "${CMAKE_CURRENT_LIST_DIR}/str.c"
        "${CMAKE_CURRENT_LIST_DIR}/math4.c"
        "${CMAKE_CURRENT_LIST_DIR}/blob_cur.c"
        "${CMAKE_CURRENT_LIST_DIR}/blob.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/lodge.h"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_assert.h"
        "${CMAKE_CURRENT_LIST_DIR}/lodge_platform.h"
        "${CMAKE_CURRENT_LIST_DIR}/log.h"
        "${CMAKE_CURRENT_LIST_DIR}/txt.h"
        "${CMAKE_CURRENT_LIST_DIR}/strview.h"
        "${CMAKE_CURRENT_LIST_DIR}/strbuf.h"
        "${CMAKE_CURRENT_LIST_DIR}/str.h"
        "${CMAKE_CURRENT_LIST_DIR}/math4.h"
        "${CMAKE_CURRENT_LIST_DIR}/blob_cur.h"
        "${CMAKE_CURRENT_LIST_DIR}/blob.h"
)

target_include_directories(lodge-lib
    PRIVATE
        "${ENGINE_PATH}"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
