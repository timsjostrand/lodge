#include "console.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>

#include "color.h"
#include "str.h"
#include "math4.h"
#include "drawable.h"
#include "env.h"
#include "lodge_keys.h"
#include "lodge_time.h"
#include "monofont.h"

static const vec4 CONSOLE_COLOR_INPUT		= { 1.0f, 1.0f, 1.0f, 1.0f };
static const vec4 CONSOLE_COLOR_DISPLAY		= { 0.8f, 0.8f, 0.8f, 1.0f };
static const vec4 CONSOLE_COLOR_CURSOR		= { 0.0f, 0.0f, 0.0f, 1.0f };
static const vec4 CONSOLE_COLOR_BG			= { 0.0f, 0.0f, 0.0f, 0.75f };

static void console_cursor_init(struct console_cursor *cur, struct monotext *txt, lodge_texture_t white_tex)
{
	cur->txt = txt;
	cur->pos = 0;
	cur->time_input = 0;
	cur->time_blink = 0;

	struct basic_sprite *s = &cur->sprite;
	s->type = 0;
	s->rotation = 0;
	s->texture = white_tex;
	s->pos = vec4_make(xyz(txt->bottom_left), 0.0f);
	s->scale = vec4_make((float)(txt->font->letter_width + txt->font->letter_spacing_x + 1),
		(float)(txt->font->letter_height + txt->font->letter_spacing_y),
		1.0f, 1.0f);
	s->color = vec4_make(rgba(CONSOLE_COLOR_CURSOR));
}

static double timer_elapsed(double since)
{
	return lodge_get_time() - since;
}

static void console_cursor_think(struct console_cursor *cur)
{
	struct basic_sprite *s = &(cur->sprite);

	/* Update cursor X. */
	s->pos.x = cur->txt->bottom_left.x + cur->pos * (cur->txt->font->letter_width + cur->txt->font->letter_spacing_x) + cur->txt->font->letter_spacing_x;

	/* Blink cursor. */
	if(timer_elapsed(cur->time_input) <= 700) {
		/* Solid while writing. */
		s->color.a = 1.0f;
	} else {
		/* Start blinking when idle. */
		if(timer_elapsed(cur->time_blink) <= 250) {
			s->color.a = 1.0f;
		} else if(timer_elapsed(cur->time_blink) <= 250+400) {
			s->color.a = 0.0f;
		} else {
			cur->time_blink = lodge_get_time();
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
		if(s[i] == '\0') {
			break;
		}
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
			char *name = (char *) malloc((word_len + 1) * sizeof(char));
			if(name == NULL) {
				console_error("Out of memory");
				return -1;
			}
			memcpy(name, s + i, word_len * sizeof(char));
			/* Append NULL terminator. */
			name[word_len] = '\0';
			/* Append parsed arg to argv. */
			list_append(argv, name);

			/* Advance token counter. */
			i += word_len - 1;
		}
	}

	return 0;
}

float console_height(struct console *c, uint32_t display_lines)
{
	return (c->padding * 2.0f) + (display_lines + 1) * (c->font->letter_height + c->font->letter_spacing_y);
}

void console_new_inplace(struct console *c, struct monofont *font, uint32_t view_width,
		uint32_t padding, lodge_texture_t white_tex, struct shader *shader, struct env *env)
{
	c->env = env;
	c->display_lines = CONSOLE_DISPLAY_LINES;
	c->font = font;
	c->padding = padding;
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

	float bg_h = console_height(c, c->display_lines);
	sprite_init(&c->background,
			0,						/* type */
			view_width/2.0f,		/* x */
			bg_h/2.0f,				/* y */
			0.0f,					/* z */
			(float)view_width,		/* w */
			bg_h,					/* h */
			CONSOLE_COLOR_BG,
			0.0f,
			white_tex);

	monotext_new(&c->txt_input, "", CONSOLE_COLOR_INPUT, font, (float)padding, (float)padding, shader);
	monotext_new(&c->txt_display, "", CONSOLE_COLOR_DISPLAY, font, (float)padding, (float)padding, shader);

	console_cursor_init(&c->cursor, &c->txt_input, white_tex);
	console_cmd_new(&c->root_cmd, "root", 0, NULL, NULL);

	c->initialized = 1;
}

void console_free_inplace(struct console *c)
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

	va_list args;
	va_start(args, fmt);
	console_vprintf(c, fmt, args);
	va_end(args);

}

void console_vprintf(struct console *c, const char *fmt, va_list args)
{
	char tmp[CONSOLE_INPUT_MAX] = { 0 };
	vsprintf(tmp, fmt, args);
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
	char *display = str_search_reverse(c->history, c->history_len - 1, '\n', c->display_lines + 1);
	/* Not enough to form a subsection? Display entire history. */
	if(display == NULL) {
		display = c->history;
	}
	size_t display_len = strnlen(display, (c->history + c->history_len) - display);

	/* Update text sprite. */
	monotext_update(&c->txt_display, display, display_len);
}

static void console_render_input(struct console *c, struct shader *s, lodge_sampler_t font_sampler)
{
	monotext_update(&c->txt_input, c->input, c->input_len);
	monotext_render(&c->txt_input, s, font_sampler);
}

void console_think(struct console *c)
{
	console_cursor_think(&c->cursor);
}

void console_render(struct console *c, struct shader *s, const mat4 projection, struct lodge_renderer *renderer, lodge_sampler_t font_sampler)
{
	sprite_render(&c->background, s, projection, renderer);
	sprite_render(&c->cursor.sprite, s, projection, renderer);
	monotext_render(&c->txt_display, s, font_sampler);
	console_render_input(c, s, font_sampler);
}

/**
 * Takes a series of command arguments and returns the corresponding
 * console_cmd struct, if it exists.
 *
 * @param cmd	Where to store the found command.
 * @param c		The console where these commands live.
 * @param argv	A list of splitted strings of the entered commands.
 */
static void console_cmd_get(struct console_cmd **cmd, int *cmd_index,
		struct console *c, struct list *argv)
{
	int argc = list_count(argv);

	/* Optimization: if no argument vector, always return root cmd. */
	if(argc == 0) {
		if(cmd_index != NULL) {
			(*cmd_index) = -1;
		}
		(*cmd) = &(c->root_cmd);
		return;
	}

	struct console_cmd *cur = &(c->root_cmd);
	int cur_index = -1;

	for(int i=0; i<argc; i++) {
		struct str_element *e = (struct str_element *) list_element_at(argv, i);

		/* Dead end? */
		if(list_count(cur->commands) == 0) {
			goto bail;
		}

		foreach_list(struct cmd_element*, f, cur->commands) {
			size_t e_len = strnlen(e->data, CONSOLE_CMD_NAME_MAX);
			size_t f_len = strnlen(f->data->name, CONSOLE_CMD_NAME_MAX);
			if(strncmp(e->data, f->data->name, e_len > f_len ? e_len : f_len) == 0) {
				cur = f->data;
				if(i + f->data->argc >= argc - 1) {
					cur_index = i;
				}
				i += f->data->argc;
				break;
			} else if(f == (struct cmd_element *) list_last(cur->commands)) {
				/* Exhausted subcommands list? */
				goto bail;
			}
		}
	}

	if(cmd_index != NULL) {
		(*cmd_index) = cur_index;
	}
	(*cmd) = cur;
	return;
bail:
	if(cmd_index != NULL) {
		(*cmd_index) = -100;
	}
	(*cmd) = NULL;
}

static void console_cmd_find(struct console_cmd **cmd, int *cmd_index,
		struct console *c, struct list *argv)
{
	if(list_empty(argv)) {
		(*cmd) = &(c->root_cmd);
		(*cmd_index) = -1;
		return;
	}

	struct list *sub = list_copy(argv);
	(*cmd) = NULL;

	while(!list_empty(sub)) {
		console_cmd_get(cmd, cmd_index, c, sub);
		if((*cmd) != NULL) {
			break;
		}
		list_element_delete(list_last(sub), 0);

		/* Reached end of argument vector: assume root command. */
		if(list_empty(sub)) {
			(*cmd) = &(c->root_cmd);
			(*cmd_index) = -1;
		}
	}

	list_free(sub, 0);
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

static void console_parse_cmd(struct console *c, struct list *argv)
{
	/* Search the command tree. */
	struct console_cmd *cmd = NULL;
	int cmd_index;
	console_cmd_get(&cmd, &cmd_index, c, argv);

	if(cmd == NULL || cmd == &(c->root_cmd)) {
		console_print(c, "Command not found\n", 18);
	} else {
		if(cmd->callback != NULL) {
			int argc = list_count(argv);

			/* Extract arguments specific to this command. */
			struct list *cmd_argv = list_copy_subset(argv, cmd_index + 1, argc - (cmd_index + 1));

			/* Run the command callback. */
			cmd->callback(c, cmd, cmd_argv);

			list_free(cmd_argv, 0);
		} else {
			console_debug("TODO: list subcommands due to NULL callback\n");
		}
	}
}

static void console_env_set(struct console *c, const char *name, const char *value)
{
	// FIXME(TS): `name` arg should already be `strview_t`
	strview_t name_view = strview_make(name, strlen(name));

	struct env_var *var = env_var_get_by_name(c->env, name_view);
	size_t value_len = strlen(value);

	if(var == NULL) {
		console_printf(c, "Unknown variable: \"%s\"\n", name);
		return;
	}

	switch(var->type) {
		case ENV_VAR_TYPE_1F: {
			float f;
			if(str_parse_1f(value, &f) != 0) {
				console_printf(c, "Usage: %s=<FLOAT> (now: %g)\n", name, (*((float *) var->value)));
				return;
			} else {
				env_set_float(c->env, name_view, f);
			}
			break;
		}
		case ENV_VAR_TYPE_2F: {
			vec2 v;
			if(str_parse_2f(value, ',', &v.x, &v.y) != 0) {
				console_printf(c, "Usage: %s=<FLOAT>,<FLOAT> (now: %g,%g)\n", name,
						((vec2 *) var->value)->x,
						((vec2 *) var->value)->y
				);
				return;
			} else {
				env_set_vec2(c->env, name_view, v);
			}
			break;
		}
		case ENV_VAR_TYPE_3F: {
			vec3 v;
			if(str_parse_3f(value, ',', &v.x, &v.y, &v.z) != 0) {
				console_printf(c, "Usage: %s=<F>,<F>,<F> (now: %g,%g,%g)\n", name,
						((vec3 *) var->value)->x,
						((vec3 *) var->value)->y,
						((vec3 *) var->value)->z
				);
				return;
			} else {
				env_set_vec3(c->env, name_view, v);
			}
			break;
		}
		case ENV_VAR_TYPE_BOOL: {
			int b;
			if(str_parse_bool(value, value_len, &b) != 0) {
				console_printf(c, "Usage: %s=<BOOL> (now: %s)\n", name, (*((int *) var->value)) ? "true" : "false");
				return;
			} else {
				env_set_bool(c->env, name_view, b);
			}
			break;
		}
		default:
			console_printf(c, "TODO: Variable type not implemented %d\n", var->type);
			break;
	}
}

static void console_parse_var(struct console *c, struct list *argv)
{
	/* Sanity check. */
	if(list_count(argv) != 1) {
		return;
	}

	struct str_element *input = (struct str_element *) list_first(argv);

	/* Sanity check. */
	if(input == NULL) {
		return;
	}

	size_t input_len = strnlen(input->data, CONSOLE_INPUT_MAX);

	/* Sanity check. */
	if(input_len == 0) {
		return;
	}

	char *input_equal_sign = memchr(input->data, '=', CONSOLE_INPUT_MAX);

	/* Sanity check. */
	if(input_equal_sign == NULL) {
		return;
	}

	size_t name_len = input_equal_sign - input->data;
	size_t value_len = input_len - name_len - 1;

	char name[ENV_VAR_NAME_MAX] = { 0 };
	char value[ENV_VAR_NAME_MAX] = { 0 };

	strncpy(name, input->data, imin(name_len, ENV_VAR_NAME_MAX));
	strncpy(value, input_equal_sign + 1, imin(value_len, ENV_VAR_NAME_MAX));

	console_env_set(c, name, value);
}

static int console_argv_is_var(struct list *argv)
{
	struct str_element *s = (struct str_element *) list_first(argv);
	return list_count(argv) == 1 && (memchr(s->data, '=', strnlen(s->data, CONSOLE_INPUT_MAX)) != NULL);
}

void console_parse(struct console *c, const char *in_str, size_t in_str_len)
{
	/* Store to input history. */
	list_prepend(c->input_history, str_copy(in_str, in_str_len, CONSOLE_INPUT_MAX));
	if(list_count(c->input_history) > CONSOLE_INPUT_HISTORY_MAX) {
		list_element_delete(list_last(c->input_history), 1);
	}

	/* Store to display history. */
	char tmp[CONSOLE_INPUT_MAX] = { 0 };
	strbuf_t strbuf = strbuf_wrap(tmp);
	strbuf_set(strbuf, strview_make(in_str, in_str_len));
	in_str_len += strbuf_insert(strbuf, 0, strview_static("# "));
	in_str_len += strbuf_insert(strbuf, in_str_len, strview_static("\n"));
	console_print(c, tmp, in_str_len);

	/* Split arguments. */
	struct list *argv = list_new();
	console_split_args(in_str, in_str_len, ' ', argv);

	if(console_argv_is_var(argv)) {
		console_parse_var(c, argv);
	} else {
		console_parse_cmd(c, argv);
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

void console_input_feed_char(struct console *c, unsigned int key, int mods)
{
	/* Sanity check. */
	if(c == NULL) {
		return;
	}

	/* Ignore focus key. */
	if(key == CONSOLE_CHAR_FOCUS) {
		return;
	}

	/* Sanity check. */
	if(key >= 127) {
		console_error("Non-ASCII char: %#04x\n", key);
		return;
	}

	c->cursor.time_input = lodge_get_time();

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
	if(index < 0) {
		console_input_clear(c);
	}
	struct str_element *elem = (struct str_element *) list_element_at(c->input_history, index);
	if(elem == NULL) {
		return;
	}
	console_input_set(c, elem->data, strnlen(elem->data, CONSOLE_INPUT_MAX));
	c->input_history_cur = index;
}

void console_input_feed_control(struct console *c, int key, int scancode, int action, int mods)
{
	/* Sanity check. */
	if(c == NULL) {
		return;
	}

	c->cursor.time_input = lodge_get_time();

	if (action == LODGE_PRESS || action == LODGE_REPEAT) {
		switch(key) {
		case LODGE_KEY_ENTER:
				console_parse(c, c->input, c->input_len);
				console_input_clear(c);
				break;
			case LODGE_KEY_UP:
				console_input_history_seek(c, +1);
				break;
			case LODGE_KEY_DOWN:
				console_input_history_seek(c, -1);
				break;
			case LODGE_KEY_LEFT:
				if (mods & LODGE_MOD_CONTROL) {
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
			case LODGE_KEY_RIGHT:
				if(mods & LODGE_MOD_CONTROL) {
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
			case LODGE_KEY_BACKSPACE:
				/* Delete char before cursor. */
				if(c->cursor.pos >= 1) {
					int deleted = str_delete(c->input, CONSOLE_INPUT_MAX, c->cursor.pos - 1, 1);
					c->input_len = imax(c->input_len - deleted, 0);
					c->cursor.pos = imax(c->cursor.pos - deleted, 0);
				}
				break;
			case LODGE_KEY_DELETE:
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
			case LODGE_KEY_END:
				c->cursor.pos = c->input_len;
				break;
			case LODGE_KEY_HOME:
				c->cursor.pos = 0;
				break;
			case LODGE_KEY_TAB:
				console_cmd_autocomplete(c, c->input, c->input_len, c->cursor.pos);
				break;
		}
	}
}

void console_toggle_focus(struct console *c)
{
	c->focused = !c->focused;
	c->cursor.time_input = lodge_get_time();
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
	foreach_list(struct str_element*, elem, in) {
		if(match == NULL || strncmp(elem->data, match, strnlen(match, match_size)) == 0) {
			list_append(out, elem->data);
		}
	}
}

static void console_env_autocomplete(struct env *env, struct list *completions)
{
	for(int i=0; i<env->len; i++) {
		list_append(completions, env->vars[i].name);
	}
}

static size_t console_common_chars(const char *s1, const char *s2, size_t s_max)
{
	size_t same = 0;
	for(size_t i=0; i<s_max && s1[i] != '\0' && s2[i] != '\0'; i++) {
		if(s1[i] == s2[i]) {
			same ++;
		} else {
			break;
		}
	}
	return same;
}

static void console_common_chars_in_list(char *out, size_t out_size, struct list *strings)
{
	if(list_count(strings) <= 0) {
		return;
	}
	struct str_element *first = (struct str_element *) list_first(strings);
	strncpy(out, first->data, strnlen(first->data, out_size));
	foreach_list(struct str_element *, s, strings) {
		size_t same = console_common_chars(out, s->data, out_size);
		same = same > out_size-1 ? out_size-1 : same;
		strncpy(out, s->data, same);
		out[same] = '\0';
	}
}

/**
 * Attempts to fit the specified strings in 'columns' number of columns, and
 * store the width of those columns into the 'widths' array.
 *
 * @return 0 on success, -1 on failure.
 */
static int console_fit_columns(struct console *c, struct list *strings, int columns, int widths[CONSOLE_COLUMNS_MAX])
{
	/* Reset widths table. */
	memset(widths, 0, CONSOLE_COLUMNS_MAX);

	int row_count = (int) ceilf(list_count(strings) / (float) columns);
	for(int row=0; row < row_count; row++) {
		int widths_sum = 0;

		for(int col=0; col < columns; col++) {
			struct str_element *str = (struct str_element *) list_element_at(strings, row + row_count * col);
			int str_len = str == NULL ? 0 : strnlen(str->data, CONSOLE_CMD_NAME_MAX);
			widths[col] = imax(str_len, widths[col]);

			widths_sum += widths[col];

			/* NOTE: subtract one padding char (space) for each column! */
			if(widths_sum >= (c->chars_per_line - columns)) {
				return -1;
			}
		}
	}

	return 0;
}

static void console_print_completions(struct console *c, struct list *completions)
{
	int column_widths[CONSOLE_COLUMNS_MAX] = { 0 };
	int column_count = CONSOLE_COLUMNS_MAX;

	/* Attempt to fit data in 1 to CONSOLE_COLUMNS_MAX columns. */
	for(; column_count >= 0; column_count--) {
		if(console_fit_columns(c, completions, column_count, column_widths) == 0) {
			break;
		}
	}

	/* One "screen" of text. */
	int tmp_size = c->display_lines * c->chars_per_line;
	char *tmp = malloc(16 * 1024);

	/* Now print into those columns. */
	int row_count = (int) ceilf(list_count(completions) / (float) column_count);
	for(int row=0; row < row_count; row++) {
		memset(tmp, '\0', 16 * 1024);
		char *tmp_cur = tmp;

		for(int col=0; col < column_count; col++) {
			struct str_element *str = (struct str_element *) list_element_at(completions, row + row_count * col);

			tmp_cur += sprintf(tmp_cur, "%-*s", column_widths[col], str != NULL ? str->data : "");

			/* Append space or newline. */
			if(col+1 < column_count) {
				tmp_cur += sprintf(tmp_cur, " ");
			}
		}

		tmp_cur += sprintf(tmp_cur, "\n");

		console_print(c, tmp, (tmp_cur - tmp));
	}

	free(tmp);
}

void console_cmd_autocomplete(struct console *c, const char *input,
		const size_t input_len, const size_t cursor_pos)
{
	/* Split arguments. */
	struct list *argv = list_new();
	console_split_args(input, CONSOLE_INPUT_MAX, ' ', argv);

	/* Find the command that most closely resembles the input. */
	struct console_cmd *cmd = NULL;
	int cmd_index = 0;
	console_cmd_find(&cmd, &cmd_index, c, argv);

	if(cmd == NULL) {
		console_debug("Autocomplete: command not found\n");
		return;
	}

	/* Completions are put here. */
	struct list *completions = list_new();

	/* Get command-specific completions. */
	if(cmd->autocomplete != NULL) {
		cmd->autocomplete(c, cmd, argv, completions);
	}

	/* Autocomplete variables? */
	if(cmd_index < 0) {
		console_env_autocomplete(c->env, completions);
	}

	/* Filter completions to match currently typed command. */
	struct str_element *leaf = (struct str_element *) list_element_at(argv, cmd_index + 1);
	size_t leaf_len = 0;
	if(leaf != NULL && leaf->data != NULL) {
		leaf_len = strnlen(leaf->data, CONSOLE_CMD_NAME_MAX);

		struct list *filtered = list_new();
		console_filter_list(filtered, completions, leaf->data, CONSOLE_CMD_NAME_MAX);

		/* Switch completions with the filtered list. */
		list_free(completions, 0);
		completions = filtered;
	}

	if(list_count(completions) == 1) {
		/* If only one (1) completion is available: automatically insert it
		 * for the user. */

		/* Insert autocompleted command (guaranteed to exist since
		 * list_count(completions) == 1). */
		char *head = ((struct str_element *) list_first(completions))->data;

		/* Insert completion, set length and update cursor. */
		size_t added = str_replace_into(c->input, CONSOLE_INPUT_MAX, c->cursor.pos - leaf_len, head, strnlen(head, CONSOLE_INPUT_MAX));
		c->input_len += added - leaf_len;
		c->cursor.pos += added - leaf_len;
		/* Add a convience character based on completed type. */
		if(list_count(argv) == 1 && env_var_get_by_name(c->env, strview_make(head, strlen(head)))) {
			/* Is variable: append equal sign. */
			added = str_replace_into(c->input, CONSOLE_INPUT_MAX, c->cursor.pos, "=", 1);
		} else {
			/* Is command: append space. */
			added = str_replace_into(c->input, CONSOLE_INPUT_MAX, c->cursor.pos, " ", 1);
		}
		c->input_len += added;
		c->cursor.pos += added;
	} else {
		/* Find common characters in autocomplete suggestions. */
		char common[CONSOLE_CMD_NAME_MAX] = { 0 };
		console_common_chars_in_list(common, CONSOLE_CMD_NAME_MAX, completions);
		size_t common_count = strnlen(common, CONSOLE_CMD_NAME_MAX);

		/* Any common characters? Autocomplete them. */
		if(common_count > 0) {
			size_t added = str_replace_into(c->input, CONSOLE_INPUT_MAX, c->cursor.pos - leaf_len, common, common_count);
			c->input_len += added - leaf_len;
			c->cursor.pos += added - leaf_len;
		}

		/* Print completions */
		console_print_completions(c, completions);
	}

	list_free(completions, 0);
	list_free(argv, 1);
}

/**
 * Parse a float value from argv[0] and store it in dst.
 *
 * @return 0 on OK, -1 on error (any error message is printed on the specified
 * console).
 */
int console_cmd_parse_1f(struct console *c, struct console_cmd *cmd,
		struct list *argv, float *dst)
{
	struct str_element *elem = (struct str_element *) list_element_at(argv, 0);
	if(elem == NULL) {
		console_printf(c, "Usage: %s <FLOAT>\n", cmd->name);
		return -1;
	}
	return str_parse_1f(elem->data, dst);
}

void console_parse_conf(struct console *console, struct console_conf *conf)
{
	if(conf->data == NULL) {
		return;
	}

	console->conf = (*conf);

	foreach_line(conf->data, conf->len) {
		/* Empty line or comment? Skip. */
		if(len == 0 || conf->data[start] == '#') {
			continue;
		}

		/* Parse and execute console command. */
		/* FIXME: there is a bug somewhere in the console stack that does not
		 * respect the len argument of console_parse(). Using str_copy manually
		 * here in the mean time to avoid confusion with missing \0. */
		char *line = str_copy(conf->data + start, len, len + 1);
		console_parse(console, line, len);
		free(line);
	}
}
