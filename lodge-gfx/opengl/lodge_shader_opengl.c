#include "lodge_shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/glew.h>

#include "math4.h"
#include "str.h"
#include "strview.h"
#include "blob.h"
#include "array.h"
#include "lodge_opengl.h"

#define SHADER_INCLUDES_MAX			255

#define shader_debug(...) debugf("Shader", __VA_ARGS__)
#define shader_error(...) errorf("Shader", __VA_ARGS__)

struct lodge_shader_stage
{
	char								name[SHADER_FILENAME_MAX];
	GLuint								shader;
	strview_t							source;
};

struct lodge_shader
{
	GLuint								program;
	char								name[SHADER_FILENAME_MAX];

	struct lodge_shader_stage			vertex_stage;
	struct lodge_shader_stage			fragment_stage;
	struct lodge_shader_stage			compute_stage;
};

static bool lodge_shader_program_log(GLuint program, const char* name)
{
	shader_debug("=== %s: program ===\n", name);

	GLint status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if(status == GL_FALSE) {
		shader_error("Link failed\n");
	}

	GLint len = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

	if(len > 0) {
		GLchar *msg = (GLchar *) malloc(len);
		glGetProgramInfoLog(program, len, &len, msg);
		/* TODO: readline() and output for each line */
		if(status == GL_FALSE) {
			shader_error("%s", msg);
		} else {
			shader_debug("%s", msg);
		}
		free(msg);
	}

	GLint uniforms = 0;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniforms);
	shader_debug("%d active uniforms\n", uniforms);

	if(status == GL_FALSE) {
		return false;
	}

	return true;
}

static bool lodge_shader_log(GLuint shader, const char *name, const char* label)
{
	shader_debug("=== %s: %s ===\n", name, label);

	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if(status == GL_FALSE) {
		shader_error("Compilation of `%s` failed\n", name);
	}

	GLint len = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

	if(len > 0) {
		GLchar *msg = (GLchar *) malloc(len);
		glGetShaderInfoLog(shader, len, &len, msg);
		/* TODO: readline() and output for each line */
		if(status == GL_FALSE) {
			shader_error("%s", msg);
		} else {
			shader_debug("%s", msg);
		}
		free(msg);
	}

	if(status == GL_FALSE) {
		glDeleteShader(shader);
		return false;
	}

	return true;
}

// FIXME(TS): remove this helper
static bool lodge_shader_stage_set_source(lodge_shader_t shader, struct lodge_shader_stage *stage, strview_t source)
{
	stage->source = source;
	return true;
}

static bool lodge_shader_stage_compile(struct lodge_shader_stage *stage, GLenum stage_type)
{
	ASSERT(glIsShader(stage->shader) == GL_FALSE);

	/* Compile fragment shader. */
	stage->shader = glCreateShader(stage_type);
	const GLint source_length[1] = { stage->source.length }; 
	glShaderSource(stage->shader, 1, &stage->source.s, source_length);
	glCompileShader(stage->shader);
	if(!lodge_shader_log(stage->shader, stage->name, "shader stage compile")) {
		return false;
	}

	return true;
}

static void lodge_shader_stage_new_inplace(struct lodge_shader_stage *stage)
{
	stage->name[0] = '\0';
	stage->shader = 0;
	stage->source = strview_static("");
}

static void lodge_shader_stage_free_inplace(struct lodge_shader_stage *stage)
{
	if(glIsShader(stage->shader) == GL_TRUE) {
		glDeleteShader(stage->shader);
	}
}

void lodge_shader_new_inplace(lodge_shader_t shader, strview_t name)
{
	memset(shader, 0, sizeof(struct lodge_shader));

	strbuf_wrap_and(shader->name, strbuf_set, name);

	lodge_shader_stage_new_inplace(&shader->vertex_stage);
	lodge_shader_stage_new_inplace(&shader->fragment_stage);
	lodge_shader_stage_new_inplace(&shader->compute_stage);
}

void lodge_shader_free_inplace(lodge_shader_t shader)
{
	if(glIsProgram(shader->program) == GL_TRUE) {
		glDeleteProgram(shader->program);
	}
	shader->program = 0;

	lodge_shader_stage_free_inplace(&shader->compute_stage);
	lodge_shader_stage_free_inplace(&shader->fragment_stage);
	lodge_shader_stage_free_inplace(&shader->vertex_stage);
}

size_t lodge_shader_sizeof()
{
	return sizeof(struct lodge_shader);
}

bool lodge_shader_set_vertex_source(lodge_shader_t shader, strview_t vertex_source)
{
	strbuf_setf(strbuf_wrap(shader->vertex_stage.name), "%s.vert", shader->name);
	if(!lodge_shader_stage_set_source(shader, &shader->vertex_stage, vertex_source)) {
		return false;
	}
	return lodge_shader_stage_compile(&shader->vertex_stage, GL_VERTEX_SHADER);
}

bool lodge_shader_set_fragment_source(lodge_shader_t shader, strview_t fragment_source)
{
	strbuf_setf(strbuf_wrap(shader->fragment_stage.name), "%s.frag", shader->name);
	if(!lodge_shader_stage_set_source(shader, &shader->fragment_stage, fragment_source)) {
		return false;
	}
	return lodge_shader_stage_compile(&shader->fragment_stage, GL_FRAGMENT_SHADER);
}

bool lodge_shader_set_compute_source(lodge_shader_t shader, strview_t compute_source)
{
	strbuf_setf(strbuf_wrap(shader->compute_stage.name), "%s.compute", shader->name);
	if(!lodge_shader_stage_set_source(shader, &shader->compute_stage, compute_source)) {
		return false;
	}
	return lodge_shader_stage_compile(&shader->compute_stage, GL_COMPUTE_SHADER);
}

bool lodge_shader_link(lodge_shader_t shader)
{
	const bool has_fragment_shader = glIsShader(shader->fragment_stage.shader) == GL_TRUE;
	const bool has_vertex_shader = glIsShader(shader->vertex_stage.shader) == GL_TRUE;
	const bool has_compute_shader = glIsShader(shader->compute_stage.shader) == GL_TRUE;

	if(!has_fragment_shader
		&& !has_vertex_shader
		&& !has_compute_shader) {
		ASSERT_FAIL("Unable to link shader -- has no stages (fragment, vertex or compute)");
		return false;
	}

	// Link shader program
	shader->program = glCreateProgram();

	if(has_fragment_shader) {
		glAttachShader(shader->program, shader->fragment_stage.shader);
	}
	if(has_vertex_shader) {
		glAttachShader(shader->program, shader->vertex_stage.shader);
	}
	if(has_compute_shader) {
		glAttachShader(shader->program, shader->compute_stage.shader);
	}

	glLinkProgram(shader->program);
	const bool link_ret = lodge_shader_program_log(shader->program, shader->name);

	if(!link_ret) {
		return false;
	}

	return true;
}

void lodge_gfx_bind_shader(lodge_shader_t shader)
{
	ASSERT(shader && shader->program);
	glUseProgram(shader ? shader->program : 0);
	GL_OK_OR_ASSERT("Failed to bind shader");
}

void lodge_shader_set_constant_float(lodge_shader_t shader, strview_t name, float f)
{
	ASSERT(shader);
	if(!shader) {
		return;
	}
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform1f(shader->program, id, f);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_shader_set_constant_float(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_shader_set_constant_vec2(lodge_shader_t shader, strview_t name, vec2 v)
{
	ASSERT(shader);
	if(!shader) {
		return;
	}
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform2f(shader->program, id, v.x, v.y);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_shader_set_constant_vec2(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_shader_set_constant_vec3(lodge_shader_t shader, strview_t name, vec3 v)
{
	ASSERT(shader);
	if(!shader) {
		return;
	}
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform3f(shader->program, id, v.x, v.y, v.z);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_shader_set_constant_vec3(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_shader_set_constant_vec4(lodge_shader_t shader, strview_t name, vec4 v)
{
	ASSERT(shader);
	if(!shader) {
		return;
	}
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform4f(shader->program, id, v.x, v.y, v.z, v.w);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_shader_set_constant_vec4(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_shader_set_constant_mat4(lodge_shader_t shader, strview_t name, mat4 mat)
{
	ASSERT(shader);
	if(!shader) {
		return;
	}
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniformMatrix4fv(shader->program, id, 1, GL_FALSE, mat.m);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_shader_set_constant_mat4(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_shader_set_constant_mvp(lodge_shader_t shader, const struct mvp *mvp)
{
	ASSERT(shader);
	if(!shader) {
		return;
	}
	lodge_shader_set_constant_mat4(shader, strview_static("model"), mvp->model);
	lodge_shader_set_constant_mat4(shader, strview_static("view"), mvp->view);
	lodge_shader_set_constant_mat4(shader, strview_static("projection"), mvp->projection);
}

void lodge_shader_bind_constant_buffer(lodge_shader_t shader, uint32_t binding, lodge_buffer_object_t buffer_object)
{
	lodge_gfx_bind_shader(shader);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, lodge_buffer_object_to_gl(buffer_object));
	GL_OK_OR_ASSERT("lodge_shader_bind_constant_buffer");
}

void lodge_shader_bind_constant_buffer_range(lodge_shader_t shader, uint32_t binding, lodge_buffer_object_t buffer_object, size_t offset, size_t size)
{
#if 0
	GLint aligment;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &aligment);
#endif
	lodge_gfx_bind_shader(shader);
	glBindBufferRange(GL_UNIFORM_BUFFER, binding, lodge_buffer_object_to_gl(buffer_object), offset, size);
	GL_OK_OR_ASSERT("lodge_shader_bind_constant_buffer_range");
}

int lodge_shader_get_constant_index(lodge_shader_t shader, strview_t constant_name)
{
	ASSERT(shader);
	if(!shader) {
		return 0;
	}
	return glGetAttribLocation(shader->program, constant_name.s);
}

void lodge_shader_dispatch_compute(uint32_t groups_x, uint32_t groups_y, uint32_t groups_z)
{
	glDispatchCompute(groups_x, groups_y, groups_z);
	GL_OK_OR_ASSERT("lodge_shader_dispatch_compute");
}
