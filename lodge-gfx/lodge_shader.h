#ifndef _LODGE_SHADER_H
#define _LODGE_SHADER_H

#include "math4.h"
#include "strview.h"

#include <stdbool.h>

#define SHADER_FILENAME_MAX 255

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct lodge_buffer_object;
typedef struct lodge_buffer_object* lodge_buffer_object_t;

void	lodge_shader_new_inplace(lodge_shader_t shader, strview_t name);
void	lodge_shader_free_inplace(lodge_shader_t shader);
size_t	lodge_shader_sizeof();

bool	lodge_shader_set_vertex_source(lodge_shader_t shader, strview_t vertex_source);
bool	lodge_shader_set_fragment_source(lodge_shader_t shader, strview_t fragment_source);
bool	lodge_shader_set_compute_source(lodge_shader_t shader, strview_t compute_source);
bool	lodge_shader_link(lodge_shader_t shader);

int		lodge_shader_get_constant_index(lodge_shader_t shader, strview_t constant_name);

void	lodge_shader_set_constant_float(lodge_shader_t shader, strview_t name, float f);
void	lodge_shader_set_constant_vec2(lodge_shader_t shader, strview_t name, vec2 v);
void	lodge_shader_set_constant_vec3(lodge_shader_t shader, strview_t name, vec3 v);
void	lodge_shader_set_constant_vec4(lodge_shader_t shader, strview_t name, vec4 v);
void	lodge_shader_set_constant_mat4(lodge_shader_t shader, strview_t name, mat4 mat);
void	lodge_shader_set_constant_mvp(lodge_shader_t shader, const struct mvp *mvp);

void	lodge_shader_bind_constant_buffer(lodge_shader_t shader, uint32_t binding, lodge_buffer_object_t buffer_object);
void	lodge_shader_bind_constant_buffer_range(lodge_shader_t shader, uint32_t binding, lodge_buffer_object_t buffer_object, size_t offset, size_t size);

void	lodge_shader_dispatch_compute(uint32_t groups_x, uint32_t groups_y, uint32_t groups_z);

#endif
