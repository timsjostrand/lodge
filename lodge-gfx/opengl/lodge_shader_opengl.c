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
	char								*compile_info;
};

struct lodge_shader
{
	GLuint								program;
	char								name[SHADER_FILENAME_MAX];

	struct lodge_shader_stage			vertex_stage;
	struct lodge_shader_stage			fragment_stage;
	struct lodge_shader_stage			compute_stage;

	char								*link_info;
};

static bool lodge_shader_check_link_status(struct lodge_shader *shader)
{
	GLint shader_link_status = 0;
	glGetProgramiv(shader->program, GL_LINK_STATUS, &shader_link_status);

	//
	// NOTE(TS): get early notification that compilation failed.
	//
	ASSERT(shader_link_status != GL_FALSE);

	GLint len = 0;
	glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &len);
	if(len > 0) {
		shader->link_info = malloc(len);
		glGetProgramInfoLog(shader->program, len, &len, shader->link_info);
	}

	return shader_link_status != GL_FALSE;
}

static bool lodge_shader_stage_check_compile_status(struct lodge_shader_stage *shader_stage)
{
	GLint shader_compile_status = 0;
	glGetShaderiv(shader_stage->shader, GL_COMPILE_STATUS, &shader_compile_status);
	
	//
	// NOTE(TS): get early notification that compilation failed.
	//
	ASSERT(shader_compile_status != GL_FALSE);

	GLint len = 0;
	glGetShaderiv(shader_stage->shader, GL_INFO_LOG_LENGTH, &len);
	if(len > 0) {
		shader_stage->compile_info = malloc(len);
		glGetShaderInfoLog(shader_stage->shader, len, &len, shader_stage->compile_info);
	}

	if(shader_compile_status == GL_FALSE) {
		glDeleteShader(shader_stage->shader);
		return false;
	}

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
	if(!lodge_shader_stage_check_compile_status(stage)) {
		return false;
	}

	return true;
}

static void lodge_shader_stage_new_inplace(struct lodge_shader_stage *stage)
{
	stage->name[0] = '\0';
	stage->shader = 0;
	stage->source = strview_static("");
	stage->compile_info = NULL;
}

static void lodge_shader_stage_free_inplace(struct lodge_shader_stage *stage)
{
	if(stage->compile_info) {
		free(stage->compile_info);
	}
	if(glIsShader(stage->shader) == GL_TRUE) {
		glDeleteShader(stage->shader);
	}
}

void lodge_shader_new_inplace(struct lodge_shader *shader, strview_t name)
{
	memset(shader, 0, sizeof(struct lodge_shader));

	strbuf_wrap_and(shader->name, strbuf_set, name);

	lodge_shader_stage_new_inplace(&shader->vertex_stage);
	lodge_shader_stage_new_inplace(&shader->fragment_stage);
	lodge_shader_stage_new_inplace(&shader->compute_stage);

	shader->link_info = NULL;
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

	if(shader->link_info) {
		free(shader->link_info);
	}
}

size_t lodge_shader_sizeof()
{
	return sizeof(struct lodge_shader);
}

bool lodge_shader_set_vertex_source(lodge_shader_t shader, strview_t vertex_source)
{
	strbuf_setf(strbuf_wrap(shader->vertex_stage.name), "%s.vert", shader->name);
	shader->vertex_stage.source = vertex_source;
	return lodge_shader_stage_compile(&shader->vertex_stage, GL_VERTEX_SHADER);
}

bool lodge_shader_set_fragment_source(lodge_shader_t shader, strview_t fragment_source)
{
	strbuf_setf(strbuf_wrap(shader->fragment_stage.name), "%s.frag", shader->name);
	shader->fragment_stage.source = fragment_source;
	return lodge_shader_stage_compile(&shader->fragment_stage, GL_FRAGMENT_SHADER);
}

bool lodge_shader_set_compute_source(lodge_shader_t shader, strview_t compute_source)
{
	strbuf_setf(strbuf_wrap(shader->compute_stage.name), "%s.compute", shader->name);
	shader->compute_stage.source = compute_source;
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

	//
	// Set up shader stages
	//
	{
		shader->program = glCreateProgram();

		int stages_count = 0;
		if(has_fragment_shader) {
			glAttachShader(shader->program, shader->fragment_stage.shader);
			stages_count++;
		}
		if(has_vertex_shader) {
			glAttachShader(shader->program, shader->vertex_stage.shader);
			stages_count++;
		}
		if(has_compute_shader) {
			glAttachShader(shader->program, shader->compute_stage.shader);
			stages_count++;
		}
		ASSERT_OR(stages_count > 0) {
			return false;
		}
	}

	//
	// Link the shader stages into a program
	//
	{
		glLinkProgram(shader->program);
	}

	const bool link_ret = lodge_shader_check_link_status(shader);
	ASSERT_OR(link_ret) {
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

void lodge_shader_set_constant_bool(lodge_shader_t shader, strview_t name, bool value)
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
	glProgramUniform1i(shader->program, id, (GLint)value);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_shader_set_constant_bool(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
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
