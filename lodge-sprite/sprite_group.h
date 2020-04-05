#ifndef _SPRITE_GROUP_H
#define _SPRITE_GROUP_H

#include "animatedsprites.h"

#define SPRITE_GROUP_MAX_SPRITES	64

struct sprite_group
{
	vec3			position;
	vec3			scale;
	struct sprite	sprites[SPRITE_GROUP_MAX_SPRITES];
	struct sprite	draw_sprites[SPRITE_GROUP_MAX_SPRITES];
	unsigned int	sprites_count;
};

void			animatedsprites_add_group(struct animatedsprites *animatedsprites, struct sprite_group *group);

void			sprite_group_get_size(struct sprite_group *group);
void			sprite_group_update(struct sprite_group *group);
struct sprite*	sprite_group_add_sprite(struct sprite_group *group, struct anim *anim, float relx, float rely,
						float relz, float sx, float sy);
void			sprite_group_clear(struct sprite_group *group);

#endif
