# lodge-renderer

add_library(lodge-renderer STATIC "")

target_sources(lodge-renderer
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_renderer_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_sampler_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_texture_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_framebuffer_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_shader_opengl.c"
		"${CMAKE_CURRENT_LIST_DIR}/drawable.c"
		"${CMAKE_CURRENT_LIST_DIR}/vertex_buffer.c"
		"${CMAKE_CURRENT_LIST_DIR}/vertex.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_renderer.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_sampler.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_texture.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_framebuffer.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_opengl.h" # This should be private, but need to port legacy code to not use OpenGL calls directly
		"${CMAKE_CURRENT_LIST_DIR}/lodge_shader.h"
		"${CMAKE_CURRENT_LIST_DIR}/vertex_buffer.h"
		"${CMAKE_CURRENT_LIST_DIR}/vertex.h" # FIXME(TS): vertex types should be more general than `lodge-renderer` but less general than `lodge-lib`
		"${CMAKE_CURRENT_LIST_DIR}/shader.h" # FIXME(TS): compat
)

target_include_directories(lodge-renderer
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-renderer
	PRIVATE
		lodge-build-flags
	PUBLIC
		${GLEW_LIBRARY}
		lodge-lib
		lodge-image
		lodge-collections
		lodge-plugins			# FIXME(TS): should be separate module `lodge-plugin-renderer`
		lodge-vfs				# FIXME(TS): only for shader.{h,c}
)
