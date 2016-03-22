/**
 * An extension of the sprite struct from animatedsprites.c.
 *
 * Sprite groups allows sprites to be organized in groups with a common origo,
 * and added in bulk to a spritebatcher.
 *
 * The positions of the individual sprites in the sprite group struct must be
 * continuously updated using the sprite_group_update() function.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <string.h>

#include "math4.h"
#include "sprite_group.h"

void sprite_group_update(struct sprite_group* group)
{
	for(int i=0; i<group->sprites_count; i++) {
		struct sprite *draw_sprite = &group->draw_sprites[i];
		struct sprite *src_sprite = &group->sprites[i];

		/* Remember prev anim state. */
		struct anim_state prev_state = draw_sprite->state;
		struct anim *prev_anim = draw_sprite->anim;

		/* Copy from source sprite into draw sprite. */
		memcpy(draw_sprite, src_sprite, sizeof(struct sprite));

		if(draw_sprite->anim == prev_anim) {
			draw_sprite->state = prev_state;
		}

		/* Scale to group scale. */
		mult2f(draw_sprite->scale, xy_of(group->scale));

		/* Scale offset position. */
		/* FIXME: is this correct? Also: scale should be vec3 so we can use
		 * mult3f below... */
		mult3f(draw_sprite->position, xyz(draw_sprite->scale));
		add3f(draw_sprite->position, xyz(group->position));
	}
}

struct sprite* sprite_group_add_sprite(struct sprite_group* group, struct anim *anim,
		float relx, float rely, float relz, float sx, float sy)
{
	/* Sanity check. */
	if(group->sprites_count >= SPRITE_GROUP_MAX_SPRITES) {
		animatedsprites_error("Too many sprite group members\n");
		return NULL;
	}

	struct sprite *sprite = &group->sprites[group->sprites_count++];
	set3f(sprite->position, relx, rely, relz);
	set2f(sprite->scale, sx, sy);
	animatedsprites_switchanim(sprite, anim);

	return sprite;
}

void animatedsprites_add_group(struct animatedsprites* as, struct sprite_group* group)
{
	if(as == NULL || group == NULL) {
		return;
	}

	/* Update draw sprites. */
	sprite_group_update(group);

	for(int i=0; i<group->sprites_count; i++) {
		animatedsprites_add(as, &group->draw_sprites[i]);
	}
}
