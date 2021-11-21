#
# External library: glew.
#
if(ENABLE_SHARED)
    set(GLEW_LIBRARY libglew_shared)
else()
    set(GLEW_LIBRARY libglew_static)
endif()
add_subdirectory(opengl/lib/glew ${CMAKE_BINARY_DIR}/lib/glew)

#
# lodge-gfx
#
add_library(lodge-gfx STATIC "")

target_sources(lodge-gfx
	PRIVATE
		"opengl/lodge_opengl.h"
		"opengl/lodge_gfx_opengl.c"
		"opengl/lodge_sampler_opengl.c"
		"opengl/lodge_texture_opengl.c"
		"opengl/lodge_framebuffer_opengl.c"
		"opengl/lodge_shader_opengl.c" 
		"opengl/lodge_buffer_object_opengl.c"
		"opengl/lodge_pipeline_opengl.c"
		"opengl/lodge_drawable_opengl.c"
		"lodge_parametric_drawable.c"

		"lodge_debug_draw.c"
		"lodge_static_mesh.c"
		"vertex_buffer.c"
		"vertex.c"
	PUBLIC
		"lodge_gfx.h"
		"lodge_gfx_types.h"
		"lodge_sampler.h"
		"lodge_texture.h"
		"lodge_framebuffer.h"
		"lodge_shader.h"
		"lodge_buffer_object.h"
		"lodge_pipeline.h"
		"lodge_drawable.h"
		"lodge_parametric_drawable.h"
		
		"lodge_debug_draw.h"
		"lodge_static_mesh.h"
		"vertex_buffer.h"
		"vertex.h" # FIXME(TS): vertex types should be more general than `lodge-gfx` but less general than `lodge-lib`
)

target_include_directories(lodge-gfx
	PRIVATE
		"opengl/"
	PUBLIC
		"./"
)

target_link_libraries(lodge-gfx
	PRIVATE
		lodge-build-flags
	PUBLIC
		${GLEW_LIBRARY}
		lodge-lib
		lodge-image
		lodge-collections
		lodge-plugins			# FIXME(TS): should be separate module `lodge-plugin-gfx`
)
