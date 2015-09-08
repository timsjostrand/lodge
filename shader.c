/**
 * Utilities for loading, compiling and linking shader programs.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"

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
 * @param uniform_names		A list of uniform names in the shader,
 *							excluding 'transform', 'projection' and 'color'.
 * @param uniforms_count	Number of elements in the uniform name list.
 */
int shader_init(struct shader *s,
		char *vert_src, int vert_src_len,
		char *frag_src, int frag_src_len,
		const char **uniform_names, int uniforms_count)
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

	/* Set up global uniforms. */
	s->uniform_transform = glGetUniformLocation(s->program, UNIFORM_NAME_TRANSFORM);
	shader_debug("uniform: %s=%d\n", UNIFORM_NAME_TRANSFORM, s->uniform_transform);
	s->uniform_projection = glGetUniformLocation(s->program, UNIFORM_NAME_PROJECTION);
	shader_debug("uniform: %s=%d\n", UNIFORM_NAME_PROJECTION, s->uniform_projection);
	s->uniform_color = glGetUniformLocation(s->program, UNIFORM_NAME_COLOR);
	shader_debug("uniform: %s=%d\n", UNIFORM_NAME_COLOR, s->uniform_color);
	s->uniform_sprite_type = glGetUniformLocation(s->program, UNIFORM_NAME_SPRITE_TYPE);
	shader_debug("uniform: %s=%d\n", UNIFORM_NAME_SPRITE_TYPE, s->uniform_sprite_type);
	s->uniform_tex = glGetUniformLocation(s->program, UNIFORM_NAME_TEX);
	shader_debug("uniform: %s=%d\n", UNIFORM_NAME_TEX, s->uniform_tex);

	/* Set up user uniforms. */
	/* NOTE: non-existing uniforms do not result in an error being returned. */
	s->uniforms = (GLint *) malloc(uniforms_count * sizeof(GLint));
	for(int i=0; i<uniforms_count; i++) {
		const char *name = uniform_names[i];
		s->uniforms[i] = glGetUniformLocation(s->program, name);
		shader_debug("uniform: %s=%d\n", name, s->uniforms[i]);
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

void shader_free(struct shader *s)
{
	free(s->uniforms);
	if(glIsProgram(s->program) == GL_TRUE) {
		glDeleteProgram(s->program);
	}
}

#if 0
/**
 * TODO: step through code and see if we still actually use/want this, or if all
 * shader thinking should be done per sprite or manually instead.
 */
void shader_think(struct shader *s, struct graphics *g, float delta_time)
{
	/* Upload transform uniform. */
	mat4 transform;
	mult(transform, g->translate, g->scale);
	mult(transform, transform, g->rotate);
	transpose_same(transform);
	glUniformMatrix4fv(s->uniform_transform, 1, GL_FALSE, transform);

	/* Upload projection uniform. */
	glUniformMatrix4fv(s->uniform_projection, 1, GL_FALSE, g->projection);
}
#endif
