#include "lodge_gfx.h"

#include "lodge_opengl.h"
#include "lodge_texture.h"
#include "lodge_sampler.h"
#include "color.h"

struct lodge_gfx
{
	strview_t				library;
};

static const char* loc_opengl_debug_type(GLenum type)
{
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		return "ERROR";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "DEPRECATED_BEHAVIOR";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "UNDEFINED_BEHAVIOR";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "PORTABILITY";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "PERFORMANCE";
	case GL_DEBUG_TYPE_OTHER:
		return "OTHER";
	case GL_DEBUG_TYPE_MARKER:
		return "MARKER";
	case GL_DEBUG_TYPE_PUSH_GROUP:
		return "PUSH_GROUP";
	case GL_DEBUG_TYPE_POP_GROUP:
		return "POP_GROUP";
	default:
		return "UNKNOWN_TYPE";
	}
}

static const char* loc_opengl_debug_severity(GLenum severity)
{
	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return "HIGH";
	case GL_DEBUG_SEVERITY_MEDIUM:
		return "MEDIUM";
	case GL_DEBUG_SEVERITY_LOW:
		return "LOW";
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return "NOTIFICATION";
	default:
		return "UNKNOWN_SEVERITY";
	}
}

static const char* loc_opengl_debug_source(GLenum source)
{
	switch(source) {
	case GL_DEBUG_SOURCE_API:
		return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "WINDOW_SYSTEM";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "SHADER_COMPILER";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "THIRD_PARTY";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "APPLICATION";
	case GL_DEBUG_SOURCE_OTHER:
		return "OTHER";
	default:
		return "UNKNOWN_SOURCE";
	}
}

static void GLAPIENTRY loc_opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		return;
	}

	printf("OpenGL: %s/%s/%s:\n `%s`\n",
		loc_opengl_debug_source(source),
		loc_opengl_debug_type(type),
		loc_opengl_debug_severity(severity),
		message
	);
}

#if 0
static struct lodge_ret lodge_gfx_opengl_init(struct lodge_gfx *gfx)
{
	gfx->library = strview_static("OpenGL");
	return lodge_success();
}

static void lodge_gfx_opengl_free(struct lodge_gfx *gfx)
{
}
#endif

#if 0
static void graphics_frames_register(struct frames *f, float delta_time)
{
	f->frames++;
	f->frame_time_min = fmin(delta_time, f->frame_time_min);
	f->frame_time_max = fmax(delta_time, f->frame_time_max);
	f->frame_time_sum += delta_time;

	if(lodge_window_get_time() - f->last_frame_report >= 1000.0) {
		f->last_frame_report = lodge_window_get_time();
		f->frame_time_avg = f->frame_time_sum / (float)f->frames;
		if(f->callback != NULL) {
			f->callback(f);
		}
		f->frame_time_max = FLT_MIN;
		f->frame_time_min = FLT_MAX;
		f->frame_time_sum = 0;
		f->frames = 0;
	}
}
#endif

struct lodge_gfx* lodge_gfx_new()
{
	struct lodge_gfx *gfx = (struct lodge_gfx *) calloc(1, sizeof(struct lodge_gfx));
	ASSERT_OR(gfx) {
		return NULL;
	}
	gfx->library = strview_static("OpenGL");
	return gfx;
}

void lodge_gfx_free(struct lodge_gfx *gfx)
{
	//ASSERT_NOT_IMPLEMENTED();
	free(gfx);
}

struct lodge_ret lodge_gfx_attach(struct lodge_gfx *gfx)
{
	/* Init GLEW. */
	glewExperimental = GL_TRUE;
	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_err));
		return lodge_error("Failed to initialize GLEW");
	}

	/* NOTE: Something in glewInit() is causing an 0x0500 OpenGL error, but the
	* Internet says we should ignore this error. */
	while(glGetError() != GL_NO_ERROR) {
		/* Ignore this error. */
	}

	glEnable(GL_DEBUG_OUTPUT);
	GL_OK_OR_RETURN(lodge_error("Failed to set `GL_DEBUG_OUTPUT`"));
	glDebugMessageCallback((GLDEBUGPROC)&loc_opengl_debug_message_callback, 0);
	GL_OK_OR_RETURN(lodge_error("Failed call glDebugMessageCallback`"));

	return lodge_success();
}

void lodge_gfx_detach(struct lodge_gfx *gfx)
{
}

strview_t lodge_gfx_get_library(struct lodge_gfx *gfx)
{
	return gfx->library;
}

void lodge_gfx_bind_texture(int slot, const lodge_texture_t texture, enum lodge_texture_target target)
{
	const GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(lodge_texture_target_to_gl(target), lodge_texture_to_gl(texture));
	GL_OK_OR_ASSERT("Failed to bind texture");
}

void lodge_gfx_bind_sampler(int slot, const lodge_sampler_t sampler)
{
	glBindSampler(slot, lodge_sampler_to_gl(sampler));
	GL_OK_OR_ASSERT("Failed to bind sampler");
}

void lodge_gfx_bind_texture_unit(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler, enum lodge_texture_target target)
{
	const GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(lodge_texture_target_to_gl(target), lodge_texture_to_gl(texture));
	GL_OK_OR_ASSERT("Failed to bind texture");

	glBindSampler(slot, lodge_sampler_to_gl(sampler));
	GL_OK_OR_ASSERT("Failed to bind sampler");
}

void lodge_gfx_bind_texture_2d(int slot, const lodge_texture_t texture)
{
	lodge_gfx_bind_texture(slot, texture, LODGE_TEXTURE_TARGET_2D);
}

void lodge_gfx_bind_texture_unit_2d(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler)
{
	lodge_gfx_bind_texture_unit(slot, texture, sampler, LODGE_TEXTURE_TARGET_2D);
}

void lodge_gfx_bind_texture_cube_map(int slot, const lodge_texture_t texture)
{
	lodge_gfx_bind_texture(slot, texture, LODGE_TEXTURE_TARGET_CUBE_MAP);
}

void lodge_gfx_bind_texture_unit_cube_map(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler)
{
	lodge_gfx_bind_texture_unit(slot, texture, sampler, LODGE_TEXTURE_TARGET_CUBE_MAP);
}

void lodge_gfx_bind_texture_unit_2d_array(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler)
{
	lodge_gfx_bind_texture_unit(slot, texture, sampler, LODGE_TEXTURE_TARGET_2D_ARRAY);
}

void lodge_gfx_bind_texture_unit_3d(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler)
{
	lodge_gfx_bind_texture_unit(slot, texture, sampler, LODGE_TEXTURE_TARGET_3D);
}

#if 0
void lodge_gfx_set_bindings(struct lodge_gfx_bindings *bindings)
{
	ASSERT_OR(bindings) { return; }

	lodge_gfx_bind_shader(bindings->shader);

	for(size_t i = 0, count = bindings->textures.count; i < count; i++) {
		struct lodge_gfx_texture_unit *texture_unit = &bindings->textures.elements[i];

		switch(texture_unit->target) {
		case LODGE_TEXTURE_TARGET_2D:
			lodge_gfx_bind_texture_unit_2d(i, texture_unit->sampler, texture_unit->texture);
			break;
		case LODGE_TEXTURE_TARGET_2D_ARRAY:
			lodge_gfx_bind_texture_unit_2d_array(i, texture_unit->sampler, texture_unit->texture);
			break;
		case LODGE_TEXTURE_TARGET_CUBE_MAP:
			lodge_gfx_bind_texture_unit_cube_map(i, texture_unit->sampler, texture_unit->texture);
			break;
		default:
			ASSERT_NOT_IMPLEMENTED();
			break;
		}
	}
}
#endif

void lodge_gfx_unbind_texture_unit(int slot)
{
	const GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindSampler(slot, 0);
	GL_OK_OR_ASSERT("Failed to unbind texture unit");
}

void lodge_gfx_set_viewport(int32_t x, int32_t y, size_t width, size_t height)
{
	glViewport(x, y, width, height);
}

void lodge_gfx_set_scissor(int32_t x, int32_t y, size_t width, size_t height)
{
	glScissor(x, y, width, height);
}

void lodge_gfx_annotate_begin(strview_t message)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, message.length, message.s);
}

void lodge_gfx_annotate_end()
{
	glPopDebugGroup();
}

