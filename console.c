/**
 * In-game terminal emulator.
 *
 * Author: Tim Sjöstrand <tim.sjostrand@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <GLFW/glfw3.h>

#include "console.h"
#include "color.h"
#include "str.h"
#include "math4.h"
#include "graphics.h"

const vec4 CONSOLE_COLOR_INPUT		= { 1.0f, 1.0f, 1.0f, 1.0f };
const vec4 CONSOLE_COLOR_DISPLAY	= { 0.8f, 0.8f, 0.8f, 1.0f };
const vec4 CONSOLE_COLOR_CURSOR		= { 0.0f, 0.0f, 0.0f, 1.0f };
const vec4 CONSOLE_COLOR_BG			= { 0.0f, 0.0f, 0.0f, 0.4f };

static void console_cursor_init(struct console_cursor *cur, struct monotext *txt, GLuint *white_tex)
{
	cur->txt = txt;
	cur->pos = 0;
	cur->time_input = 0;
	cur->time_blink = 0;

	struct sprite *s = &cur->sprite;
	s->type = 0;
	s->rotation = 0;
	s->texture = white_tex;
	set4f(s->pos, xyz(txt->bottom_left), 0.0f);
	set4f(s->scale,
			txt->font->letter_width  + txt->font->letter_spacing_x + 1,
			txt->font->letter_height + txt->font->letter_spacing_y,
			1, 1);
	set4f(s->color, rgba(CONSOLE_COLOR_CURSOR));
}

static double timer_elapsed(double since)
{
	return now() - since;
}

static void console_cursor_think(struct console_cursor *cur, float dt)
{
	struct sprite *s = &(cur->sprite);

	/* Update cursor X. */
	s->pos[0] = cur->txt->bottom_left[0] + cur->pos * (cur->txt->font->letter_width + cur->txt->font->letter_spacing_x) + cur->txt->font->letter_spacing_x;

	/* Blink cursor. */
	if(timer_elapsed(cur->time_input) <= 700) {
		/* Solid while writing. */
		s->color[3] = 1.0f;
	} else {
		/* Start blinking when idle. */
		if(timer_elapsed(cur->time_blink) <= 250) {
			s->color[3] = 1.0f;
		} else if(timer_elapsed(cur->time_blink) <= 250+400) {
			s->color[3] = 0.0f;
		} else {
			cur->time_blink = now();
		}
	}
}

/**
* @param s			The string to tokenize.
* @param s_size		The size of the buffer holding s.
* @param token		Split strings at this token.
*
* @return			0 on success, -1 on error.
*/
static int console_split_args(const char *s, size_t s_size, const char token, struct list *argv)
{
	if(s_size == 0) {
		return -1;
	}

	for(size_t i=0; i<s_size-1; i++) {
		if(i == 0 || (s[i-1] == token && s[i] != token)) {
			/* Find end token. */
			size_t n;
			for(n = i + 1; n<s_size; n++) {
				/* Could not find end token? */
				if(n >= s_size) {
					return -1;
				}
				if(s[n] == token || s[n] == '\0') {
					break;
				}
			}

			/* Length of this word. */
			int word_len = n - i;
			/* Create string to store parsed argument in. */
			char *name = (char *) malloc(word_len + 1);
			if(name == NULL) {
				console_error("Out of memory");
				return -1;
			}
			memcpy(name, s + i, word_len);
			/* Append NULL terminator. */
			name[word_len] = '\0';
			/* Append parsed arg to argv. */
			list_append(argv, name);

			/* Advance token counter. */
			i += word_len - 1;
		}
		if(s[i] == '\0') {
			break;
		}
	}

	return 0;
}

void console_new(struct console *c, struct monofont *font, int view_width,
		int padding, GLuint *white_tex)
{
	c->font = font;
	c->chars_per_line = view_width / (font->letter_width + font->letter_spacing_x);
	c->history_len = CONSOLE_HISTORY_LINES * c->chars_per_line;
	c->history = (char *) calloc(c->history_len, sizeof(char));
	if(c->history == NULL) {
		console_error("Out of memory\n");
		return;
	}
	c->history_cur = c->history;
	c->input_history = list_new();
	c->input_history_cur = -1;

	float bg_h = padding*2.0f + (CONSOLE_DISPLAY_LINES + 1) * (font->letter_height + font->letter_spacing_y);
	sprite_init(&c->background,
			0,						/* type */
			view_width/2.0f,		/* x */
			bg_h/2.0f,				/* y */
			0.0f,					/* z */
			view_width,				/* h */
			bg_h,
			CONSOLE_COLOR_BG,
			0.0f,
			white_tex);

	monotext_new(&c->txt_input, "", CONSOLE_COLOR_INPUT, font, padding, padding);
	monotext_new(&c->txt_display, "", CONSOLE_COLOR_DISPLAY, font, padding, padding);
	
	console_cursor_init(&c->cursor, &c->txt_input, white_tex);
	console_cmd_new(&c->root_cmd, NULL, 0, NULL, NULL);
}

void console_free(struct console *c)
{
	free(c->history);
	list_free(c->input_history, 1);
	monotext_free(&c->txt_display);
	monotext_free(&c->txt_input);
}

void console_printf(struct console *c, const char *fmt, ...)
{
	/* Sanity check: invalid arguments. */
	if(c == NULL || fmt == NULL) {
		return;
	}

	char tmp[CONSOLE_INPUT_MAX] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf(tmp, fmt, args);
	va_end(args);

	console_print(c, tmp, strnlen(tmp, CONSOLE_INPUT_MAX));
}

void console_print(struct console *c, const char *text, size_t text_len)
{
	/* Sanity check. */
	if(c == NULL || text == NULL) {
		return;
	}

	/* FIXME: implement line splitting. OR: implement wordwrapping in monotext
	 * instead? */
	if(text_len > (size_t) (c->chars_per_line-1)) {
		text_len = c->chars_per_line-1;
	}

	/* Rotate the circular buffer. */
	if((c->history_cur + text_len) >= (c->history + c->history_len)) {
		memmove(c->history, c->history + text_len, c->history_len - text_len);
		c->history_cur -= text_len;
		assert(c->history_cur >= c->history && c->history_cur < c->history + c->history_len);
	}

	/* Push new line. */
	strncpy(c->history_cur, text, text_len);
	c->history_cur += text_len;

	/* Display only subsection of history. */
	char *display = str_search_reverse(c->history, c->history_len, '\n', CONSOLE_DISPLAY_LINES + 1);
	/* Not enough to form a subsection? Display entire history. */
	if(display == NULL) {
		display = c->history;
	}
	size_t display_len = strnlen(display, (c->history + c->history_len) - display);

	/* Update text sprite. */
	monotext_update(&c->txt_display, display, display_len);
}

static void console_render_input(struct console *c, struct shader *s, struct graphics *g)
{
	monotext_update(&c->txt_input, c->input, c->input_len);
	monotext_render(&c->txt_input, s, g);
}

void console_think(struct console *c, float dt)
{
	console_cursor_think(&c->cursor, dt);
}

void console_render(struct console *c, struct shader *s, struct graphics *g)
{
	sprite_render(&c->background, s, g);
	sprite_render(&c->cursor.sprite, s, g);
	monotext_render(&c->txt_display, s, g);
	console_render_input(c, s, g);
}

/**
 * Takes a series of command arguments and returns the corresponding
 * console_cmd struct, if it exists.
 *
 * @param cmd	Where to store the found command.
 * @param c		The console where these commands live.
 * @param argv	A list of splitted strings of the entered commands.
 */
static void console_cmd_get(struct console_cmd **cmd, struct console *c,
		struct list *argv)
{
	int argc = list_count(argv);
	struct console_cmd *cur = &(c->root_cmd);

	for(int i=0; i<argc; i++) {
		struct char_element *e = (struct char_element *) list_element_at(argv, i);

		/* Dead end? */
		if(list_count(cur->commands) == 0) {
			return;
		}

		foreach_list(struct cmd_element*, f, cur->commands) {
			if(strncmp(e->data, f->data->name, strnlen(f->data->name, CONSOLE_CMD_NAME_MAX)) == 0) {
				cur = f->data;
				i += f->data->argc;
				break;
			} else if(f == (struct cmd_element *) list_last(cur->commands)) {
				/* Exhausted subcommands list? */
				return;
			}
		}
	}

	(*cmd) = cur;
}

/**
 * Like console_cmd_get(), returns a console_cmd structure matching a series
 * of command names. However: if the series of command names does not match
 * _exactly_ one command, it will shave off the last command name in order
 * to find the first command that matches.
 *
 * This is useful for retrieving the console_cmd struct responsible for auto-
 * completing the given argument vector.
 *
 * @see console_cmd_get()
 */
static void console_cmd_find(struct console_cmd **cmd, struct console *c,
		struct list *argv)
{
	struct console_cmd *tmp = NULL;
	
	/* Exact match? */
	console_cmd_get(&tmp, c, argv);

	/* Keep shaving the arguments list until an exact match is found. */
	if(tmp == NULL) {
		struct list *sub = list_copy(argv);
		while(!list_empty(sub)) {
			list_element_delete(list_last(sub), 0);
			if(list_empty(sub)) {
				break;
			}
			console_cmd_get(&tmp, c, sub);
			if(tmp != NULL) {
				break;
			}
		}
		list_free(sub, 0);
	}

	if(tmp != NULL) {
		(*cmd) = tmp;
	}
}

static void console_cmd_autocomplete_default(struct console *c, struct console_cmd *cmd,
		struct list *argv, struct list *completions)
{
	foreach_list(struct cmd_element*, e, cmd->commands) {
		list_append(completions, e->data->name);
	}
}

static void console_cmd_func_default(struct console *c, struct console_cmd *cmd,
		struct list *argv)
{
	if(list_count(cmd->commands) > 0) {
		console_printf(c, "Usage: %s [", cmd->name);

		foreach_list(struct cmd_element*, e, cmd->commands) {
			console_printf(c, "%s", e->data->name);

			for(int i=0; i<e->data->argc; i++) {
				console_print(c, " <ARG>", 6);
			}

			if(e != (struct cmd_element *) list_last(cmd->commands)) {
				console_print(c, " | ", 3);
			}
		}

		console_print(c, "]\n", 2);
	}
}

void console_parse(struct console *c, const char *cmd_str, size_t cmd_str_len)
{
	/* Split arguments. */
	struct list *argv = list_new();
	console_split_args(cmd_str, cmd_str_len, ' ', argv);

	/* Search the command tree. */
	struct console_cmd *cmd = NULL;
	console_cmd_get(&cmd, c, argv);

	/* Store to input history. */
	list_prepend(c->input_history, str_copy(cmd_str, CONSOLE_CMD_NAME_MAX));
	if(list_count(c->input_history) > CONSOLE_INPUT_HISTORY_MAX) {
		list_element_delete(list_last(c->input_history), 1);
	}

	/* Store to display history. */
	char tmp[CONSOLE_INPUT_MAX] = { 0 };
	strncpy(tmp, cmd_str, cmd_str_len);
	cmd_str_len += str_insert(tmp, CONSOLE_INPUT_MAX, 0, "# ", 2);
	cmd_str_len += str_insert(tmp, CONSOLE_INPUT_MAX, cmd_str_len, "\n", 1);
	console_print(c, tmp, cmd_str_len);

	if(cmd == NULL || cmd == &(c->root_cmd)) {
		console_print(c, "Command not found\n", 18);
	} else {
		/* Run the command callback. */
		if(cmd->callback != NULL) {
			cmd->callback(c, cmd, argv);
		} else {
			console_debug("TODO: list subcommands due to NULL callback\n");
		}
	}

	/* Free argv list and its elements. */
	list_free(argv, 1);
}

void console_input_clear(struct console *c)
{
	memset(c->input, '\0', CONSOLE_INPUT_MAX);
	c->input_len = 0;
	c->cursor.pos = 0;
	c->input_history_cur = -1;
}

void console_input_feed_char(struct console *c, char key, int mods)
{
	/* Sanity check. */
	if(c == NULL) {
		return;
	}

	c->cursor.time_input = now();

	/* Type char into input if input buffer is not full. */
	if(c->cursor.pos < CONSOLE_INPUT_MAX-1) {
		char typed = (char) key;
		int added = str_insert(c->input, CONSOLE_INPUT_MAX, c->cursor.pos, &typed, 1);
		c->input_len += added;
		c->cursor.pos += added;
	}
}

static void console_input_set(struct console *c, const char *s, const size_t s_len)
{
	strncpy(c->input, s, s_len);
	c->input[s_len] = '\0';
	c->input_len = s_len;
	c->cursor.pos = s_len;
}

static void console_input_history_seek(struct console *c, int index_delta)
{
	int index = imin(imax(c->input_history_cur + index_delta, -1), list_count(c->input_history));
	console_debug("History seek, index=%d\n", index);
	if(index < 0) {
		console_input_clear(c);
	}
	struct char_element *elem = (struct char_element *) list_element_at(c->input_history, index);
	if(elem == NULL) {
		return;
	}
	console_input_set(c, elem->data, strnlen(elem->data, CONSOLE_CMD_NAME_MAX));
	c->input_history_cur = index;
}

void console_input_feed_control(struct console *c, int key, int scancode, int action, int mods)
{
	/* Sanity check. */
	if(c == NULL) {
		return;
	}

	c->cursor.time_input = now();

	if(action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch(key) {
			case GLFW_KEY_ENTER:
				console_parse(c, c->input, c->input_len);
				console_input_clear(c);
				break;
			case GLFW_KEY_UP:
				console_input_history_seek(c, +1);
				break;
			case GLFW_KEY_DOWN:
				console_input_history_seek(c, -1);
				break;
			case GLFW_KEY_LEFT:
				if(mods & GLFW_MOD_CONTROL) {
					/* Move 1 word. */
					/* FIXME: per needle cursor behavior: ie after '=' */
					char *prev_word = str_prev_word(c->input, CONSOLE_INPUT_MAX, c->cursor.pos - 1, " (=");

					if(prev_word == NULL) {
						c->cursor.pos = 0;
					} else {
						int prev_word_offset = (int) (prev_word - c->input);
						c->cursor.pos = imax(prev_word_offset, 0);
					}
				} else {
					/* Move 1 char. */
					c->cursor.pos = imax(c->cursor.pos - 1, 0);
				}
				break;
			case GLFW_KEY_RIGHT:
				if(mods & GLFW_MOD_CONTROL) {
					/* Move 1 word. */
					/* FIXME: per needle cursor behavior: ie after '=' */
					char *next_word = str_next_word(c->input, CONSOLE_INPUT_MAX, c->cursor.pos + 1, " )=");

					if(next_word == NULL) {
						c->cursor.pos = c->input_len;
					} else {
						int next_word_offset = (int) (next_word - c->input);
						c->cursor.pos = imin(next_word_offset, c->input_len);
					}
				} else {
					/* Move 1 char. */
					c->cursor.pos = imin(c->cursor.pos + 1, c->input_len);
				}
				break;
			case GLFW_KEY_BACKSPACE:
				/* Delete char before cursor. */
				if(c->cursor.pos >= 1) {
					int deleted = str_delete(c->input, CONSOLE_INPUT_MAX, c->cursor.pos - 1, 1);
					c->input_len = imax(c->input_len - deleted, 0);
					c->cursor.pos = imax(c->cursor.pos - deleted, 0);
				}
				break;
			case GLFW_KEY_DELETE:
#if 0
				/* NOTE: fix implementation, not perfect when input is empty. */
				/* Delete char under cursor. */
				if(c->input_len > 0) {
					int deleted = str_delete(c->input, CONSOLE_INPUT_MAX, c->cursor.pos, 1);
					c->input_len = imax(c->input_len - deleted, 0);
					c->cursor.pos = imin(c->cursor.pos, imax(c->input_len - 1, 0));
				}
#endif
				break;
			case GLFW_KEY_END:
				c->cursor.pos = c->input_len;
				break;
			case GLFW_KEY_HOME:
				c->cursor.pos = 0;
				break;
			case GLFW_KEY_TAB:
				console_cmd_autocomplete(c, c->input, c->input_len, c->cursor.pos);
				break;
		}
	}
}

void console_toggle_focus(struct console *c)
{
	c->focused = !c->focused;
	c->cursor.time_input = now();
}

void console_cmd_new(struct console_cmd *cmd, const char *name, int argc,
		console_cmd_func_t callback, console_cmd_autocomplete_t autocomplete)
{
	if(name != NULL) {
		strncpy(cmd->name, name, CONSOLE_CMD_NAME_MAX);
	}
	cmd->argc = argc;
	cmd->callback = callback != NULL ? callback : console_cmd_func_default;
	cmd->autocomplete = autocomplete != NULL ? autocomplete : console_cmd_autocomplete_default;
	cmd->commands = list_new();
}

void console_cmd_add(struct console_cmd *cmd, struct console_cmd *parent)
{
	list_append(parent->commands, cmd);
}

void console_cmd_free(struct console_cmd *cmd)
{
	list_free(cmd->commands, 0);
}

/**
 * Filters a list of strings for the string "match" and stores them in the
 * output list of strings.
 */
static void console_filter_list(struct list *out, struct list *in, const char *match, size_t match_size)
{
	foreach_list(struct char_element*, elem, in) {
		if(strncmp(elem->data, match, strnlen(match, match_size)) == 0) {
			list_append(out, elem->data);
		}
	}
}

void console_cmd_autocomplete(struct console *c, const char *input,
		const size_t input_len, const size_t cursor_pos)
{
	/* Split arguments. */
	struct list *argv = list_new();
	console_split_args(input, CONSOLE_INPUT_MAX, ' ', argv);

	/* Find the command that most closely resembles the input. */
	struct console_cmd *cmd = NULL;
	console_cmd_find(&cmd, c, argv);

	if(cmd == NULL) {
		if(list_count(argv) == 1) {
			cmd = &(c->root_cmd);
		} else {
			console_error("cmd == null\n");
			return;
		}
	}
	
	/* Completions are put here. */
	struct list *completions = list_new();

	/* Get command-specific completions. */
	cmd->autocomplete(c, cmd, argv, completions);

	/* Filter completions to match currently typed command. */
	struct char_element *leaf = (struct char_element *) list_last(argv);
	struct list *filtered = list_new();
	console_filter_list(filtered, completions, leaf->data, CONSOLE_CMD_NAME_MAX);

	/* Switch completions with the filtered list. */
	list_free(completions, 0);
	completions = filtered;

	if(list_count(completions) == 1) {
		/* If only one (1) completion is available: automatically insert it
		 * for the user. */
		size_t input_cmd_len = strnlen(leaf->data, CONSOLE_CMD_NAME_MAX);

		/* Insert autocompleted command (guaranteed to exist since
		 * list_count(completions) == 1). */
		char *head = ((struct char_element *) list_first(completions))->data;

		/* Insert completion, set length and update cursor. */
		size_t added = str_replace_into(c->input, CONSOLE_INPUT_MAX, c->cursor.pos - input_cmd_len, head, strnlen(head, CONSOLE_INPUT_MAX));
		c->input_len += added - input_cmd_len;
		c->cursor.pos += added - input_cmd_len;
		/* For convenience, append an extra space. */
		added = str_replace_into(c->input, CONSOLE_INPUT_MAX, c->cursor.pos, " ", 1);
		c->input_len += added;
		c->cursor.pos += added;
	} else {
		/* Print completions */
		foreach_list(struct char_element*, e, completions) {
			console_printf(c, "%s\n", e->data);
		}
	}
	
	list_free(completions, 0);
	list_free(argv, 1);
}
