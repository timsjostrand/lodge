#ifndef _PARTICLES_H
#define _PARTICLES_H

#include "log.h"
#include "math4.h"
#include "alist.h"
#include "animatedsprites.h"

#define particles_error(...) errorf("Particles", __VA_ARGS__)

#define PARTICLES_MAX	1024

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct particle;

typedef void (*particle_think_t)(struct particle *p, float dt);

struct particle {
	int					dead;
	struct sprite		sprite;
	float				age;		/* How many game units this particle has existed. */
	float				age_max;	/* When this particle dies. */
	vec3				v;			/* Velocity. */
	particle_think_t	think;
};

struct particles {
	int						particles_count;
	int						particles_max;			/* How many particles to spawn. */
	int						particles_max_counter;	/* DEBUG: How many times the particles_max limit was reached. */
	int						emit_interval_min;		/* NOTE: not implemented! */
	int						emit_interval_max;		/* NOTE: not implemented! */
	struct animatedsprites	*sprites;				/* Sprite batcher. */
	struct alist			*particles;				/* A list of 'struct particle's. */
	struct particle			mem[PARTICLES_MAX];
};

void particles_init(struct particles *em, int particles_max);
void particles_free(struct particles *em);
void particles_think(struct particles *em, struct atlas *atlas, float dt);
void particles_render(struct particles *em, lodge_shader_t s, lodge_texture_t texture, mat4 projection, mat4 transform);

void particles_emit(struct particles *em,
		struct anim *anim,
		particle_think_t particle_think,
		int count_min, int count_max,
		float x_min, float x_max,
		float y_min, float y_max,
		float w_min, float w_max,
		float h_min, float h_max,
		float angle_min, float angle_max,
		float vx_min, float vx_max,
		float vy_min, float vy_max,
		float age_max_min, float age_max_max);

#endif
