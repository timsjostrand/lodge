#ifndef _LODGE_DEBUG_DRAW_H
#define _LODGE_DEBUG_DRAW_H

#include "math4.h"
#include "geometry.h"

#define LODGE_DEBUG_DRAW_LINES_MAX		(1024 * 8)
#define LODGE_DEBUG_DRAW_SPHERES_MAX	1024

struct lodge_debug_draw;

struct shader;
typedef struct shader* lodge_shader_t;

void	lodge_debug_draw_new_inplace(struct lodge_debug_draw *debug_draw);
void	lodge_debug_draw_free_inplace(struct lodge_debug_draw *debug_draw);
size_t	lodge_debug_draw_sizeof();

void	lodge_debug_draw_update(struct lodge_debug_draw *debug_draw, float dt);
void	lodge_debug_draw_render(struct lodge_debug_draw *debug_draw, lodge_shader_t shader[2], struct mvp mvp);

void	lodge_debug_draw_line(struct lodge_debug_draw *debug_draw, struct line line, vec4 color, float lifetime);
void	lodge_debug_draw_sphere(struct lodge_debug_draw *debug_draw, struct sphere sphere, vec4 color, float lifetime);
void	lodge_debug_draw_aabb_outline(struct lodge_debug_draw *debug_draw, struct aabb aabb, vec4 color, float lifetime);

size_t	lodge_debug_draw_get_line_count(struct lodge_debug_draw *debug_draw);
size_t	lodge_debug_draw_get_sphere_count(struct lodge_debug_draw *debug_draw);

#endif