/**
 * Sets up a typical console.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <GLFW/glfw3.h>

#include "sound.h"
#include "console.h"
#include "graphics.h"
#include "vfs.h"
#include "core_console.h"

struct commands_sound {
	struct console_cmd root;
	struct console_cmd volume;
};

struct commands_graphics {
	struct console_cmd root;
	struct console_cmd dt;
	struct console_cmd quit;
};

struct commands_vfs {
	struct console_cmd root;
	struct console_cmd list;
	struct console_cmd reload;
	struct console_cmd mount;
};

static struct state {
	struct graphics				*graphics;
	struct console				*console;
	struct commands_sound		cmd_sound;
	struct commands_graphics	cmd_graphics;
	struct commands_vfs			cmd_vfs;
} state = { 0 };

void core_console_printf(const char *fmt, ...)
{
	if(state.console == NULL) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	console_vprintf(state.console, fmt, args);
	va_end(args);
}

/* Sound */

static void core_console_sound_volume(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	float f;
	if(console_cmd_parse_1f(c, cmd, argv, &f) != 0) {
		return;
	}
	sound_master_gain(f);
}

static void core_console_sound_free(struct commands_sound *cmd)
{
	console_cmd_free(&cmd->root);
	console_cmd_free(&cmd->volume);
}

static void core_console_sound_init(struct console *c, struct commands_sound *cmd)
{
	/* Create commands. */
	console_cmd_new(&cmd->root, "sound", 0, NULL, NULL);
	console_cmd_new(&cmd->volume, "volume", 1, &core_console_sound_volume, NULL);

	/* Register command tree. */
	console_cmd_add(&cmd->root, &c->root_cmd);
	console_cmd_add(&cmd->volume, &cmd->root);
}

/* Graphics */

static void core_console_graphics_quit(struct console *c, struct console_cmd *cmd,
		struct list *argv)
{
	glfwSetWindowShouldClose(state.graphics->window, 1);
}

static void core_console_graphics_free(struct commands_graphics *cmd)
{
	console_cmd_free(&cmd->root);
	console_cmd_free(&cmd->dt);
	console_cmd_free(&cmd->quit);
}

static void core_console_graphics_init(struct console *c, struct commands_graphics *cmd)
{
	/* Create commands. */
	console_cmd_new(&cmd->quit, "quit", 0, &core_console_graphics_quit, NULL);
	console_cmd_new(&cmd->root, "graphics", 0, NULL, NULL);

	/* Register command tree. */
	console_cmd_add(&cmd->quit,		&c->root_cmd);
	console_cmd_add(&cmd->root,		&c->root_cmd);

	/* Bind variables. */
	console_env_bind_1f(c, "dt", &(state.graphics->delta_time_factor));
}

/* VFS */

static void core_console_vfs_list(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	const char *name;
	for(int i=0; i<vfs_file_count(); i++) {
		name = vfs_get_simple_name(i);
		console_printf(c, "%s\n", name);
	}
}

static void core_console_vfs_reload(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	struct str_element *asset = (struct str_element *) list_element_at(argv, 0);
	if(asset == NULL) {
		console_printf(c, "ERROR: No asset specified\n");
	} else {
		console_printf(c, "TODO: Reload asset: \"%s\"\n", asset->data);
	}
}

static void core_console_vfs_mount(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	struct str_element *e = (struct str_element *) list_element_at(argv, 0);
	if(e == NULL) {
		return;
	}
	console_printf(c, "Mounting \"%s\"...\n", e->data);
	vfs_mount(e->data);
}

static void core_console_vfs_autocomplete_simplename(struct console *c, struct console_cmd *cmd,
		struct list *argv, struct list *matches)
{
	const char *name;
	for(int i=0; i<vfs_file_count(); i++) {
		name = vfs_get_simple_name(i);
		list_append(matches, name);
	}
}

static void core_console_vfs_init(struct console *c, struct commands_vfs *cmd)
{
	console_cmd_new(&cmd->root, "vfs", 0, NULL, NULL);
	console_cmd_new(&cmd->list, "list", 0, &core_console_vfs_list, NULL);
	console_cmd_new(&cmd->reload, "reload", 1, &core_console_vfs_reload, core_console_vfs_autocomplete_simplename);
	console_cmd_new(&cmd->mount, "mount", 1, &core_console_vfs_mount, core_console_vfs_autocomplete_simplename);

	console_cmd_add(&cmd->root,		&c->root_cmd);
	console_cmd_add(&cmd->list,		&cmd->root);
	console_cmd_add(&cmd->reload,	&cmd->root);
	console_cmd_add(&cmd->mount,	&cmd->root);
}

static void core_console_vfs_free(struct commands_vfs *cmd)
{
	console_cmd_free(&cmd->root);
	console_cmd_free(&cmd->list);
	console_cmd_free(&cmd->reload);
	console_cmd_free(&cmd->mount);
}

/* Main API */

void core_console_init(struct graphics *g, struct console *c)
{
	/* Global state. */
	state.graphics = g;
	state.console = c;

	core_console_sound_init(c, &state.cmd_sound);
	core_console_graphics_init(c, &state.cmd_graphics);
	core_console_vfs_init(c, &state.cmd_vfs);
}

void core_console_free()
{
	core_console_sound_free(&state.cmd_sound);
	core_console_graphics_free(&state.cmd_graphics);
	core_console_vfs_free(&state.cmd_vfs);

	console_free(state.console);
}

