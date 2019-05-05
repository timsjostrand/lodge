#ifndef _BASIC_SPRITE_H
#define _BASIC_SPRITE_H

#include "texture.h"
#include "math4.h"

struct graphics;

struct basic_sprite {
	int		type;
	vec4	pos;
	vec4	scale;
	vec4	color;
	float	rotation;
	tex_t	*texture;
};

void		sprite_init(struct basic_sprite *sprite, int type, float x, float y, float z, float w, float h, const vec4 color, float rotation, tex_t *texture);
void		sprite_render(struct basic_sprite *sprite, struct shader *s, struct graphics *g);

#endif