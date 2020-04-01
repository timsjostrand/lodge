/**
 * Sets up a typical console.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "core_console.h"
#include "sound.h"
#include "console.h"
#include "vfs.h"
#include "core.h"
#include "env.h"

#include "lodge_window.h"

/**
 * Convenience function to malloc, set up and add a new console command to the
 * command tree.
 */
static struct console_cmd* cmd_new(struct console_cmd *parent, const char *name, int argc,
		console_cmd_func_t callback, console_cmd_autocomplete_t autocomplete)
{
	/* Allocate and init the command. */
	struct console_cmd *cmd = (struct console_cmd *) calloc(1, sizeof(struct console_cmd));
	console_cmd_new(cmd, name, argc, callback, autocomplete);

	/* Register command in parent. */
	if(parent != NULL) {
		console_cmd_add(cmd, parent);
	}

	return cmd;
}

/* Meta */

static void core_console_meta_lines(struct console *c, struct console_cmd *cmd, struct list *argv)
{
	float f;
	if(console_cmd_parse_1f(c, cmd, argv, &f) != 0) {
		return;
	}
	c->display_lines = (int) round(f);

	/* Update height & pos. */
	float bg_h = console_height(c, c->display_lines);
	printf("got %g typed=%g\n", bg_h, f);
	c->background.pos.y = bg_h/2.0f;
	c->background.scale.y = bg_h;

	/* Trim line count. */
	console_print(c, "", 0);
}

static void core_console_meta_init(struct console *c)
{
	struct console_cmd *root = cmd_new(&c->root_cmd, "console", 0, NULL, NULL);
	cmd_new(root, "lines", 1, &core_console_meta_lines, NULL);
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

static void core_console_sound_init(struct console *c)
{
	/* Create commands. */
	struct console_cmd *root = cmd_new(&c->root_cmd, "sound", 0, NULL, NULL);
	cmd_new(root, "volume", 1, &core_console_sound_volume, NULL);
}

/* Graphics */

static void core_console_graphics_quit(struct console *c, struct console_cmd *cmd,
		struct list *argv)
{
#if 1
	ASSERT_NOT_IMPLEMENTED();
#else
	lodge_window_destroy(core_global->graphics.window);
#endif
}

static void core_console_graphics_init(struct console *c, struct env *env)
{
	/* Create commands. */
	cmd_new(&c->root_cmd, "quit", 0, &core_console_graphics_quit, NULL);
	//struct console *cmd root = cmd_new(&c->root_cmd, "graphics", 0, NULL, NULL);

#if 1
	ASSERT_NOT_IMPLEMENTED();
#else
	/* Bind variables. */
	env_bind_1f(env, "dt", &(core_global->graphics.delta_time_factor));
#endif
}

/* VFS */

#if 0

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
		if(vfs_reload_file(asset->data) != VFS_OK) {
			console_printf(c, "ERROR: Could not reload asset: \"%s\"\n", asset->data);
		} else {
			console_printf(c, "Reloaded asset: \"%s\"\n", asset->data);
		}
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

static void core_console_vfs_init(struct console *c)
{
	struct console_cmd *root = cmd_new(&c->root_cmd, "vfs", 0, NULL, NULL);
	cmd_new(root, "list", 0, &core_console_vfs_list, NULL);
	cmd_new(root, "reload", 1, &core_console_vfs_reload, core_console_vfs_autocomplete_simplename);
	cmd_new(root, "mount", 1, &core_console_vfs_mount, core_console_vfs_autocomplete_simplename);
}

#endif

/* Main API */

void core_console_new(struct console *c, struct env *env)
{
	core_console_meta_init(c);
	core_console_sound_init(c);
#if 0
	core_console_graphics_init(c, env);
	core_console_vfs_init(c);
#endif
}

void core_console_printf(const char *fmt, ...)
{
	if(core_global == NULL || !core_global->console.initialized) {
		return;
	}
	va_list args;
	va_start(args, fmt);
	console_vprintf(&core_global->console, fmt, args);
	va_end(args);
}
