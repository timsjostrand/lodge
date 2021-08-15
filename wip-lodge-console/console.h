/**
 * In-game console.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdarg.h>

#include "list.h"
#include "drawable.h"
#include "basic_sprite.h"
#include "monotext.h"
#include "monofont.h"
#include "log.h"

#define console_debug(...) debugf("Console", __VA_ARGS__)
#define console_error(...) errorf("Console", __VA_ARGS__)

#define CONSOLE_CHAR_FOCUS			0x00A7	/* Paragraph key. */
#define CONSOLE_HISTORY_LINES		128	/* These many screen-width lines are guaranteed to fit into the history. */
#define CONSOLE_DISPLAY_LINES		25	/* These many screen-width lines are drawn at any given moment to the screen. */
#define CONSOLE_INPUT_MAX			256	/* Max number of chars user is allowed to input. */
#define CONSOLE_CMD_NAME_MAX		32	/* Max length of a console_cmd name. */
#define CONSOLE_INPUT_HISTORY_MAX	128	/* Number of command input stored in console->input_history. */
#define CONSOLE_COLUMNS_MAX			4 /* Number of columns that can be printed into. */

struct console;
struct console_cmd;

typedef void (*console_cmd_func_t)(struct console *c, struct console_cmd *cmd, struct list *argv);
typedef void (*console_cmd_autocomplete_t)(struct console *c, struct console_cmd *cmd, struct list *argv, struct list *completions);

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct console_cmd {
	char						name[CONSOLE_CMD_NAME_MAX];	/* Name of this command. */
	int							argc;						/* How many (non sub-command) arguments this command takes. */
	console_cmd_func_t			callback;					/* Callback when this command is entered. */
	console_cmd_autocomplete_t	autocomplete;				/* Run to determine how to autocomplete command. */
	struct list					*commands;					/* Subcommands. */
};

define_element(struct cmd_element, struct console_cmd*);

struct console_cursor {
	size_t				pos;				/* Input cursor position. */
	double				time_input;			/* When input was last entered. */
	double				time_blink;			/* When the cursor was last blinked. */
	struct basic_sprite	sprite;				/* Sprite used for rendering the cursor. */
	struct monotext*	txt;				/* Text sprite used for rendering input text. */
};

struct console_conf {
	const char		*data;
	size_t			len;
};

struct console {
	int						initialized;
	struct env				*env;
	uint32_t				padding;
	uint32_t				display_lines;				/* Number of lines to display. */
	uint32_t				focused;					/* Use console_toggle_focus() to set. */
	char					*history;					/* History buffer. */
	char					*history_cur;				/* Current offset in history buffer. */
	size_t					history_len;
	uint32_t				chars_per_line;
	char					input[CONSOLE_INPUT_MAX];
	size_t					input_len;					/* Length of input buffer. */
	struct list				*input_history;				/* The last entered commands are stored here. */
	uint32_t				input_history_cur;			/* The currently selected line in the input history. */
	struct monotext			txt_display;
	struct monotext			txt_input;
	struct monofont			*font;
	struct console_cursor	cursor;
	struct console_cmd		root_cmd;
	struct basic_sprite		background;
	struct console_conf		conf;
};

void console_new_inplace(struct console *c, struct monofont *font, uint32_t view_width,
		uint32_t padding, lodge_texture_t white_tex, lodge_shader_t shader, struct env *env);
void console_free_inplace(struct console *c);

void console_print(struct console *c, const char *text, size_t text_len);
void console_printf(struct console *c, const char *fmt, ...);
void console_vprintf(struct console *c, const char *fmt, va_list args);
void console_update(struct console *c);
void console_render(struct console *c, lodge_shader_t s, const mat4 projection, struct lodge_renderer *renderer, lodge_sampler_t font_sampler);
void console_toggle_focus(struct console *c);
void console_parse_conf(struct console *c, struct console_conf *conf);
float console_height(struct console *c, uint32_t display_lines);

void console_input_feed_control(struct console *c, int key, int scancode, int action, int mods);
void console_input_feed_char(struct console *c, unsigned int key, int mods);
void console_input_clear(struct console *c);

void console_cmd_new(struct console_cmd *cmd, const char *name, int argc, console_cmd_func_t callback, console_cmd_autocomplete_t autocomplete);
void console_cmd_add(struct console_cmd *cmd, struct console_cmd *parent);
void console_cmd_free(struct console_cmd *cmd);
void console_cmd_autocomplete(struct console *c, const char *input, const size_t input_len, const size_t cursor_pos);

void console_parse(struct console *c, const char *in_str, size_t in_str_len);
int  console_cmd_parse_1f(struct console *c, struct console_cmd *cmd, struct list *argv, float *dst);

#endif
