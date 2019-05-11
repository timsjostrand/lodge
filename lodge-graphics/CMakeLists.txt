# lodge-graphics

add_library(lodge-graphics STATIC "")

target_sources(lodge-graphics
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/graphics_types.c"
		"${CMAKE_CURRENT_LIST_DIR}/vertex_buffer.c"
		"${CMAKE_CURRENT_LIST_DIR}/drawable.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/graphics_types.h"
		"${CMAKE_CURRENT_LIST_DIR}/vertex_buffer.h"
		"${CMAKE_CURRENT_LIST_DIR}/drawable.h"
)

target_include_directories(lodge-graphics
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-graphics
	PUBLIC
		"${GLEW_LIBRARY}"
		"lodge-collections"
)
