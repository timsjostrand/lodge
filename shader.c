/**
 * Utilities for loading, compiling and linking shader programs.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/glew.h>

#include "shader.h"
#include "math4.h"
#include "str.h"
#include "blob.h"
#include "vfs.h"
#include "array.h"

static int shader_program_log(GLuint program, const char* name)
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
		return SHADER_LINK_ERROR;
	}

	return SHADER_OK;
}

int shader_log(GLuint shader, const char *name, const char* label)
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
		return SHADER_COMPILE_ERROR;
	}

	return SHADER_OK;
}

static int shader_include(txt_t *txt, size_t start, array_t includes, const char* include_file)
{
	size_t include_data_size = 0;
	const char* include_data = vfs_get_file(include_file, &include_data_size);
	if(!include_data) {
		return 0;
	} else {
		*txt = txt_insert(*txt, start, strview_make(include_data, include_data_size));
		*txt = txt_insert(*txt, start + include_data_size, strview_static("\n"));
		array_append(includes, include_file);
		return 1;
	}
}

static int shader_resolve_includes_str(txt_t txt, array_t includes, txt_t *out)
{
	static const int INCLUDE_LENGTH = 9;
	int retry = 1;

	static const char* global_include_file = "global.fxh";
	if(!shader_include(&txt, 0, includes, global_include_file)) {
		shader_debug("Could not include file: `%s`\n", global_include_file);
	}

	while(retry) {
		retry = 0;

		size_t count = txt_length(txt); 
		foreach_line(txt, count) {
			if(len == 0) {
				continue;
			}

			size_t post_whitespace = start;
			while(isspace(txt[post_whitespace]) && post_whitespace < count) {
				post_whitespace++;
			}

			if(str_begins_with(&txt[post_whitespace], INCLUDE_LENGTH, "#include ")) {
				txt_t include_file = txt_new(strview_make(&txt[start + INCLUDE_LENGTH], len - INCLUDE_LENGTH));

				// Trim whitespace
				txt_trim(include_file);

				// Strip quotes
				if(txt_begins_with(include_file, strview_static("\""))) {
					txt_delete(include_file, 0, 1);
				}
				if(txt_ends_with(include_file, strview_static("\""))) {
					txt_delete_from_tail(include_file, 1);
				}

				int include_index = array_find_string(includes, include_file, txt_length(include_file));
				if(include_index == -1) {
					shader_debug("Including file: `%s`\n", include_file);

					// Remove `#include` line
					txt_delete(txt, start, len);

					if(shader_include(&txt, start, includes, include_file)) {
						retry = 1;
					} else {
						shader_error("Could not include file: `%s`\n", include_file);
						txt_free(include_file);
						return SHADER_INCLUDE_ERROR;
					}
				} else {
					shader_debug("Skipping already included file: `%s`\n", include_file);
					txt_delete(txt, start, len);
				}

				txt_free(include_file);

				if(retry) {
					shader_debug("Retry include pass\n");
					break;
				}
			}
		}
	};

	*out = txt;
	return SHADER_OK;
}

static int shader_resolve_includes(struct shader *s)
{
	array_clear(s->vert_includes);
	array_clear(s->frag_includes);

	int success = shader_resolve_includes_str(s->vert_transformed, s->vert_includes, &s->vert_transformed) == SHADER_OK;
	success &= shader_resolve_includes_str(s->frag_transformed, s->frag_includes, &s->frag_transformed) == SHADER_OK;

	return success ? SHADER_OK : SHADER_INCLUDE_ERROR;
}

/**
 * Compiles and links a new shader program.
 *
 * @param s					The shader struct to store into.
 * @param vert_src			Vertex shader source.
 * @param frag_src			Fragment shader source.
 */
int shader_init(struct shader *s,
	const char *name,
	const strview_t vert_src,
	const strview_t frag_src)
{
	int ret = 0;

	strncpy(s->name, name, sizeof(s->name));

	s->vert_src = vert_src;
	s->frag_src = frag_src;

	// FIXME(TS): leaking when reloading atm
	s->vert_includes = array_create(SHADER_FILENAME_MAX, 32);
	s->frag_includes = array_create(SHADER_FILENAME_MAX, 32);

	// FIXME(TS): leaking when reloading atm
	s->vert_transformed = txt_new(vert_src);
	s->frag_transformed = txt_new(frag_src);

	if(shader_resolve_includes(s) != SHADER_OK) {
		return ret;
	}

	/* Compile vertex shader. */
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &s->vert_transformed, NULL);
	glCompileShader(vs);
	ret = shader_log(vs, s->name, "vertex shader");

	if(ret != SHADER_OK) {
		return ret;
	}

	/* Compile fragment shader. */
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &s->frag_transformed, NULL);
	glCompileShader(fs);
	ret = shader_log(fs, s->name, "fragment shader");

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
	ret = shader_program_log(s->program, s->name);

	if(ret != SHADER_OK) {
		return ret;
	}

	s->initialized = 1;

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

	txt_free(s->frag_transformed);
	txt_free(s->vert_transformed);

	array_destroy(s->vert_includes);
	array_destroy(s->frag_includes);
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

void shader_uniforms_think(struct shader *s)
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
