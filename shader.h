#ifndef _SHADER_H
#define _SHADER_H

#ifdef DEBUG
#define shader_debug(...) fprintf(stderr, "DEBUG @ Shader: " __VA_ARGS__)
#else
#define shader_debug(...) do {} while (0)
#endif

#define shader_error(...) fprintf(stderr, "ERROR @ Shader: " __VA_ARGS__)

#define SHADER_OK					 0
#define SHADER_COMPILE_ERROR		-1
#define SHADER_LINK_ERROR			-2

#define UNIFORM_NAME_TRANSFORM		"transform"
#define UNIFORM_NAME_PROJECTION		"projection"
#define UNIFORM_NAME_COLOR			"color"
#define UNIFORM_NAME_SPRITE_TYPE	"sprite_type"
#define UNIFORM_NAME_TEX			"tex"

#define ATTRIB_NAME_POSITION		"vp"
#define ATTRIB_NAME_TEXCOORD		"texcoord_in"

struct shader {
	const char	*vert_src;
	int			vert_src_len;
	const char	*frag_src;
	int			frag_src_len;
	GLuint		program;
	GLint		uniform_transform;
	GLint		uniform_projection;
	GLint		uniform_color;
	GLint		uniform_sprite_type;
	GLint		uniform_tex;
	GLint		*uniforms;
};

int  shader_init(struct shader *s,
		char *vert_src, int vert_src_len,
		char *frag_src, int frag_src_len,
		const char **uniform_names, int uniforms_count);
void shader_free(struct shader *s);

#endif
