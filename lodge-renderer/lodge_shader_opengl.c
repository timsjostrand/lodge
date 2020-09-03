#include "lodge_shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/glew.h>

#include "math4.h"
#include "str.h"
#include "txt.h"
#include "strview.h"
#include "blob.h"
#include "vfs.h"
#include "array.h"
#include "lodge_opengl.h"

#define SHADER_INCLUDES_MAX			255

#define shader_debug(...) debugf("Shader", __VA_ARGS__)
#define shader_error(...) errorf("Shader", __VA_ARGS__)

struct lodge_shader_stage
{
	char								name[SHADER_FILENAME_MAX];
	GLuint								shader;
	array_t								includes;
	txt_t								source;
};

struct shader
{
	GLuint								program;
	char								name[SHADER_FILENAME_MAX];

	struct lodge_shader_stage			vertex_stage;
	struct lodge_shader_stage			fragment_stage;

	struct lodge_shader_source_factory	source_factory;
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

static bool lodge_shader_include(const lodge_shader_t shader, txt_t *txt, size_t start, array_t includes, strview_t include_file)
{
	strview_t include_data = { 0 };
	if(!shader->source_factory.get_func || !shader->source_factory.get_func(shader->source_factory.userdata, strview_wrap(shader->name), include_file, &include_data)) {
		shader_error("Failed to find include file: " STRVIEW_PRINTF_FMT "\n", STRVIEW_PRINTF_ARG(include_file));
		return false;
	}

	if(strview_empty(include_data)) {
		// NOTE(TS): maybe this is a warning and still succeeds? (to keep track of dependencies)
		shader_error("Include file was empty: " STRVIEW_PRINTF_FMT "\n", STRVIEW_PRINTF_ARG(include_file));
		return false;
	}

	*txt = txt_insert(*txt, start, include_data);
	*txt = txt_insert(*txt, start + strview_length(include_data), strview_static("\n"));

	array_append(includes, include_file.s); // NOTE(TS): possibly leaking because of non-null terminated
	
	return true;


#if 0
	size_t include_data_size = 0;
	const char* include_data = vfs_get_file(vfs, strview_make(include_file, strlen(include_file)), &include_data_size);
	if(!include_data) {
		return 0;
	} else {
		*txt = txt_insert(*txt, start, strview_make(include_data, include_data_size));
		*txt = txt_insert(*txt, start + include_data_size, strview_static("\n"));
		array_append(includes, include_file);
		return 1;
	}
#endif
}

static bool lodge_shader_stage_resolve_includes(lodge_shader_t shader, struct lodge_shader_stage *stage, strview_t source)
{
	//
	// TODO(TS): release dependencies
	//
	array_clear(stage->includes);

	stage->source = txt_set(stage->source, source);

	static const int INCLUDE_LENGTH = 9;
	int retry = 1;

	const strview_t global_include_file = strview_static("global.fxh");

	if(!lodge_shader_include(shader, &stage->source, 0, stage->includes, global_include_file)) {
		shader_debug("Could not include global include file: `" STRVIEW_PRINTF_FMT "`\n", STRVIEW_PRINTF_ARG(global_include_file));
	}

	while(retry) {
		retry = 0;

		size_t count = txt_length(stage->source); 
		foreach_line(stage->source, count) {
			if(len == 0) {
				continue;
			}

			size_t post_whitespace = start;
			while(isspace(stage->source[post_whitespace]) && post_whitespace < count) {
				post_whitespace++;
			}

			if(str_begins_with(&stage->source[post_whitespace], INCLUDE_LENGTH, "#include ")) {
				txt_t include_file = txt_new(strview_make(&stage->source[start + INCLUDE_LENGTH], len - INCLUDE_LENGTH));

				// Trim whitespace
				txt_trim(include_file);

				// Strip quotes
				if(txt_begins_with(include_file, strview_static("\""))) {
					txt_delete(include_file, 0, 1);
				}
				if(txt_ends_with(include_file, strview_static("\""))) {
					txt_delete_from_tail(include_file, 1);
				}

				int include_index = array_find_string(stage->includes, include_file, txt_length(include_file));
				if(include_index == -1) {
					shader_debug("Including file: `%s`\n", include_file);

					// Remove `#include` line
					txt_delete(stage->source, start, len);

					if(lodge_shader_include(shader, &stage->source, start, stage->includes, txt_to_strview(include_file))) {
						retry = 1;
					} else {
						shader_error("Could not include file: `%s`\n", include_file);
						txt_free(include_file);
						return false;
					}
				} else {
					shader_debug("Skipping already included file: `%s`\n", include_file);
					txt_delete(stage->source, start, len);
				}

				txt_free(include_file);

				if(retry) {
					shader_debug("Retry include pass\n");
					break;
				}
			}
		}
	};

	return true;
}

static bool lodge_shader_stage_set_source(lodge_shader_t shader, struct lodge_shader_stage *stage, strview_t source)
{
	return lodge_shader_stage_resolve_includes(shader, stage, source);
}

static bool lodge_shader_stage_compile(struct lodge_shader_stage *stage, GLenum stage_type)
{
	ASSERT(glIsShader(stage->shader) == GL_FALSE);

	/* Compile fragment shader. */
	stage->shader = glCreateShader(stage_type);
	const GLint source_length[1] = { (GLint)txt_length(stage->source) }; 
	glShaderSource(stage->shader, 1, &stage->source, source_length);
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
	stage->includes = array_new(SHADER_FILENAME_MAX, SHADER_INCLUDES_MAX);
	stage->source = txt_new(strview_static(""));
}

static void lodge_shader_stage_release_includes(struct lodge_shader_stage *stage, struct lodge_shader_source_factory source_factory, strview_t shader_name)
{
	array_foreach(stage->includes, const char, include) {
		printf("release include: %s\n", include);

		strview_t include_stringview = strview_make_from_str(include, SHADER_FILENAME_MAX);
		source_factory.release_func(source_factory.userdata, shader_name, include_stringview);
	}
}

static void lodge_shader_stage_free_inplace(struct lodge_shader_stage *stage)
{
	if(glIsShader(stage->shader) == GL_TRUE) {
		glDeleteShader(stage->shader);
	}
	txt_free(stage->source);
	// TODO(TS): release dependencies
	array_free(stage->includes);
}

void lodge_shader_new_inplace(lodge_shader_t shader, strview_t name, struct lodge_shader_source_factory source_factory)
{
	*shader = (struct shader) { 0 };
	shader->source_factory = source_factory;

	strbuf_wrap_and(shader->name, strbuf_set, name);

	lodge_shader_stage_new_inplace(&shader->vertex_stage);
	lodge_shader_stage_new_inplace(&shader->fragment_stage);
}

void lodge_shader_free_inplace(lodge_shader_t shader)
{
	// NOTE(TS): shader asset does `lodge_res_clear_dependency` instead. Good?
	//lodge_shader_stage_release_includes(&shader->vertex_stage, shader->source_factory, strview_wrap(shader->name));
	//lodge_shader_stage_release_includes(&shader->fragment_stage, shader->source_factory, strview_wrap(shader->name));

	if(glIsProgram(shader->program) == GL_TRUE) {
		glDeleteProgram(shader->program);
	}
	shader->program = 0;

	lodge_shader_stage_free_inplace(&shader->vertex_stage);
	lodge_shader_stage_free_inplace(&shader->fragment_stage);
}

size_t lodge_shader_sizeof()
{
	return sizeof(struct shader);
}

bool lodge_shader_set_vertex_source(lodge_shader_t shader, strview_t vertex_source)
{
	if(!lodge_shader_stage_set_source(shader, &shader->vertex_stage, vertex_source)) {
		return false;
	}
	return lodge_shader_stage_compile(&shader->vertex_stage, GL_VERTEX_SHADER);
}

bool lodge_shader_set_fragment_source(lodge_shader_t shader, strview_t fragment_source)
{
	if(!lodge_shader_stage_set_source(shader, &shader->fragment_stage, fragment_source)) {
		return false;
	}
	return lodge_shader_stage_compile(&shader->fragment_stage, GL_FRAGMENT_SHADER);
}

bool lodge_shader_link(lodge_shader_t shader)
{
	const bool has_fragment_shader = glIsShader(shader->fragment_stage.shader) == GL_TRUE;
	const bool has_vertex_shader = glIsShader(shader->vertex_stage.shader) == GL_TRUE;

	if(!has_fragment_shader && !has_vertex_shader) {
		shader_error("No fragment or vertex source\n");
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

	glLinkProgram(shader->program);
	const bool link_ret = lodge_shader_program_log(shader->program, shader->name);

	if(!link_ret) {
		return false;
	}

	return true;
}

void lodge_renderer_bind_shader(lodge_shader_t shader)
{
	glUseProgram(shader->program);
	GL_OK_OR_ASSERT("Failed to bind shader");
}

void lodge_renderer_set_constant_float(lodge_shader_t shader, strview_t name, float f)
{
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform1f(shader->program, id, f);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_renderer_set_constant_float(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_renderer_set_constant_vec2(lodge_shader_t shader, strview_t name, vec2 v)
{
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform2f(shader->program, id, v.x, v.y);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_renderer_set_constant_vec2(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_renderer_set_constant_vec3(lodge_shader_t shader, strview_t name, vec3 v)
{
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform3f(shader->program, id, v.x, v.y, v.z);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_renderer_set_constant_vec3(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_renderer_set_constant_vec4(lodge_shader_t shader, strview_t name, vec4 v)
{
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniform4f(shader->program, id, v.x, v.y, v.z, v.w);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_renderer_set_constant_vec4(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_renderer_set_constant_mat4(lodge_shader_t shader, strview_t name, mat4 mat)
{
	const GLint id = glGetUniformLocation(shader->program, name.s);
	if(id == -1) {
		//errorf("OpenGL", "uniform " STRVIEW_PRINTF_FMT " not found\n", STRVIEW_PRINTF_ARG(name));
		return;
	}
	glProgramUniformMatrix4fv(shader->program, id, 1, GL_FALSE, mat.m);
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		errorf("OpenGL", "lodge_renderer_set_constant_mat4(" STRVIEW_PRINTF_FMT ") failed: 0x%04x\n", STRVIEW_PRINTF_ARG(name), err);
	}
}

void lodge_renderer_set_constant_mvp(lodge_shader_t s, const struct mvp *mvp)
{
	lodge_renderer_set_constant_mat4(s, strview_static("model"), mvp->model);
	lodge_renderer_set_constant_mat4(s, strview_static("view"), mvp->view);
	lodge_renderer_set_constant_mat4(s, strview_static("projection"), mvp->projection);
}

int lodge_shader_get_constant_index(lodge_shader_t shader, strview_t constant_name)
{
	return glGetAttribLocation(shader->program, constant_name.s);
}