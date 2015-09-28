#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "list.h"
#include "monotext.h"

#ifdef DEBUG
#define console_debug(...) fprintf(stderr, "DEBUG @ Console: " __VA_ARGS__)
#else
#define console_debug(...) do {} while (0)
#endif
#define console_error(...) fprintf(stderr, "ERROR @ Console: " __VA_ARGS__)

#define CONSOLE_KEY_FOCUS		96	/* Paragraph key. */

#define CONSOLE_HISTORY_LINES		128	/* These many screen-width lines are guaranteed to fit into the history. */
#define CONSOLE_DISPLAY_LINES		10	/* These many screen-width lines are drawn at any given moment to the screen. */
#define CONSOLE_INPUT_MAX			256	/* Max number of chars user is allowed to input. */
#define CONSOLE_CMD_NAME_MAX		32	/* Max length of a console_cmd name. */
#define CONSOLE_INPUT_HISTORY_MAX	4	/* Number of command input stored in console->input_history. */

struct console;
struct console_cmd;

typedef void(*console_cmd_func_t)(struct console *c, struct console_cmd *cmd, struct list *argv);
typedef void(*console_cmd_autocomplete_t)(struct console *c, struct console_cmd *cmd, struct list *argv, struct list *completions);

struct console_cmd {
	char						name[CONSOLE_CMD_NAME_MAX];	/* Name of this command. */
	int							argc;						/* How many (non sub-command) arguments this command takes. */
	console_cmd_func_t			callback;					/* Callback when this command is entered. */
	console_cmd_autocomplete_t	autocomplete;				/* Run to determine how to autocomplete command. */
	struct list					*commands;					/* Subcommands. */
};

define_element(struct cmd_element, struct console_cmd*);

struct console_cursor {
	size_t			pos;				/* Input cursor position. */
	double			time_input;			/* When input was last entered. */
	double			time_blink;			/* When the cursor was last blinked. */
	struct sprite	sprite;				/* Sprite used for rendering the cursor. */
	struct monotext *txt;				/* Text sprite used for rendering input text. */
};

struct console {
	int						focused;					/* Use console_toggle_focus() to set. */
	char					*history;					/* History buffer. */
	char					*history_cur;				/* Current offset in history buffer. */
	size_t					history_len;
	int						chars_per_line;
	char					input[CONSOLE_INPUT_MAX];
	size_t					input_len;					/* Length of input buffer. */
	struct list				*input_history;				/* The last entered commands are stored here. */
	int						input_history_cur;			/* The currently selected line in the input history. */
	struct monotext			txt_display;
	struct monotext			txt_input;
	struct monofont			*font;
	struct console_cursor	cursor;
	struct console_cmd		root_cmd;
	struct sprite			background;
};

void console_new(struct console *c, struct monofont *font, int view_width,
		int padding, GLuint *white_tex);
void console_free(struct console *c);
void console_print(struct console *c, const char *text, size_t text_len);
void console_printf(struct console *c, const char *fmt, ...);
void console_think(struct console *c, float delta_time);
void console_render(struct console *c, struct shader *s, struct graphics *g);
void console_toggle_focus(struct console *c);

void console_input_feed_control(struct console *c, int key, int scancode, int action, int mods);
void console_input_feed_char(struct console *c, char key, int mods);
void console_input_clear(struct console *c);

void console_cmd_new(struct console_cmd *cmd, const char *name, int argc,
		console_cmd_func_t callback, console_cmd_autocomplete_t autocomplete);
void console_cmd_add(struct console_cmd *cmd, struct console_cmd *parent);
void console_cmd_free(struct console_cmd *cmd);
void console_cmd_autocomplete(struct console *c, const char *input,
		const size_t input_len, const size_t cursor_pos);

#endif
