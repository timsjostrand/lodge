# lodge-renderer

add_library(lodge-renderer STATIC "")

target_sources(lodge-renderer
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_renderer_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_sampler_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_texture_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_framebuffer_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/drawable.c"
		"${CMAKE_CURRENT_LIST_DIR}/vertex_buffer.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_renderer.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_sampler.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_texture.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_framebuffer.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_opengl.h" # This should be private, but need to port legacy code to not use OpenGL calls directly
		"${CMAKE_CURRENT_LIST_DIR}/vertex_buffer.h"
)

target_include_directories(lodge-renderer
	PRIVATE
		"${ENGINE_PATH}"
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-renderer
	PUBLIC
		"${GLEW_LIBRARY}"
		"lodge-image"
		"lodge-collections"
)