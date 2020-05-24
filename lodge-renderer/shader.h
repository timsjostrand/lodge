#ifndef _SHADER_H
#define _SHADER_H

#include <GL/glew.h>

#include "math4.h"
#include "log.h"
#include "str.h"
#include "txt.h"
#include "array.h"

#define shader_debug(...) debugf("Shader", __VA_ARGS__)
#define shader_error(...) errorf("Shader", __VA_ARGS__)

#define SHADER_OK					 0
#define SHADER_COMPILE_ERROR		-1
#define SHADER_LINK_ERROR			-2
#define SHADER_OOM_ERROR			-3
#define SHADER_UNIFORMS_MAX_ERROR	-4
#define SHADER_INCLUDE_ERROR		-5

#define SHADER_FILENAME_MAX			255
#define SHADER_UNIFORM_NAME_MAX_LEN	255

#define ATTRIB_NAME_POSITION		"vertex_in"
#define ATTRIB_NAME_TEXCOORD		"texcoord_in"
#define ATTRIB_NAME_TANGENT			"tangent_in"
#define ATTRIB_NAME_BITANGENT		"bitangent_in"

#define TYPE_VEC_1F	                0
#define TYPE_VEC_2F	                1
#define TYPE_VEC_3F	                2
#define TYPE_VEC_4F	                3
//#define TYPE_MAT_2F	            4
//#define TYPE_MAT_3F               5
#define TYPE_MAT_4F	                6
#define TYPE_VEC_1I	                7

#define UNIFORMS_MAX                64

struct uniform {
	char			name[SHADER_UNIFORM_NAME_MAX_LEN];
    GLint			id;
    int				datatype;

    void			*data;
	char			constant_data[64];
};

struct shader {
	int				initialized;
	char			name[SHADER_FILENAME_MAX];

	strview_t		vert_src;
	strview_t		frag_src;
	
	txt_t			vert_transformed;
	txt_t			frag_transformed;

	array_t			vert_includes;					// Included files in the vertex source
	array_t			frag_includes;					// Included files in the fragment source

	GLuint			program;

	struct uniform	*uniforms[UNIFORMS_MAX];
};

int     shader_init(struct shader *s,
				const char* name,
				const strview_t vert_src,
                const strview_t frag_src,
				struct vfs *vfs);
void    shader_delete(struct shader *s);
void    shader_free(struct shader *s);

int		shader_uniform(struct shader *s, const char *name, void *data, int type);
int		shader_uniform1f(struct shader *s, const char *name, float *data);
int		shader_uniform2f(struct shader *s, const char *name, vec2 *data);
int		shader_uniform3f(struct shader *s, const char *name, vec3 *data);
int		shader_uniform4f(struct shader *s, const char *name, vec4 *data);
int		shader_uniform1i(struct shader *s, const char *name, int* data);

int		shader_uniform_matrix4f(struct shader *s, const char *name, mat4 *data);

void	shader_constant_uniform1i(struct shader *s, const char *name, int data);
void	shader_constant_uniform4f(struct shader *s, const char *name, vec4 data);

void	shader_uniforms_relocate(struct shader *s);
void	shader_uniforms_free(struct shader *s);
void    shader_uniforms_think(struct shader *s);

#endif
