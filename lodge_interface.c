#include "lodge_interface.h"

#include <string.h>

#include "core.h"
#include "vfs.h"
#include "assets.h"
#include "events.h"
#include "util_reload.h"
#include "util_graphics.h"
#include "events_generated.h"

#define VIEW_WIDTH		640
#define VIEW_HEIGHT		360

// temporary flag to enable building of other projects until lodgei does not
// depend on existing assets
//#define FIXME_BUILD_LODGEI

struct lodgei {
	struct sprite			sprite;
	struct atlas			atlas;
	struct animatedsprites	*batcher;
	struct anim				anim_sprite;
	struct events			events;
} *lodgei = NULL;

void lodgei_init_memory(struct shared_memory *shared_memory, int reload)
{
	if (!reload) {
		memset(shared_memory->game_memory, 0, sizeof(struct lodgei));
	}

	lodgei = (struct lodgei *) shared_memory->game_memory;
	core_global = shared_memory->core;
	assets = shared_memory->assets;
	vfs_global = shared_memory->vfs;
	input_global = shared_memory->input;
}

void lodgei_think(struct graphics *g, float dt)
{
	events_update(&lodgei->events);
	animatedsprites_update(lodgei->batcher, &lodgei->atlas, dt);
#ifdef FIXME_BUILD_LODGEI
	shader_uniforms_think(&assets->shaders.basic_shader, dt);
#endif
}

void lodgei_render(struct graphics *g, float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 transform;
	identity(transform);
#ifdef FIXME_BUILD_LODGEI
	animatedsprites_render(lodgei->batcher, &assets->shaders.basic_shader, g, assets->textures.textures, transform);
#endif
}

void lodgei_mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		float x = 0, y = 0;
		util_view_get_cursor(window, &x, &y, core_global->graphics.projection);

		/* Play sound event */
		vec3 pos = { x, y, 0.0f };
		vec3 vel1 = { randr(0.1f, 10.0f), randr(0.1f, 10.0f), randr(0.1f, 10.0f) };
		vec3 vel2 = { randr(0.1f, 10.0f), randr(0.1f, 10.0f), randr(0.1f, 10.0f) };
		vec3 vel3 = { randr(0.1f, 10.0f), randr(0.1f, 10.0f), randr(0.1f, 10.0f) };

#ifdef FIXME_BUILD_LODGEI
		events_send_sound_play(&lodgei->events, &core_global->sound, assets->sounds.pickup,
			pos, vel1, 0, 1.0f, randr(0.1f, 10.0f), 0);
		events_send_sound_play(&lodgei->events, &core_global->sound, assets->sounds.pickup,
			pos, vel2, 0, 1.0f, randr(0.1f, 10.0f), 0);
		events_send_sound_play(&lodgei->events, &core_global->sound, assets->sounds.pickup,
			pos, vel3, 0, 1.0f, randr(0.1f, 10.0f), 0);
#endif

		set3f(lodgei->sprite.position, xyz(pos));

		console_debug("Click at %.0fx%.0f\n", x, y);
	}
}

void lodgei_console_init(struct console *c, struct env *env)
{
	/* env_bind_1f(env, "print_fps", &(game->print_fps)); */
	env_bind_3f(env, "sprite_pos", &lodgei->sprite.position);
}

void lodgei_init()
{
	events_init(&lodgei->events);
#ifdef FIXME_BUILD_LODGEI
	events_register_generated(&lodgei->events);
#endif

	/* Create animated sprite batcher. */
	lodgei->batcher = animatedsprites_create();

	/* Create sprite animation. */
	animatedsprites_setanim(&lodgei->anim_sprite, 1, 0, 1, 300.0f);

	/* Create sprite. */
	set3f(lodgei->sprite.position, VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f, 0);
	set2f(lodgei->sprite.scale, 1.0f, 1.0f);
	animatedsprites_playanimation(&lodgei->sprite, &lodgei->anim_sprite);
	animatedsprites_add(lodgei->batcher, &lodgei->sprite);
}

void lodgei_key_callback(struct input *input, GLFWwindow *window, int key,
	int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			break;
		}
	}
}

void lodgei_assets_load()
{
	assets_load();
	vfs_register_callback("textures.json", util_reload_atlas, &lodgei->atlas);
}

void lodgei_assets_release()
{
	assets_release();
	atlas_free(&lodgei->atlas);
}

void lodgei_fps_callback(struct frames *f)
{
	core_debug("FPS:% 5d, MS:% 3.1f/% 3.1f/% 3.1f\n", f->frames,
		f->frame_time_min, f->frame_time_avg, f->frame_time_max);
}

void lodgei_start(struct lodge_settings* settings, const char* mount_path, int window_mode)
{
	/* Setup callbacks */
	core_set_think_callback(&lodgei_think);
	core_set_render_callback(&lodgei_render);
	core_set_asset_callbacks(&lodgei_assets_load, &lodgei_init, &lodgei_assets_release);
	core_set_key_callback(&lodgei_key_callback);
	core_set_mousebutton_callback(&lodgei_mousebutton_callback);
	core_set_fps_callback(&lodgei_fps_callback);
	core_set_init_memory_callback(&lodgei_init_memory);
	core_set_console_init_callback(&lodgei_console_init);

	/* Start the virtual file system */
	vfs_init(mount_path);

	printf("listener: %f, %f, %f max: %f\n", xyz(settings->sound_listener), settings->sound_distance_max);

	/* Sound setup */
	core_set_up_sound(&settings->sound_listener, settings->sound_distance_max);

	/* Initialize subsystems and run main loop. */
	core_setup(settings->window_title,
		settings->view_width, settings->view_height,
		settings->window_width, settings->window_height,
		window_mode, 1000000);

	vfs_run_callbacks();

	core_run();

	vfs_shutdown();
}
