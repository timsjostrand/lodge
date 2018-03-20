/**
 * Utilities for loading, compiling and linking shader programs.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "shader.h"
#include "math4.h"

int shader_program_log(GLuint program, const char *name)
{
	shader_debug("=== %s ===\n", name);

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
		return SHADER_LINK_ERROR;
	}

	return SHADER_OK;
}

int shader_log(GLuint shader, const char *name)
{
	shader_debug("=== %s ===\n", name);

	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if(status == GL_FALSE) {
		shader_error("Compilation of %s failed\n", name);
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
		return SHADER_COMPILE_ERROR;
	}

	return SHADER_OK;
}

/**
 * Compiles and links a new shader program.
 *
 * @param s					The shader struct to store into.
 * @param vert_src			Vertex shader source.
 * @param frag_src			Fragment shader source.
 */
int shader_init(struct shader *s,
		char *vert_src, int vert_src_len,
		char *frag_src, int frag_src_len)
{
	int ret = 0;

	/* Make sure struct is up-to-date. */
	s->vert_src = vert_src;
	s->vert_src_len = vert_src_len;
	s->frag_src = frag_src;
	s->frag_src_len = frag_src_len;

	/* FIXME: make sure string ends with \0. */
	vert_src[vert_src_len-1] = '\0';
	frag_src[frag_src_len-1] = '\0';

	/* Compile vertex shader. */
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vert_src, NULL);
	glCompileShader(vs);
	ret = shader_log(vs, "vertex shader");

	if(ret != SHADER_OK) {
		return ret;
	}

	/* Compile fragment shader. */
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &frag_src, NULL);
	glCompileShader(fs);
	ret = shader_log(fs, "fragment shader");

	if(ret != SHADER_OK) {
		return ret;
	}

	/* Compile shader. */
	s->program = glCreateProgram();
	glAttachShader(s->program, fs);
	glAttachShader(s->program, vs);
	glLinkProgram(s->program);
	glDeleteShader(vs);
	glDeleteShader(fs);
	ret = shader_program_log(s->program, "program");

	if(ret != SHADER_OK) {
		return ret;
	}

	/* Position stream. */
	GLint posAttrib = glGetAttribLocation(s->program, ATTRIB_NAME_POSITION);
	shader_debug("attrib: %s=%d\n", ATTRIB_NAME_POSITION, posAttrib);
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

	/* Texcoord stream. */
	GLint texcoordAttrib = glGetAttribLocation(s->program, ATTRIB_NAME_TEXCOORD);
	shader_debug("attrib: %s=%d\n", ATTRIB_NAME_TEXCOORD, texcoordAttrib);
	glEnableVertexAttribArray(texcoordAttrib);
	glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
			(void*) (3 * sizeof(float)));

	return SHADER_OK;
}

void shader_delete(struct shader *s)
{
	if(glIsProgram(s->program) == GL_TRUE) {
		glDeleteProgram(s->program);
	}
}

void shader_free(struct shader *s)
{
	shader_uniforms_free(s);
	shader_delete(s);
}

/**
 * Updates the location of all uniforms in a shader.
 */
void shader_uniforms_relocate(struct shader *s)
{
	glUseProgram(s->program);

	for(int i=0; i<UNIFORMS_MAX; i++) {
		struct uniform *u = s->uniforms[i];

		if(u == NULL) {
			continue;
		} else {
			int old_id = u->id;
			u->id = glGetUniformLocation(s->program, u->name);
			if(u->id != old_id) {
				shader_debug("Relocated uniform \"%s\", id: %d => %d\n", u->name, old_id, u->id);
			}
		}
	}
}

/**
 * Find the next unused uniform index in this shader.
 */
static int shader_uniform_idx_next(struct shader *s)
{
	for(int i=0; i<UNIFORMS_MAX; i++) {
		if(s->uniforms[i] == NULL) {
			return i;
		}
	}
	return -1;
}

static int shader_uniform_create(struct shader *s, const char *name, struct uniform** out)
{
	int index = shader_uniform_idx_next(s);
	if (index < 0) {
		shader_error("UNIFORMS_MAX reached\n");
		return SHADER_UNIFORMS_MAX_ERROR;
	}

	struct uniform *u = (struct uniform *) malloc(sizeof(struct uniform));

	if (u == NULL) {
		shader_error("Out of memory");
		return SHADER_OOM_ERROR;
	}

	strcpy(u->name, name);
	s->uniforms[index] = u;

	*out = u;

	return SHADER_OK;
}

int shader_uniform_init(struct shader *s, struct uniform* uniform, void* data, int type)
{
	glUseProgram(s->program);

	uniform->datatype = type;
	uniform->data = data;
	uniform->id = glGetUniformLocation(s->program, uniform->name);

	return SHADER_OK;
}

int shader_uniform(struct shader *s, const char *name, void *data, int type)
{
	struct uniform* uniform = NULL;
	int result = shader_uniform_create(s, name, &uniform );
	if( result != SHADER_OK )
	{
		return result;
	}
	return shader_uniform_init(s, uniform, data, type);
}

int shader_uniform1f(struct shader *s, const char *name, float *data)
{
	return shader_uniform(s, name, (void *) data, TYPE_VEC_1F);
}

int shader_uniform2f(struct shader *s, const char *name, vec2 *data)
{
	return shader_uniform(s, name, (void *) data, TYPE_VEC_2F);
}

int shader_uniform3f(struct shader *s, const char *name, vec3 *data)
{
	return shader_uniform(s, name, (void *) data, TYPE_VEC_3F);
}

int shader_uniform4f(struct shader *s, const char *name, vec4 *data)
{
	return shader_uniform(s, name, (void *) data, TYPE_VEC_4F);
}

int shader_uniform1i(struct shader *s, const char *name, int* data)
{
	return shader_uniform(s, name, (void *)data, TYPE_VEC_1I);
}

int shader_uniform_matrix4f(struct shader *s, const char *name, mat4 *data)
{
	return shader_uniform(s, name, (void *) data, TYPE_MAT_4F);
}

void shader_constant_uniform1i(struct shader *s, const char *name, int data)
{
	struct uniform* uniform = NULL;
	int result = shader_uniform_create(s, name, &uniform);
	if( result != SHADER_OK )
	{
		return;
	}
	memcpy(uniform->constant_data, &data, sizeof(int));
	shader_uniform_init(s, uniform, (void*)uniform->constant_data, TYPE_VEC_1I);
}

void shader_constant_uniform4f(struct shader *s, const char *name, vec4 data)
{
	struct uniform* uniform = NULL;
	int result = shader_uniform_create(s, name, &uniform);
	if( result != SHADER_OK )
	{
		return;
	}
	memcpy(uniform->constant_data, &data, sizeof(float)*4);
	shader_uniform_init(s, uniform, (void*)uniform->constant_data, TYPE_VEC_4F);
}

void shader_uniforms_free(struct shader *s)
{
	for(int i=0; i<UNIFORMS_MAX; i++) {
		if(s->uniforms[i] != NULL) {
			free(s->uniforms[i]);
		}
		s->uniforms[i] = NULL;
	}
}

void shader_uniforms_think(struct shader *s, float delta_time)
{
	glUseProgram(s->program);

	for(int i=0; i<UNIFORMS_MAX; i++) {
		struct uniform *u = s->uniforms[i];

		if(u == NULL) {
			continue;
		}

		switch(u->datatype) {
		case TYPE_VEC_1F: {
			glUniform1f(u->id, *((GLfloat *) u->data));
			break;
		}
		case TYPE_VEC_2F: {
			float *v = u->data;
			glUniform2f(u->id, v[0], v[1]);
			break;
		}
		case TYPE_VEC_3F: {
			vec3 *v = (vec3 *) u->data;
			glUniform3f(u->id, v->x, v->y, v->z);
			break;
		}
		case TYPE_VEC_4F: {
			vec4 *v = (vec4 *) u->data;
			glUniform4f(u->id, v->x, v->y, v->z, v->w);
			break;
		}
		case TYPE_MAT_4F: {
			glUniformMatrix4fv(u->id, 1, GL_FALSE, (float *) u->data);
			break;
		}
		case TYPE_VEC_1I: {
			glUniform1i(u->id, *((GLint*)u->data));
			break;
		}
		default:
			shader_debug("Unknown datatype for uniform \"%s\"\n", u->name);
			break;
		}
	}
}
