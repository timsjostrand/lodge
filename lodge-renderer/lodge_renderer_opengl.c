#include "lodge_renderer.h"

#include "lodge_opengl.h"
#include "lodge_texture.h"
#include "lodge_sampler.h"
#include "color.h"
#include "drawable.h"

// TODO(TS): this should be shared between all renderers/contexts
struct unit_drawables
{
	struct drawable			rect;
};

struct lodge_renderer
{
	strview_t				library;
	struct unit_drawables	unit_drawables;
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
static struct lodge_ret lodge_renderer_opengl_init(struct lodge_renderer *renderer)
{
	renderer->library = strview_static("OpenGL");
	return lodge_success();
}

static void lodge_renderer_opengl_free(struct lodge_renderer *renderer)
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

static struct lodge_ret unit_drawables_init(struct unit_drawables *ud)
{
	// TODO(TS): should be able to return lodge_ret
	drawable_new_rect_solidf(&ud->rect, -0.5f, -0.5f, 1.0f, 1.0f );
	return lodge_success();
}

static void unit_drawables_reset(struct unit_drawables *ud)
{
	drawable_reset(&ud->rect);
}

static GLenum lodge_texture_target_to_gl(enum lodge_texture_target target)
{
	switch(target)
	{
	case LODGE_TEXTURE_TARGET_2D:
		return GL_TEXTURE_2D;
	case LODGE_TEXTURE_TARGET_2D_ARRAY:
		return GL_TEXTURE_2D_ARRAY;
	case LODGE_TEXTURE_TARGET_CUBE_MAP:
		return GL_TEXTURE_CUBE_MAP;
	default:
		ASSERT("Unknown OpenGL texture target");
		return GL_TEXTURE_2D;
	}
};

struct lodge_renderer* lodge_renderer_new()
{
	struct lodge_renderer *renderer = (struct lodge_renderer *) calloc(1, sizeof(struct lodge_renderer));

	renderer->library = strview_static("OpenGL");

	return renderer;
}

void lodge_renderer_free(struct lodge_renderer *renderer)
{
	//ASSERT_NOT_IMPLEMENTED();
	free(renderer);
}

struct lodge_ret lodge_renderer_attach(struct lodge_renderer *renderer)
{
	/* Init GLEW. */
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
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

	struct lodge_ret unit_drawables_ret = unit_drawables_init(&renderer->unit_drawables);
	if(!unit_drawables_ret.success) {
		return unit_drawables_ret;
	}

	return lodge_success();
}

void lodge_renderer_detach(struct lodge_renderer *renderer)
{
	unit_drawables_reset(&renderer->unit_drawables);
}

strview_t lodge_renderer_get_library(struct lodge_renderer *renderer)
{
	return renderer->library;
}

struct drawable* lodge_renderer_get_unit_rect(struct lodge_renderer *renderer)
{
	return &renderer->unit_drawables.rect;
}

void lodge_renderer_bind_texture(int slot, const lodge_texture_t texture, enum lodge_texture_target target)
{
	const GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(lodge_texture_target_to_gl(target), lodge_texture_to_gl(texture));
	GL_OK_OR_ASSERT("Failed to bind texture");
}

void lodge_renderer_bind_sampler(int slot, const lodge_sampler_t sampler)
{
	glBindSampler(slot, lodge_sampler_to_gl(sampler));
	GL_OK_OR_ASSERT("Failed to bind sampler");
}

void lodge_renderer_bind_texture_unit(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler, enum lodge_texture_target target)
{
	const GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(lodge_texture_target_to_gl(target), lodge_texture_to_gl(texture));
	GL_OK_OR_ASSERT("Failed to bind texture");

	glBindSampler(slot, lodge_sampler_to_gl(sampler));
	GL_OK_OR_ASSERT("Failed to bind sampler");
}

void lodge_renderer_bind_texture_2d(int slot, const lodge_texture_t texture)
{
	lodge_renderer_bind_texture(slot, texture, LODGE_TEXTURE_TARGET_2D);
}

void lodge_renderer_bind_texture_unit_2d(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler)
{
	lodge_renderer_bind_texture_unit(slot, texture, sampler, LODGE_TEXTURE_TARGET_2D);
}

void lodge_renderer_bind_texture_cube_map(int slot, const lodge_texture_t texture)
{
	lodge_renderer_bind_texture(slot, texture, LODGE_TEXTURE_TARGET_CUBE_MAP);
}

void lodge_renderer_bind_texture_unit_cube_map(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler)
{
	lodge_renderer_bind_texture_unit(slot, texture, sampler, LODGE_TEXTURE_TARGET_CUBE_MAP);
}

void lodge_renderer_bind_texture_unit_2d_array(int slot, const lodge_texture_t texture, const lodge_sampler_t sampler)
{
	lodge_renderer_bind_texture_unit(slot, texture, sampler, LODGE_TEXTURE_TARGET_2D_ARRAY);
}

void lodge_renderer_unbind_texture_unit(int slot)
{
	const GLenum slot_opengl = (GLenum)((GLint)GL_TEXTURE0 + (GLint)slot);
	glActiveTexture(slot_opengl);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindSampler(slot, 0);
	GL_OK_OR_ASSERT("Failed to unbind texture unit");
}

void lodge_renderer_annotate_begin(strview_t message)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, message.length, message.s);
}

void lodge_renderer_annotate_end()
{
	glPopDebugGroup();
}

