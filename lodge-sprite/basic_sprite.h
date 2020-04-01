#ifndef _BASIC_SPRITE_H
#define _BASIC_SPRITE_H

#include "lodge_texture.h"
#include "math4.h"

struct graphics;

struct basic_sprite {
	int						type;
	vec4					pos;
	vec4					scale;
	vec4					color;
	float					rotation;
	lodge_texture_t			texture;
	struct lodge_renderer	*renderer;
};

void		sprite_init(struct basic_sprite *sprite, int type, float x, float y, float z, float w, float h, const vec4 color, float rotation, lodge_texture_t texture, struct lodge_renderer *renderer);
void		sprite_render(struct basic_sprite *sprite, struct shader *s, mat4 projection);

#endif