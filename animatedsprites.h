#ifndef _ANIMATEDSPRITES_H
#define _ANIMATEDSPRITES_H

#include "math4.h"
#include "graphics.h"
#include "spritebatch.h"
#include "atlas.h"
#include "log.h"

#define ANIMATEDSPRITES_MAX_SPRITES	10000

#define animatedsprites_debug(...) debugf("Animbatch", __VA_ARGS__)
#define animatedsprites_error(...) errorf("Animbatch", __VA_ARGS__)

typedef float GLfloat;
typedef unsigned int GLuint;

struct anim
{
	int looping;
	int frame_start;
	int frame_count;
	float frame_length;
};

struct anim_state
{
	int done;
	int frame_current;
	float frame_time;
};

struct sprite
{
	vec3 position;
	vec2 scale;

	const struct anim *anim;
	struct anim_state state;
};

struct animatedsprites
{
	struct sprite* sprites_todraw[ANIMATEDSPRITES_MAX_SPRITES];
	struct spritebatch spritebatch;
	unsigned int sprite_todraw_count;
};

struct animatedsprites* animatedsprites_create();
void animatedsprites_init(struct animatedsprites* animatedsprites);
void animatedsprites_destroy(struct animatedsprites* animatedsprites);

void animatedsprites_playanimation(struct sprite* sprite, struct anim* anim);
void animatedsprites_switchanim(struct sprite* sprite, struct anim* anim);
void animatedsprites_update(struct animatedsprites* animatedsprites, struct atlas* atlas, float delta_time);
void animatedsprites_render(struct animatedsprites* animatedsprites, struct shader *s, struct graphics *g, GLuint tex, mat4 transform);

void animatedsprites_add(struct animatedsprites* animatedsprites, struct sprite* sprite);
void animatedsprites_clear(struct animatedsprites* animatedsprites);
void animatedsprites_sort(struct animatedsprites* animatedsprites, spritebatch_sort_fn sorting_function);

void animatedsprites_setanim(struct anim* anim, int looping, int frame_start, int frame_count, float frame_length);

#endif //_ANIMATEDSPRITES_H
