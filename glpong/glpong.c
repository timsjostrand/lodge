/**
 * OpenGL Pong!
 *
 * Authors: Tim Sjöstrand <tim.sjostrand@gmail.com>
 *			Johan Yngman <johan.yngman@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "game.h"
#include "math4.h"
#include "color.h"
#include "graphics.h"
#include "shader.h"
#include "input.h"
#include "texture.h"
#include "sound.h"
#include "vfs.h"
#include "atlas.h"
#include "monotext.h"
#include "console.h"
#include "str.h"

#include "core.h"
#include "core_argv.h"
#include "core_reload.h"

#include "assets.h"

#define VIEW_WIDTH		640
#define VIEW_HEIGHT		360	/* 16:9 aspect ratio */

#define BOARD_TOP		VIEW_HEIGHT
#define BOARD_BOTTOM	0
#define BOARD_LEFT		0
#define BOARD_RIGHT		VIEW_WIDTH

#define BALL_WIDTH		16
#define BALL_HEIGHT		16
#define BALL_SPEED_MAX	0.5f

#define PLAYER_WIDTH	16
#define PLAYER_HEIGHT	64
#define PLAYER1_HIT		32.0f
#define PLAYER2_HIT		608.0f

#define PARTICLES_MAX	100
#define PARTICLE_ALPHA	0.5f

#define SPRITE_TYPE_UNKNOWN		0
#define SPRITE_TYPE_BALL		1
#define SPRITE_TYPE_PLAYER		2
#define SPRITE_TYPE_PARTICLE	3

struct game_settings settings = {
	.view_width = VIEW_WIDTH,
	.view_height = VIEW_HEIGHT,
	.window_width = VIEW_WIDTH,
	.window_height = VIEW_HEIGHT,
	.window_title = "glpong",
	.sound_listener = { VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f, 0.0f },
	.sound_distance_max = 500.0f, // distance3f(vec3(0), sound_listener)
};

struct stats {
	int points;
	int hits;
	int streak;
	int current_streak;
};

struct player {
	struct basic_sprite	sprite;
	struct stats		stats;
	float				charge;
	float				last_emit;
};

struct ball {
	struct basic_sprite	sprite;
	float				vx;
	float				vy;
	float				speed;
	float				last_hit_x;
	float				last_hit_y;
};

struct basic_particle {
	struct basic_sprite	sprite;
	int					dead;
	float				age;
	float				age_max;
	float				vx;
	float				vy;
	float				va;
};

struct game {
	float					time;						/* Time since start. */
	float					graphics_detail;
	struct player			player1;					/* Left player. */
	struct player			player2;					/* Right player. */
	struct basic_sprite		effectslayer;
	struct ball				ball;
	struct stats			total_stats;
	struct basic_particle	particles[PARTICLES_MAX];
	int						particles_count;
	struct sound_emitter	*vivaldi_src;
	sound_buf_t				tone_hit;
	sound_buf_t				tone_bounce;
	struct atlas			atlas_earl;
	struct monofont			font;
	struct monotext			txt_debug;
	const char				*conf;
	size_t					conf_size;
	vec3					mouse_pos;
};

struct game* game = 0;
struct vfs* vfs = NULL;
struct input* input_global = NULL;

struct game_settings* game_get_settings()
{
	return &settings;
}

void print_stats()
{
	printf("Player 1: Points: %-2d, Hits: %-2d, Longest Streak: %-2d\n",
		game->player1.stats.points,
		game->player1.stats.hits,
		game->player1.stats.streak);
	printf("Player 2: Points: %-2d, Hits: %-2d, Longest Streak: %-2d\n",
		game->player2.stats.points,
		game->player2.stats.hits,
		game->player2.stats.streak);
	printf("Longest total streak: %-2d\n", game->total_stats.streak);
}

void basic_particle_init(struct basic_particle *p, float x, float y, float w, float h,
	float angle, float vx, float vy, float va, float age_max)
{
	p->dead = 0;
	p->age = 0.0f;
	p->age_max = age_max;
	p->vx = vx;
	p->vy = vy;
	p->va = va;
	p->sprite.type = SPRITE_TYPE_PARTICLE;
	p->sprite.rotation = angle;
	p->sprite.texture = &core_global->textures.none;

	set4f(p->sprite.pos, x, y, 0.0f, 1.0f);
	set4f(p->sprite.color, rgb(COLOR_WHITE), PARTICLE_ALPHA);
	set4f(p->sprite.scale, w, h, 1.0f, 1.0f);
}

void basic_particle_new(float x, float y, float w, float h, float angle, float vx,
	float vy, float va, float age_max)
{
	if (game->particles_count >= PARTICLES_MAX) {
		printf("max particle count reached\n");
		return;
	}
	basic_particle_init(&game->particles[game->particles_count], x, y, w, h, angle,
		vx, vy, va, age_max);
	game->particles_count++;
}

int player_is_charged(struct player *p)
{
	/* p->charge is idle time in ms. */
	return p->charge >= 3200.0f;
}

void think_player_charged(struct player *p, float dt)
{
	if (player_is_charged(p) && p->last_emit >= 16.0f) {
		float size = randr(8.0f, 16.0f);
		basic_particle_new(p->sprite.pos[0],											// x
			p->sprite.pos[1] + randr(-PLAYER_HEIGHT / 2, PLAYER_HEIGHT / 2),	// y
			size, size,														// w, h
			randr(0, 2 * M_PI),												// angle
			randr(-0.05f, 0.05f),											// vx
			randr(-0.05f, 0.05f),											// vy
			randr(-(2 * M_PI) * 0.0001f, (2 * M_PI) * 0.0001f),				// va
			randr(200.0f, 600.0f)											// time_max
			);
		p->last_emit = 0;
	}
	p->last_emit += dt;
}

void ball_player_bounce(struct ball *ball, struct player *p)
{
	/* 0 on player1, 1 on player2. */
	int player_no = (p == &game->player2);

	p->stats.hits++;
	p->stats.current_streak++;
	p->stats.streak = imax(p->stats.streak, p->stats.current_streak);

	game->total_stats.current_streak++;
	game->total_stats.streak = imax(game->total_stats.streak, game->total_stats.current_streak);

	float diff = (ball->sprite.pos[1] - p->sprite.pos[1]) / (PLAYER_HEIGHT / 2.0f);
	diff = clamp(diff, -0.8f, 0.8f);
	/* angle table:
	* player1: angle = M_PI/2.0f - acos(diff)
	* player2: angle = M_PI/2.0f + acos(diff) */
	float angle = M_PI / 2.0f + (player_no == 0 ? -1 : +1) * acos(diff);
	float force = 1.0f + (1.5f * player_is_charged(p));
	float current_speed = sqrt(ball->vx*ball->vx + ball->vy*ball->vy);
	current_speed = fmax(BALL_SPEED_MAX, current_speed);
	ball->vx = cos(angle) * current_speed * force;
	ball->vy = sin(angle) * current_speed * force;
	ball->last_hit_x = 0.0f;

	p->charge = 0.0f;

	sound_buf_play_pitched(&core_global->sound, game->tone_hit, ball->sprite.pos, 0.05f);
}

void ball_think(float dt)
{
	// Ball: left/right hit detection
	int ball_reset = 0;
	if (game->ball.sprite.pos[0] < BOARD_LEFT + 8) {
		ball_reset = 1;
		game->total_stats.current_streak = 0;
		game->player1.stats.current_streak = 0;
		game->player2.stats.points++;
		printf("player 1 looses \n");
		print_stats();
	}
	else if (game->ball.sprite.pos[0] > BOARD_RIGHT - 8) {
		ball_reset = 1;
		game->total_stats.current_streak = 0;
		game->player2.stats.current_streak = 0;
		game->player1.stats.points++;
		printf("player 2 looses \n");
		print_stats();
	}

	if (ball_reset) {
		/* Shameball! */
		game->ball.vx = clamp(game->ball.vx / 4.0f,
			-game->ball.speed / 8.0f,
			game->ball.speed / 8.0f);
		game->ball.vy = clamp(game->ball.vy / 4.0f,
			-game->ball.speed / 8.0f,
			game->ball.speed / 8.0f);
		/* Restart at center of view. */
		game->ball.sprite.pos[0] = VIEW_WIDTH / 2;
		game->ball.sprite.pos[1] = VIEW_HEIGHT / 2;
	}

	// Ball: top/bottom hit detection
	if (game->ball.sprite.pos[1] > BOARD_TOP - BALL_HEIGHT / 2) {
		game->ball.sprite.pos[1] = BOARD_TOP - BALL_HEIGHT / 2;
		game->ball.vy *= -1.0f;
		game->ball.last_hit_y = 0.0f;
		sound_buf_play_pitched(&core_global->sound, game->tone_bounce, game->ball.sprite.pos, 0.05f);
	}
	else if (game->ball.sprite.pos[1] < BOARD_BOTTOM + BALL_HEIGHT / 2) {
		game->ball.sprite.pos[1] = BOARD_BOTTOM + BALL_HEIGHT / 2;
		game->ball.vy *= -1.0f;
		game->ball.last_hit_y = 0.0f;
		sound_buf_play_pitched(&core_global->sound, game->tone_bounce, game->ball.sprite.pos, 0.05f);
	}

	// Ball: move
	game->ball.sprite.pos[0] += dt*game->ball.vx;
	game->ball.sprite.pos[1] += dt*game->ball.vy;
	game->ball.last_hit_x += dt;
	game->ball.last_hit_y += dt;
}

void player1_think(struct player *p, float dt)
{
	p->charge += dt;

	// Move?
	if (key_down(GLFW_KEY_W)) {
		p->sprite.pos[1] += dt*0.6f;
		p->charge = 0;
	}
	else if (key_down(GLFW_KEY_S)) {
		p->sprite.pos[1] -= dt*0.6f;
		p->charge = 0;
	}

	// Outside board?
	p->sprite.pos[1] = clamp(p->sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
}

void player2_think(struct player *p, float dt)
{
	p->charge += dt;

	// Move?
	if (key_down(GLFW_KEY_UP)) {
		p->sprite.pos[1] += dt*0.6f;
		p->charge = 0;
	}
	else if (key_down(GLFW_KEY_DOWN)) {
		p->sprite.pos[1] -= dt*0.6f;
		p->charge = 0;
	}

	// Outside board?
	p->sprite.pos[1] = clamp(p->sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
}

void glpong_think(float dt)
{
	ball_think(dt);
	player1_think(&game->player1, dt);
	player2_think(&game->player2, dt);

	// Ball collides with Player 1?
	if (game->ball.vx < 0
		&& game->ball.sprite.pos[1] >= (game->player1.sprite.pos[1] - PLAYER_HEIGHT / 2 - BALL_HEIGHT / 2)
		&& game->ball.sprite.pos[1] <= (game->player1.sprite.pos[1] + PLAYER_HEIGHT / 2 + BALL_HEIGHT / 2)
		&& game->ball.sprite.pos[0] <= (PLAYER1_HIT + PLAYER_WIDTH / 2 + BALL_WIDTH / 2)) {
		ball_player_bounce(&game->ball, &game->player1);
	}

	// Ball collides with Player 2?
	if (game->ball.vx > 0
		&& game->ball.sprite.pos[1] >= (game->player2.sprite.pos[1] - PLAYER_HEIGHT / 2 - BALL_HEIGHT / 2)
		&& game->ball.sprite.pos[1] <= (game->player2.sprite.pos[1] + PLAYER_HEIGHT / 2 + BALL_HEIGHT / 2)
		&& game->ball.sprite.pos[0] >= (PLAYER2_HIT - PLAYER_WIDTH / 2 - BALL_WIDTH / 2)) {
		ball_player_bounce(&game->ball, &game->player2);
	}

	// Emit player charge particles
	think_player_charged(&game->player1, dt);
	think_player_charged(&game->player2, dt);
}

void basic_particles_think(float dt)
{
	/* Think for each particle. */
	for (int i = 0; i<game->particles_count; i++) {
		struct basic_particle *p = &game->particles[i];

		p->age += dt;
		p->sprite.pos[0] += p->vx * dt;
		p->sprite.pos[1] += p->vy * dt;
		p->sprite.color[3] = PARTICLE_ALPHA - clamp(p->age / p->age_max, 0.0f, 1.0f) * PARTICLE_ALPHA;
		p->sprite.rotation += p->va * dt;

		if (p->age >= p->age_max) {
			p->dead = 1;
		}
	}

	/* Remove dead particles. */
	for (int i = 0; i<game->particles_count; i++) {
		if (game->particles[i].dead) {
			for (int n = i + 1; n<game->particles_count; n++) {
				game->particles[i] = game->particles[n];
			}
			game->particles_count--;
		}
	}
}

void init_effectslayer(struct basic_sprite* b)
{
	b->type = SPRITE_TYPE_UNKNOWN;
	b->texture = &core_global->textures.none;
	set4f(b->pos, VIEW_WIDTH / 2, VIEW_HEIGHT / 2, 0.5f, 1.0f);
	set4f(b->scale, VIEW_WIDTH, VIEW_HEIGHT, 1.0f, 1.0f);
	copyv(b->color, COLOR_BLACK);
}

void init_player1(struct player *p)
{
	p->sprite.type = SPRITE_TYPE_PLAYER;
	p->sprite.texture = &assets->textures.paddle;
	set4f(p->sprite.pos, 32.0f, VIEW_HEIGHT / 2, 0.1f, 1.0f);
	set4f(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
	copyv(p->sprite.color, COLOR_WHITE);
}

void init_player2(struct player *p)
{
	p->sprite.type = SPRITE_TYPE_PLAYER;
	p->sprite.texture = &assets->textures.paddle;
	set4f(p->sprite.pos, 608.0f, VIEW_HEIGHT / 2, 0.1f, 1.0f);
	set4f(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
	copyv(p->sprite.color, COLOR_WHITE);
}

void init_ball(struct ball *ball)
{
	ball->sprite.type = SPRITE_TYPE_BALL;
	ball->sprite.texture = &assets->textures.testball;
	ball->speed = 0.6f;
	rand(); rand(); rand();
	float random_angle = randr(0.0f, 2.0f * M_PI);
	ball->vy = ball->speed * sin(random_angle) / 4.0f;
	ball->vx = ball->speed * cos(random_angle) / 4.0f;
	ball->last_hit_x = 10.0f;
	ball->last_hit_y = 0.0f;

	set4f(ball->sprite.pos, VIEW_WIDTH / 2, VIEW_HEIGHT / 2, 0.06f, 1.0f);
	set4f(ball->sprite.scale, BALL_WIDTH, BALL_HEIGHT, 1.0f, 1.0f);
	copyv(ball->sprite.color, COLOR_WHITE);
}

void load_console_conf()
{
	vfs_register_callback("glpong.rc", &core_reload_console_conf, &core_global->console);
}

void game_mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		double x = 0;
		double y = 0;
		glfwGetCursorPos(window, &x, &y);
		int win_w = 0;
		int win_h = 0;
		glfwGetWindowSize(window, &win_w, &win_h);
		/* Convert screen space => game space. */
		game->mouse_pos[0] = x * (VIEW_WIDTH / (float)win_w);
		game->mouse_pos[1] = VIEW_HEIGHT - y * (VIEW_HEIGHT / (float)win_h);

		sound_buf_play_pitched(&core_global->sound, game->tone_hit, game->mouse_pos, 0.2f);
		console_debug("Click at %.0fx%.0f (distance to listener: %.0f)\n",
			game->mouse_pos[0], game->mouse_pos[1],
			distance3f(sound_listener, game->mouse_pos));
	}
}

void game_console_init(struct console* c)
{
	/* Set up game-specific console variables. */
	console_env_bind_1f(c, "graphics_detail", &(game->graphics_detail));
}

void game_key_callback(struct core* core, struct input* input, GLFWwindow* window, int key,
	int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_O:
			core_global->graphics.delta_time_factor *= 2.0f;
			sound_src_pitch(game->vivaldi_src->src, core_global->graphics.delta_time_factor);
			printf("time_mod=%f\n", core_global->graphics.delta_time_factor);
			break;
		case GLFW_KEY_P:
			core_global->graphics.delta_time_factor /= 2.0f;
			sound_src_pitch(game->vivaldi_src->src, core_global->graphics.delta_time_factor);
			printf("time_mod=%f\n", core_global->graphics.delta_time_factor);
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			break;
		}
	}
}

void load_sounds()
{
	if (sound_buf_load_filter(&game->tone_hit,
		0.1 * SOUND_SAMPLE_RATE,
		SOUND_SAMPLE_RATE,
		&sound_filter_add_440hz) != SOUND_OK) {
		sound_error("could not generate tone\n");
	}

	if (sound_buf_load_filter(&game->tone_bounce,
		0.1 * SOUND_SAMPLE_RATE,
		SOUND_SAMPLE_RATE,
		&sound_filter_add_220hz) != SOUND_OK) {
		sound_error("could not generate tone\n");
	}
}

void load_shaders()
{
	/* Sprite shader: set up uniforms */
	shader_uniform1f(&assets->shaders.basic_shader, "time", &game->time);
	shader_uniform1f(&assets->shaders.basic_shader, "ball_last_hit_x", &game->ball.last_hit_x);
	shader_uniform1f(&assets->shaders.basic_shader, "ball_last_hit_y", &game->ball.last_hit_y);

	/* Effects shader: set up uniforms */
	shader_uniform1f(&assets->shaders.ball_trail, "time", &game->time);
	shader_uniform1f(&assets->shaders.ball_trail, "ball_last_hit_x", &game->ball.last_hit_x);
	shader_uniform1f(&assets->shaders.ball_trail, "ball_last_hit_y", &game->ball.last_hit_y);
	shader_uniform4f(&assets->shaders.ball_trail, "ball_pos", &game->ball.sprite.pos);
	shader_uniform1f(&assets->shaders.ball_trail, "view_width", &core_global->view_width);
	shader_uniform1f(&assets->shaders.ball_trail, "view_height", &core_global->view_height);
}

void load_atlases()
{
	/* Register asset callbacks */
	vfs_register_callback("earl.json", core_reload_atlas, &game->atlas_earl);
}

void release_sounds()
{
	sound_buf_free(game->tone_bounce);
	sound_buf_free(game->tone_hit);
}

void release_atlases()
{
	atlas_free(&game->atlas_earl);
}

static void load_fonts()
{
	monofont_new(&game->font, "manaspace.png", 16, 16, -7, 0);
}

static void release_fonts()
{
	monofont_free(&game->font);
}

void game_fps_callback(struct frames *f)
{
	monotext_updatef(&game->txt_debug, "FPS:% 5d, MS:% 3.1f/% 3.1f/% 3.1f",
		f->frames, f->frame_time_min, f->frame_time_avg, f->frame_time_max);
}

void game_think(struct core* core, struct graphics* g, float delta_time)
{
	game->time = (float)glfwGetTime();
	glpong_think(delta_time);
	basic_particles_think(delta_time);
	shader_uniforms_think(&assets->shaders.basic_shader, delta_time);
	shader_uniforms_think(&assets->shaders.ball_trail, delta_time);
}

void game_render(struct core* core, struct graphics* g, float delta_time)
{
	/* Clear. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Ball. */
	sprite_render(&game->ball.sprite, &assets->shaders.basic_shader, g);

	/* Particles. */
	for (int i = 0; i<game->particles_count; i++) {
		if (!game->particles[i].dead) {
			sprite_render(&game->particles[i].sprite, &assets->shaders.basic_shader, g);
		}
	}

	/* Sprites. */
	sprite_render(&game->player1.sprite, &assets->shaders.basic_shader, g);
	sprite_render(&game->player2.sprite, &assets->shaders.basic_shader, g);

	/* Effectslayer */
	if (game->graphics_detail <= 0) {
		sprite_render(&game->effectslayer, &assets->shaders.ball_trail, &core_global->graphics);
	}

	/* Text. */
	monotext_render(&game->txt_debug, &assets->shaders.basic_shader, g);
}

void game_init()
{
	/* Entities. */
	init_effectslayer(&game->effectslayer);
	init_player1(&game->player1);
	init_player2(&game->player2);
	init_ball(&game->ball);
	monotext_new(&game->txt_debug, "FPS: 0", COLOR_WHITE, &game->font, 16.0f,
		VIEW_HEIGHT - 16.0f);
	game->vivaldi_src = sound_buf_play_music(&core_global->sound, assets->sounds.vivaldi, 1.0f);
}

void game_init_memory(struct shared_memory* shared_memory, int reload)
{
	if (!reload)
	{
		struct game tmp = { 0 };
		memcpy(shared_memory->game_memory, &tmp, sizeof(tmp));
	}

	game = (struct game*)shared_memory->game_memory;
	core_global = (struct core*)shared_memory->core;
	assets = (struct assets*)shared_memory->assets;
	vfs_global = shared_memory->vfs;
	input_global = shared_memory->input;
}

void game_assets_load()
{
	assets_load();

	load_shaders();
	load_sounds();
	load_atlases();
	load_fonts();
	load_console_conf();
}

void game_assets_release()
{
	release_sounds();
	release_atlases();
	release_fonts();

	assets_release();
}
