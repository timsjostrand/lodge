#ifndef _LODGE_SHADER_H
#define _LODGE_SHADER_H

#include "math4.h"
#include "strview.h"

#include <stdbool.h>

#define SHADER_FILENAME_MAX 255

// TODO(TS): rename `struct lodge_shader`
struct shader;
typedef struct shader* lodge_shader_t;

typedef bool (*lodge_shader_get_source_func_t)(void *userdata, strview_t shader_name, strview_t name, strview_t *source_out);
typedef bool (*lodge_shader_release_source_func_t)(void *userdata, strview_t shader_name, strview_t name);

struct lodge_shader_source_factory
{
	void								*userdata;
	lodge_shader_get_source_func_t		get_func;
	lodge_shader_release_source_func_t	release_func;
};

void	lodge_shader_new_inplace(lodge_shader_t shader, strview_t name, struct lodge_shader_source_factory source_factory);
void	lodge_shader_free_inplace(lodge_shader_t shader);
size_t	lodge_shader_sizeof();

bool	lodge_shader_set_vertex_source(lodge_shader_t shader, strview_t vertex_source);
bool	lodge_shader_set_fragment_source(lodge_shader_t shader, strview_t fragment_source);
bool	lodge_shader_link(lodge_shader_t shader);

int		lodge_shader_get_constant_index(lodge_shader_t shader, strview_t constant_name);

#endif
