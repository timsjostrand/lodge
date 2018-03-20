/**
 * Particle emitter.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <string.h>

#include "particles.h"
#include "alist.h"
#include "animatedsprites.h"

void particles_init(struct particles *em, int particles_max)
{
	if(particles_max > PARTICLES_MAX) {
		particles_error("Number of particles (%d) > PARTICLES_MAX (%d)\n",
				particles_max, PARTICLES_MAX);
		return;
	}

	em->particles_count = 0;
	em->particles_max = particles_max;
	em->sprites = animatedsprites_create();
	em->particles = alist_new(particles_max);

	for(int i=0; i<particles_max; i++) {
		em->mem[i].dead = 1;
	}
}

void particles_free(struct particles *em)
{
	animatedsprites_destroy(em->sprites);
}

void particles_think(struct particles *em, struct atlas *atlas, float dt)
{
	/* FIXME: This could be changed to only removing particles when they die, and
	 * never having to call animatedsprites_clear(). This is not supported in the
	 * animatedsprites api yet though. */
	animatedsprites_clear(em->sprites);

	/* Update particles. */
	foreach_alist(struct particle *, p, index, em->particles) {
		p->age += dt;

		/* Is particle dead? Remove it and continue iteration. */
		if(p->age >= p->age_max) {
			p->dead = 1;
			alist_delete_at(em->particles, index, 0);
			index --;
			continue;
		}

		/* Update particle. */
		p->think(p, dt);

		/* Add to sprite batcher. */
		animatedsprites_add(em->sprites, &p->sprite);
	}

	animatedsprites_update(em->sprites, atlas, dt);
}

void particles_render(struct particles *em, struct shader *s, GLuint texture, mat4 projection, mat4 transform)
{
	animatedsprites_render_simple(em->sprites, s, texture, projection, transform);
}

static void particle_init(struct particle *p, struct anim *anim,
		particle_think_t particle_think,
		float x, float y, float w, float h,
		float angle, float vx, float vy, float age_max)
{
	memset(p, 0, sizeof(struct particle));
	p->think = particle_think;
	p->dead = 0;
	p->age = 0;
	p->age_max = age_max;
	vec3_init(&p->v, vx, vy, 0);

	/* Init sprite */
	vec3_init(&p->sprite.position, x, y, 0);
	vec2_init(&p->sprite.scale, w, h);

	animatedsprites_playanimation(&p->sprite, anim);
}

/**
 * Get the next available particle memory.
 */
static struct particle* particles_particle_next(struct particles *em)
{
	for(int i=0; i<em->particles_max; i++) {
		if(em->mem[i].dead) {
			return &em->mem[i];
		}
	}
	return NULL;
}

/**
 * Spawn a new particle.
 */
void particles_particle_spawn(struct particles *em,
		struct anim *anim,
		particle_think_t particle_think,
		float x, float y, float w, float h,
		float angle, float vx, float vy, float age_max)
{
	/* Max particles reached? */
	if(em->particles_count >= em->particles_max) {
		em->particles_max_counter++;
		return;
	}

	/* Get the next available particle. */
	struct particle *p = particles_particle_next(em);
	if(p == NULL) {
		em->particles_max_counter++;
		return;
	}
	particle_init(p, anim, particle_think, x, y, w, h, angle, vx, vy, age_max);

	/* Add it to the list of alive particles. */
	alist_append(em->particles, p);
}

/**
 * Random range of integers.
 *
 * @param min	The minimum value returned (inclusive).
 * @param max	The maximum value returned (inclusive).
 */
int randri(int min, int max)
{
	return min + (rand() % (max-min));
}

/**
 * Emit new particles.
 *
 * NOTE: angle_min,angle_max is not implemented yet.
 */
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
		float age_max_min, float age_max_max
)
{
	for(int i = 0, count = randri(count_min, count_max); i < count; i++) {
		particles_particle_spawn(em, anim, particle_think,
				randr(x_min, x_max),
				randr(y_min, y_max),
				randr(w_min, w_max),
				randr(h_min, h_max),
				0.0f, // angle
				randr(vx_min, vx_max),
				randr(vy_min, vy_max),
				randr(age_max_min, age_max_max)
		);
	}
}
