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

#define VIEW_WIDTH		640
#define VIEW_HEIGHT		360		/* 16:9 aspect ratio */

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

struct stats {
	int points;
	int hits;
	int streak;
	int current_streak;
};

struct player {
	struct sprite	sprite;
	struct stats	stats;
	float			charge;
	float			last_emit;
};

struct ball {
	struct sprite	sprite;
	float			vx;
	float			vy;
	float			speed;
	float			last_hit_x;
	float			last_hit_y;
};

struct particle {
	struct sprite	sprite;
	int				dead;
	float			age;
	float			age_max;
	float			vx;
	float			vy;
	float			va;
};

struct textures {
	GLuint	none;
	GLuint	test;
	GLuint	paddle;
};

struct game {
	float			time;						/* Time since start. */
	float			view_width;
	float			view_height;
	struct graphics	graphics;					/* Graphics state. */
	struct shader	shader;						/* Shader program information. */
	struct shader	bg_shader;
	struct player	player1;					/* Left player. */
	struct player	player2;					/* Right player. */
	struct sprite	effectslayer;
	struct ball		ball;
	struct stats	total_stats;
	struct particle	particles[PARTICLES_MAX];
	int				particles_count;
	struct input	input;
	struct textures	textures;
	struct sound	sound;
	struct sound_fx	vivaldi;
	struct sound_fx	tone_hit;
	struct sound_fx	tone_bounce;
	struct atlas	atlas_earl;
	struct monofont	font;
	struct monofont	font_console;
	struct monotext	txt_debug;
	struct console	console;
	struct console_cmd cmd_quit;
	struct console_cmd cmd_assets;
	struct console_cmd cmd_assets_list;
	struct console_cmd cmd_assets_reload;
} game = { 0 };

void print_stats()
{
	printf("Player 1: Points: %-2d, Hits: %-2d, Longest Streak: %-2d\n",
			game.player1.stats.points,
			game.player1.stats.hits,
			game.player1.stats.streak);
	printf("Player 2: Points: %-2d, Hits: %-2d, Longest Streak: %-2d\n",
			game.player2.stats.points,
			game.player2.stats.hits,
			game.player2.stats.streak);
	printf("Longest total streak: %-2d\n", game.total_stats.streak);
}

void particle_init(struct particle *p, float x, float y, float w, float h,
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
	p->sprite.texture = &game.textures.none;

	set4f(p->sprite.pos, x, y, 0.0f, 1.0f);
	set4f(p->sprite.color, rgb(COLOR_WHITE), PARTICLE_ALPHA);
	set4f(p->sprite.scale, w, h, 1.0f, 1.0f);
}

void particle_new(float x, float y, float w, float h, float angle, float vx,
		float vy, float va, float age_max)
{
	if(game.particles_count >= PARTICLES_MAX) {
		printf("max particle count reached\n");
		return;
	}
	particle_init(&game.particles[game.particles_count], x, y, w, h, angle,
			vx, vy, va, age_max);
	game.particles_count ++;
}

int player_is_charged(struct player *p)
{
	/* p->charge is idle time in ms. */
	return p->charge >= 3200.0f;
}

void think_player_charged(struct player *p, float dt)
{
	if(player_is_charged(p) && p->last_emit >= 16.0f) {
		float size = randr(8.0f, 16.0f);
		particle_new(p->sprite.pos[0],											// x
				p->sprite.pos[1] + randr(-PLAYER_HEIGHT/2, PLAYER_HEIGHT/2),	// y
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
	int player_no = (p == &game.player2);
	printf("collides with player %d\n", player_no + 1);

	p->stats.hits ++;
	p->stats.current_streak ++;
	p->stats.streak = imax(p->stats.streak, p->stats.current_streak);

	game.total_stats.current_streak ++;
	game.total_stats.streak = imax(game.total_stats.streak, game.total_stats.current_streak);

	float diff = (ball->sprite.pos[1] - p->sprite.pos[1]) / (PLAYER_HEIGHT/2.0f);
	diff = clamp(diff, -0.8f, 0.8f);
	/* angle table:
	 * player1: angle = M_PI/2.0f - acos(diff)
	 * player2: angle = M_PI/2.0f + acos(diff) */
	float angle = M_PI/2.0f + (player_no == 0 ? -1 : +1) * acos(diff);
	float force = 1.0f + (1.5f * player_is_charged(p));
	float current_speed = sqrt(ball->vx*ball->vx + ball->vy*ball->vy);
	printf("force=%6f, current_speed=%6f\n", force, current_speed);
	current_speed = fmax(BALL_SPEED_MAX, current_speed);
	ball->vx = cos(angle) * current_speed * force;
	ball->vy = sin(angle) * current_speed * force;
	ball->last_hit_x = 0.0f;

	p->charge = 0.0f;

	sound_fx_play(&game.tone_hit);
}

void ball_think(float dt)
{
	// Ball: left/right hit detection
	int ball_reset = 0;
	if(game.ball.sprite.pos[0] < BOARD_LEFT + 8) {
		ball_reset = 1;
		game.total_stats.current_streak = 0;
		game.player1.stats.current_streak = 0;
		game.player2.stats.points ++;
		printf("player 1 looses \n");
		print_stats();
	} else if(game.ball.sprite.pos[0] > BOARD_RIGHT - 8) {
		ball_reset = 1;
		game.total_stats.current_streak = 0;
		game.player2.stats.current_streak = 0;
		game.player1.stats.points ++;
		printf("player 2 looses \n");
		print_stats();
	}

	if(ball_reset) {
		/* Shameball! */
		game.ball.vx = clamp(game.ball.vx / 4.0f,
				-game.ball.speed / 8.0f,
				game.ball.speed / 8.0f);
		game.ball.vy = clamp(game.ball.vy / 4.0f,
				-game.ball.speed / 8.0f,
				game.ball.speed / 8.0f);
		/* Restart at center of view. */
		game.ball.sprite.pos[0] = VIEW_WIDTH/2;
		game.ball.sprite.pos[1] = VIEW_HEIGHT/2;
	}
		
	// Ball: top/bottom hit detection
	if(game.ball.sprite.pos[1] > BOARD_TOP - BALL_HEIGHT/2) {
		game.ball.sprite.pos[1] = BOARD_TOP - BALL_HEIGHT/2;
		game.ball.vy *= -1.0f;
		game.ball.last_hit_y = 0.0f;
		sound_fx_play(&game.tone_bounce);
	} else if(game.ball.sprite.pos[1] < BOARD_BOTTOM + BALL_HEIGHT/2) {
		game.ball.sprite.pos[1] = BOARD_BOTTOM + BALL_HEIGHT/2;
		game.ball.vy *= -1.0f;
		game.ball.last_hit_y = 0.0f;
		sound_fx_play(&game.tone_bounce);
	}

	// Ball: move
	game.ball.sprite.pos[0] += dt*game.ball.vx;
	game.ball.sprite.pos[1] += dt*game.ball.vy;
	game.ball.last_hit_x += dt;
	game.ball.last_hit_y += dt;
}

void player1_think(struct player *p, float dt)
{
	p->charge += dt;

	// Move?
	if(key_down(GLFW_KEY_W)) {
		p->sprite.pos[1] += dt*0.6f;
		p->charge = 0;
	} else if(key_down(GLFW_KEY_S)) {
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
	if(key_down(GLFW_KEY_UP)) {
		p->sprite.pos[1] += dt*0.6f;
		p->charge = 0;
	} else if(key_down(GLFW_KEY_DOWN)) {
		p->sprite.pos[1] -= dt*0.6f;
		p->charge = 0;
	}

	// Outside board?
	p->sprite.pos[1] = clamp(p->sprite.pos[1], BOARD_BOTTOM, BOARD_TOP);
}

void game_think(float dt)
{
	ball_think(dt);
	player1_think(&game.player1, dt);
	player2_think(&game.player2, dt);

	// Ball collides with Player 1?
	if(game.ball.vx < 0
			&& game.ball.sprite.pos[1] >= (game.player1.sprite.pos[1] - PLAYER_HEIGHT/2 - BALL_HEIGHT/2)
			&& game.ball.sprite.pos[1] <= (game.player1.sprite.pos[1] + PLAYER_HEIGHT/2 + BALL_HEIGHT/2)
			&& game.ball.sprite.pos[0] <= (PLAYER1_HIT + PLAYER_WIDTH/2 + BALL_WIDTH/2)) {
		ball_player_bounce(&game.ball, &game.player1);
	}

	// Ball collides with Player 2?
	if(game.ball.vx > 0
			&& game.ball.sprite.pos[1] >= (game.player2.sprite.pos[1] - PLAYER_HEIGHT/2 - BALL_HEIGHT/2)
			&& game.ball.sprite.pos[1] <= (game.player2.sprite.pos[1] + PLAYER_HEIGHT/2 + BALL_HEIGHT/2)
			&& game.ball.sprite.pos[0] >= (PLAYER2_HIT - PLAYER_WIDTH/2 - BALL_WIDTH/2)) {
		ball_player_bounce(&game.ball, &game.player2);
	}

	// Emit player charge particles
	think_player_charged(&game.player1, dt);
	think_player_charged(&game.player2, dt);
}

void particles_think(float dt)
{
	/* Think for each particle. */
	for(int i=0; i<game.particles_count; i++) {
		struct particle *p = &game.particles[i];

		p->age += dt;
		p->sprite.pos[0] += p->vx * dt;
		p->sprite.pos[1] += p->vy * dt;
		p->sprite.color[3] = PARTICLE_ALPHA - clamp(p->age / p->age_max, 0.0f, 1.0f) * PARTICLE_ALPHA;
		p->sprite.rotation += p->va * dt;

		if(p->age >= p->age_max) {
			p->dead = 1;
		}
	}

	/* Remove dead particles. */
	for(int i=0; i<game.particles_count; i++) {
		if(game.particles[i].dead) {
			for(int n=i+1; n<game.particles_count; n++) {
				game.particles[i] = game.particles[n];
			}
			game.particles_count --;
		}
	}
}

void think(struct graphics *g, float delta_time)
{
	vfs_filewatch();
	game.time = (float) glfwGetTime();
	game_think(delta_time);
	particles_think(delta_time);
	input_think(&game.input, delta_time);
	console_think(&game.console, delta_time);
	shader_uniforms_think(&game.shader, delta_time);
	shader_uniforms_think(&game.bg_shader, delta_time);
}

void render(struct graphics *g, float delta_time)
{
	//glEnableClientState(0);
	glEnableVertexAttribArray(0);

	/* Clear. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Ball. */
	sprite_render(&game.ball.sprite, &game.shader, &game.graphics);

	/* Particles. */
	for(int i=0; i<game.particles_count; i++) {
		if(!game.particles[i].dead) {
			sprite_render(&game.particles[i].sprite, &game.shader, &game.graphics);
		}
	}

	/* Sprites. */
	sprite_render(&game.player1.sprite, &game.shader, &game.graphics);
	sprite_render(&game.player2.sprite, &game.shader, &game.graphics);

	/* Effectslayer */
	sprite_render(&game.effectslayer, &game.bg_shader, &game.graphics);

	/* Text. */
	monotext_render(&game.txt_debug, &game.shader, &game.graphics);
	if(game.console.focused) {
		console_render(&game.console, &game.shader, &game.graphics);
	}
}

void init_effectslayer(struct sprite* b)
{
	b->type = SPRITE_TYPE_UNKNOWN;
	b->texture = &game.textures.none;
	set4f(b->pos, VIEW_WIDTH / 2, VIEW_HEIGHT / 2, 0.5f, 1.0f);
	set4f(b->scale, VIEW_WIDTH, VIEW_HEIGHT, 1.0f, 1.0f);
	copyv(b->color, COLOR_BLACK);
}

void init_player1(struct player *p)
{
	p->sprite.type = SPRITE_TYPE_PLAYER;
	p->sprite.texture = &game.textures.paddle;
	set4f(p->sprite.pos, 32.0f, VIEW_HEIGHT / 2, 0.1f, 1.0f);
	set4f(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
	copyv(p->sprite.color, COLOR_WHITE);
}

void init_player2(struct player *p)
{
	p->sprite.type = SPRITE_TYPE_PLAYER;
	p->sprite.texture = &game.textures.paddle;
	set4f(p->sprite.pos, 608.0f, VIEW_HEIGHT / 2, 0.1f, 1.0f);
	set4f(p->sprite.scale, PLAYER_WIDTH, PLAYER_HEIGHT, 1.0f, 1.0f);
	copyv(p->sprite.color, COLOR_WHITE);
}

void init_ball(struct ball *ball)
{
	ball->sprite.type = SPRITE_TYPE_BALL;
	ball->sprite.texture = &game.textures.test;
	ball->speed = 0.6f;
	rand(); rand(); rand();
	float random_angle = randr(0.0f, 2.0f * M_PI);
	ball->vy = ball->speed * sin(random_angle) / 4.0f;
	ball->vx = ball->speed * cos(random_angle) / 4.0f;
	ball->last_hit_x = 10.0f;
	ball->last_hit_y = 0.0f;

	set4f(ball->sprite.pos, VIEW_WIDTH/2, VIEW_HEIGHT/2, 0.06f, 1.0f);
	set4f(ball->sprite.scale, BALL_WIDTH, BALL_HEIGHT, 1.0f, 1.0f);
	copyv(ball->sprite.color, COLOR_WHITE);
}

static void cmd_quit_func(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	glfwSetWindowShouldClose(game.graphics.window, 1);
}

static void cmd_assets_list_func(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	char *simplename;
	for(int i=0; i<vfs_file_count(); i++) {
		vfs_get_simple_name(&simplename, i);
		if(simplename != NULL) {
			console_printf(c, "%s\n", simplename);
		}
	}
}

static void cmd_assets_reload_func(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	struct char_element *asset = (struct char_element *) list_element_at(argv, 2);
	if(asset == NULL) {
		console_printf(c, "ERROR: No asset specified\n");
	} else {
		console_printf(c, "TODO: Reload asset: \"%s\"\n", asset->data);
	}
}

static void cmd_assets_reload_autocomplete(struct console *c, struct console_cmd *cmd,
		struct list *argv, struct list *matches)
{
	char *simplename;
	for(int i=0; i<vfs_file_count(); i++) {
		vfs_get_simple_name(&simplename, i);
		list_append(matches, simplename);
	}
}

void init_console()
{
	console_new(&game.console, &game.font_console, VIEW_WIDTH, 16, &game.textures.none);

	/* Create commands. */
	console_cmd_new(&game.cmd_quit, "quit", 0, &cmd_quit_func, NULL);
	console_cmd_new(&game.cmd_assets, "assets", 0, NULL, NULL);
	console_cmd_new(&game.cmd_assets_list, "list", 0, &cmd_assets_list_func, NULL);
	console_cmd_new(&game.cmd_assets_reload, "reload", 1, &cmd_assets_reload_func, &cmd_assets_reload_autocomplete);

	/* Register command tree. */
	console_cmd_add(&game.cmd_quit,				&game.console.root_cmd);
	console_cmd_add(&game.cmd_assets,			&game.console.root_cmd);
	console_cmd_add(&game.cmd_assets_reload,	&game.cmd_assets);
	console_cmd_add(&game.cmd_assets_list,		&game.cmd_assets);
}

void init_game()
{
	game.view_width = VIEW_WIDTH;
	game.view_height = VIEW_HEIGHT;
	/* Entities. */
	init_effectslayer(&game.effectslayer);
	init_player1(&game.player1);
	init_player2(&game.player2);
	init_ball(&game.ball);
	monotext_new(&game.txt_debug, "FPS: 0", COLOR_WHITE, &game.font, 16.0f,
			VIEW_HEIGHT - 16.0f);
	init_console();
}

static void char_callback(struct input *input, GLFWwindow *window, char key,
		int mods)
{
	/* Input localized text into console. */
	if(key != CONSOLE_KEY_FOCUS
			&& game.console.focused) {
		console_input_feed_char(&game.console, key, mods);
	}
}

static void key_callback(struct input *input, GLFWwindow *window, int key,
		int scancode, int action, int mods)
{
	/* Input control characters into console. */
	if(key != CONSOLE_KEY_FOCUS
			&& game.console.focused
			&& key >= 256) {
		console_input_feed_control(&game.console, key, scancode, action, mods);
	}

	if(action == GLFW_RELEASE) {
		/* Toggle console. */
		if(key == CONSOLE_KEY_FOCUS) {
			console_toggle_focus(&game.console);
			input->enabled = !game.console.focused;
		}
	} else if(action == GLFW_PRESS) {
		if(!game.console.focused) {
			switch(key) {
				case GLFW_KEY_O:
					game.graphics.delta_time_factor *= 2.0f;
					sound_fx_pitch(&game.vivaldi, game.graphics.delta_time_factor);
					printf("time_mod=%f\n", game.graphics.delta_time_factor);
					break;
				case GLFW_KEY_P:
					game.graphics.delta_time_factor /= 2.0f;
					sound_fx_pitch(&game.vivaldi, game.graphics.delta_time_factor);
					printf("time_mod=%f\n", game.graphics.delta_time_factor);
					break;
				case GLFW_KEY_ESCAPE:
					glfwSetWindowShouldClose(window, 1);
					break;
			}
		}
	}
}

void test_make_sound_manual()
{
	/* 1 s buffer. */
	ALshort buf[SOUND_SAMPLE_RATE] = { 0 };

	/* Add a 440 Hz tone into the buffer. */
	sound_fx_add_filter(buf, 0, SOUND_SAMPLE_RATE, sound_filter_add_440hz);

	/* Lower the volume of half the buffer. */
	sound_fx_add_filter(buf, SOUND_SAMPLE_RATE/2, SOUND_SAMPLE_RATE, sound_filter_half_gain);

	/* Make a sound. */
	if(sound_fx_load_pcm(&game.tone_hit, buf, sizeof(buf)) != SOUND_OK) {
		sound_error("Could not load manual sound\n");
	}
}

void reload_sound(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		sound_debug("Skipped reload of %s (%u bytes)\n", filename, size);
		return;
	}

	struct sound_fx tmp = { 0 };
	struct sound_fx *dst = (struct sound_fx *) userdata;

	/* Reload sound. */
	if(sound_fx_load_vorbis(&tmp, data, size) != SOUND_OK) {
		sound_error("Could not load %s (%u bytes)\n", filename, size);
	} else {
		/* Release current sound (if any). */
		sound_fx_free(dst);

		/* Assign new sound (only if parsing was OK). */
		*(dst) = tmp;

		/* Start playing music. */
		sound_fx_play(dst);
	}
}

void load_sounds()
{
	vfs_register_callback("vivaldi.ogg", &reload_sound, &game.vivaldi);

	if(sound_fx_load_filter(&game.tone_hit,
				0.1 * SOUND_SAMPLE_RATE,
				SOUND_SAMPLE_RATE,
				&sound_filter_add_440hz) != SOUND_OK) {
		sound_error("could not generate tone\n");
	}

	if(sound_fx_load_filter(&game.tone_bounce,
				0.1 * SOUND_SAMPLE_RATE,
				SOUND_SAMPLE_RATE,
				&sound_filter_add_220hz) != SOUND_OK) {
		sound_error("could not generate tone\n");
	}
}

void reload_shader(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		shader_debug("Skipped reload of %s (%u bytes)\n", filename, size);
		return;
	}

	struct shader *dst = (struct shader *) userdata;

	if(!dst) {
		shader_error("Invalid argument to reload_shader()\n");
		return;
	}

	/* Keep references to shader sources. */
	if(strstr(filename, ".frag") != NULL) {
		dst->frag_src = data;
		dst->frag_src_len = size;
	} else if(strstr(filename, ".vert") != NULL) {
		dst->vert_src = data;
		dst->vert_src_len = size;
	} else {
		shader_error("Unknown source name \"%s\" (%u bytes)\n", filename, size);
		return;
	}

	/* Have both sources been loaded? */
	if(dst->vert_src_len == 0) {
		shader_debug("Awaiting vertex shader source...\n");
		return;
	}
	if(dst->frag_src_len == 0) {
		shader_debug("Awaiting fragment shader source...\n");
		return;
	}

	/* Recompile shader. */
	struct shader tmp = *(dst);

	int ret = shader_init(&tmp,
			dst->vert_src, dst->vert_src_len,
			dst->frag_src, dst->frag_src_len);
	if(ret != SHADER_OK) {
		shader_error("Error %d when loading shader %s (%u bytes)\n", ret, filename, size);
	} else {
		/* Delete the old shader. */
		shader_delete(dst);
		/* Assign the new shader only if compilation succeeded. */
		*(dst) = tmp;
		/* Relocate uniforms in the shader, if they changed. */
		shader_uniforms_relocate(dst);
	}
}

void load_shaders()
{
	/* Register asset callbacks */
	vfs_register_callback("basic_shader.frag", reload_shader, &game.shader);
	vfs_register_callback("basic_shader.vert", reload_shader, &game.shader);
	vfs_register_callback("ball_trail.frag", reload_shader, &game.bg_shader);
	vfs_register_callback("ball_trail.vert", reload_shader, &game.bg_shader);

	/* Sprite shader: set up uniforms */
	shader_uniform1f(&game.shader, "time", &game.time);
	shader_uniform1f(&game.shader, "ball_last_hit_x", &game.ball.last_hit_x);
	shader_uniform1f(&game.shader, "ball_last_hit_y", &game.ball.last_hit_y);

	/* Effects shader: set up uniforms */
	shader_uniform1f(&game.bg_shader, "time", &game.time);
	shader_uniform1f(&game.bg_shader, "ball_last_hit_x", &game.ball.last_hit_x);
	shader_uniform1f(&game.bg_shader, "ball_last_hit_y", &game.ball.last_hit_y);
	shader_uniform4f(&game.bg_shader, "ball_pos", &game.ball.sprite.pos);
	shader_uniform1f(&game.bg_shader, "view_width", &game.view_width);
	shader_uniform1f(&game.bg_shader, "view_height", &game.view_height);
}

static void release_shaders()
{
	shader_free(&game.shader);
	shader_free(&game.bg_shader);
}

void reload_atlas(const char *filename, unsigned int size, void *data, void *userdata)
{
	if(size == 0) {
		shader_debug("Skipped reload of %s (%u bytes)\n", filename, size);
		return;
	}

	struct atlas *dst = (struct atlas *) userdata;

	if(!dst) {
		shader_error("Invalid argument to reload_atlas()\n");
		return;
	}

	struct atlas tmp = { 0 };

	int ret = atlas_load(&tmp, data, size);
	if(ret != ATLAS_OK) {
		graphics_error("Error %d when loading atlas %s (%u bytes)\n", ret, filename, size);
	} else {
		/* Delete the old shader. */
		atlas_free(dst);
		/* Assign the new shader only if compilation succeeded. */
		*(dst) = tmp;
		/* DEBUG: Dump debug information about atlas to stdout. */
		atlas_print(dst);
	}
}


void load_atlases()
{
	/* Register asset callbacks */
	vfs_register_callback("earl.json", reload_atlas, &game.atlas_earl);
}

void release_sounds()
{
	sound_fx_free(&game.vivaldi);
}

void release_textures()
{
	texture_free(game.textures.none);
	texture_free(game.textures.test);
}

void release_atlases()
{
	atlas_free(&game.atlas_earl);
}

void reload_textures(const char *filename, unsigned int size, void *data, void* userdata)
{
	if(size == 0) {
		graphics_debug("Skipped reload of texture %s (%u bytes)\n", filename, size);
		return;
	}

	GLuint tmp;

	int ret = texture_load(&tmp, NULL, NULL, data, size);

	if(ret != GRAPHICS_OK) {
		graphics_error("Texture load failed: %s (%u bytes)\n", filename, size);
	} else {
		if(userdata) {
			if(strcmp("paddle.png", filename) == 0) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
			
			(*(GLuint*)userdata) = tmp;
		} else {
			graphics_debug("Unassigned texture: %s (%u bytes)\n", filename, size);
			texture_free(tmp);
		}
	}
}

void load_textures()
{
	/* Create a square white texture for texturing empty sprites with. */
	texture_white(&game.textures.none);

	/* Load textures. */
	vfs_register_callback("test.png", &reload_textures, &game.textures.test);
	vfs_register_callback("paddle.png", &reload_textures, &game.textures.paddle);
}

void test_read_file(const char* filename, unsigned int size, void* data)
{
	printf("%s: ", filename);
	for (unsigned int i = 0; i < size; i++)
	{
		printf("%c", ((char*)data)[i]);
	}
	printf("\n\n");
}

static void load_fonts()
{
	monofont_new(&game.font, "manaspace.png", 16, 16, -7, 0);
	monofont_new(&game.font_console, "04B03_8px.png", 8, 8, -2, 0);
}

static void release_fonts()
{
	monofont_free(&game.font);
	monofont_free(&game.font_console);
}

static void fps_callback(struct frames *f)
{
	monotext_updatef(&game.txt_debug, "FPS:% 5d, MS:% 3.1f/% 3.1f/% 3.1f",
			f->frames, f->frame_time_min, f->frame_time_avg, f->frame_time_max);
}

static void load_assets()
{
	load_shaders();
	load_sounds();
	load_textures();
	load_atlases();
	load_fonts();
}

static void release_assets()
{
	release_shaders();
	release_sounds();
	release_textures();
	release_atlases();
	release_fonts();
}

int main(int argc, char **argv)
{
	/* Start the virtual file system */
	vfs_init(argc, argv);

	int ret = 0;

	/* Seed random number generator. */
	srand(time(NULL));

	/* Set up sound. */
	sound_init(&game.sound);

	/* Set up graphics. */
	int windowed = (argc >= 2 && strncmp(argv[1], "windowed", 8) == 0);
	ret = graphics_init(&game.graphics, &think, &render, &fps_callback,
			VIEW_WIDTH, VIEW_HEIGHT, windowed);
	
	if(ret != GRAPHICS_OK) {
		graphics_error("Graphics initialization failed (%d)\n", ret);
		graphics_free(&game.graphics);
		exit(ret);
	}

	load_assets();

	/* Get input events. */
	ret = input_init(&game.input, game.graphics.window, key_callback,
			char_callback);

	if(ret != GRAPHICS_OK) {
		graphics_error("Input initialization failed (%d)\n", ret);
		graphics_free(&game.graphics);
		exit(ret);
	}

	/* Load all assets */
	vfs_run_callbacks();

	/* Set up game. */
	init_game();

	/* Loop until the user closes the window */
	graphics_loop(&game.graphics);

	release_assets();

	/* Free OpenAL. */
	sound_free(&game.sound);

	/* If we reach here, quit the game. */
	graphics_free(&game.graphics);

	vfs_shutdown();

	return 0;
}
