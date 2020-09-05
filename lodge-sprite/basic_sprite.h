#ifndef _BASIC_SPRITE_H
#define _BASIC_SPRITE_H

#include "math4.h"

struct graphics;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct basic_sprite {
	int						type;
	vec4					pos;
	vec4					scale;
	vec4					color;
	float					rotation;
	lodge_texture_t			texture;
};

void		sprite_init(struct basic_sprite *sprite, int type, float x, float y, float z, float w, float h, const vec4 color, float rotation, lodge_texture_t texture);
void		sprite_render(struct basic_sprite *sprite, lodge_shader_t s, mat4 projection, struct lodge_renderer *renderer);

#endif
