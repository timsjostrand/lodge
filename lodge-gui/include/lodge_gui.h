#ifndef _LODGE_GUI_H
#define _LODGE_GUI_H

#include "math4.h"

struct lodge_window;
typedef struct lodge_window* lodge_window_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_gui;
typedef struct lodge_gui* lodge_gui_t;

struct nk_context;
struct nk_font_atlas;

void				lodge_gui_new_inplace(lodge_gui_t gui, lodge_window_t window, int max_vertex_buffer, int max_element_buffer);
void				lodge_gui_free_inplace(lodge_gui_t gui);
size_t				lodge_gui_sizeof();

lodge_gui_t			lodge_gui_new(lodge_window_t window, int max_vertex_buffer, int max_element_buffer);
void				lodge_gui_free(lodge_gui_t gui);

void				lodge_gui_font_stash_begin(lodge_gui_t gui, struct nk_font_atlas **atlas);
void				lodge_gui_font_stash_end(lodge_gui_t gui);
void				lodge_gui_update(lodge_gui_t gui, float dt);
void				lodge_gui_render(lodge_gui_t gui, lodge_shader_t shader);

void				lodge_gui_char_callback(lodge_window_t win, unsigned int codepoint, int mods, lodge_gui_t gui);
void				lodge_gui_scroll_callback(lodge_window_t win, double xoff, double yoff, lodge_gui_t gui);
void				lodge_gui_mouse_button_callback(lodge_window_t win, int button, int action, int mods, lodge_gui_t gui);

struct nk_context*	lodge_gui_to_ctx(lodge_gui_t gui);

#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_UINT_DRAW_INDEX
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"

struct nk_color		nk_color_from_vec4(const vec4 v);

#endif