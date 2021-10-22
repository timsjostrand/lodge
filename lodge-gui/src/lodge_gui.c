#include "lodge_gui.h"

#include "lodge_platform.h"
#include "lodge_shader.h"
#include "lodge_texture.h"
#include "lodge_drawable.h"
#include "lodge_buffer_object.h"
#include "lodge_pipeline.h"
#include "lodge_shader.h"
#include "lodge_gfx.h"
#include "lodge_sampler.h"

#include "lodge_keys.h"
#include "lodge_window.h"

#define NK_IMPLEMENTATION
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_UINT_DRAW_INDEX
#include "nuklear.h"

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_LO
#define NK_GLFW_DOUBLE_CLICK_LO 2
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_HI
#define NK_GLFW_DOUBLE_CLICK_HI 500
#endif

struct lodge_gui_vertex
{
	vec2							position;
	vec2							uv;
	vec4							col;
};

struct lodge_gui_device
{
	struct nk_buffer				cmds;
	struct nk_draw_null_texture		null;

	lodge_pipeline_t				pipeline;

	lodge_buffer_object_t			vertex_buffer;
	lodge_buffer_object_t			index_buffer;
	lodge_drawable_t				drawable;
	lodge_sampler_t					sampler;

	lodge_texture_t					font_texture;
	int								max_vertex_buffer;
	int								max_element_buffer;

	struct lodge_gui_vertex			*vert_buffer;
	uint32_t						*elem_buffer;
};

struct lodge_gui
{
	lodge_window_t					win;
	int								width;
	int								height;
	int								display_width;
	int								display_height;
	struct lodge_gui_device			dev;
	struct nk_context				ctx;
	struct nk_font_atlas			atlas;
	struct nk_vec2					fb_scale;
	unsigned int					text[NK_GLFW_TEXT_MAX];
	int								text_len;
	struct nk_vec2					scroll;
	lodge_timestamp_t				last_button_click;
	int								is_double_click_down;
	struct nk_vec2					double_click_pos;
	struct nk_vec2					last_click_pos;
};

typedef struct lodge_gui* lodge_gui_t;

static void lodge_gui_device_create(lodge_gui_t gui)
{
	struct lodge_gui_device *dev = &gui->dev;
	nk_buffer_init_default(&dev->cmds);

	{
		struct lodge_pipeline_desc pipeline_desc = lodge_pipeline_desc_make();

		pipeline_desc.blend.enabled = true;
		pipeline_desc.blend.blend_op_rgb = LODGE_BLEND_OP_ADD;
		pipeline_desc.blend.blend_op_alpha = LODGE_BLEND_OP_ADD;
		pipeline_desc.blend.src_factor_rgb = LODGE_BLEND_FACTOR_SRC_ALPHA;
		pipeline_desc.blend.dst_factor_rgb = LODGE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline_desc.depth_stencil.depth_test = false;
		pipeline_desc.depth_stencil.depth_write = false;
		pipeline_desc.rasterizer.cull_mode = LODGE_RASTERIZER_CULL_MODE_NONE;

		dev->pipeline = lodge_pipeline_make(pipeline_desc);
	}

	dev->vert_buffer = malloc(dev->max_vertex_buffer);
	dev->elem_buffer = malloc(dev->max_element_buffer);

	dev->vertex_buffer = lodge_buffer_object_make_dynamic(dev->max_vertex_buffer);
	dev->index_buffer = lodge_buffer_object_make_dynamic(dev->max_element_buffer);

	dev->drawable = lodge_drawable_make((struct lodge_drawable_desc) {
		.indices = dev->index_buffer,
		.attribs_count = 3,
		.attribs = {
			{
				.name = strview_static("Position"),
				.buffer_object = dev->vertex_buffer,
				.float_count = 2,
				.offset = offsetof(struct lodge_gui_vertex, position),
				.stride = sizeof(struct lodge_gui_vertex),
				.instanced = 0,
			},
			{
				.name = strview_static("TexCoord"),
				.buffer_object = dev->vertex_buffer,
				.float_count = 2,
				.offset = offsetof(struct lodge_gui_vertex, uv),
				.stride = sizeof(struct lodge_gui_vertex),
				.instanced = 0,
			},
			{
				.name = strview_static("Color"),
				.buffer_object = dev->vertex_buffer,
				.float_count = 4,
				.offset = offsetof(struct lodge_gui_vertex, col),
				.stride = sizeof(struct lodge_gui_vertex),
				.instanced = 0,
			},
		}
	});

	dev->sampler = lodge_sampler_make((struct lodge_sampler_desc) {
		.min_filter = MIN_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_CLAMP_TO_EDGE,
		.wrap_y = WRAP_CLAMP_TO_EDGE,
		.wrap_z = WRAP_CLAMP_TO_EDGE,
	});
}

static void lodge_gui_device_upload_atlas(lodge_gui_t gui, const void *image, int width, int height)
{
	struct lodge_gui_device *dev = &gui->dev;

	dev->font_texture = lodge_texture_2d_make_from_data(
		&(struct lodge_texture_2d_desc) {
			.width = width,
			.height = height,
			.mipmaps_count = 1,
			.texture_format = LODGE_TEXTURE_FORMAT_RGBA8,
		},
		&(struct lodge_texture_data_desc) {
			.pixel_format = LODGE_PIXEL_FORMAT_RGBA,
			.pixel_type = LODGE_PIXEL_TYPE_UINT8,
			.data = image,
		}
	);
}

static void lodge_gui_device_destroy(lodge_gui_t gui)
{
	struct lodge_gui_device *dev = &gui->dev;

	lodge_texture_reset(dev->font_texture);
	lodge_buffer_object_reset(dev->vertex_buffer);
	lodge_buffer_object_reset(dev->index_buffer);
	lodge_drawable_reset(dev->drawable);
	
	nk_buffer_free(&dev->cmds);
}

void lodge_gui_render(lodge_gui_t gui, lodge_shader_t shader)
{
	struct lodge_gui_device *dev = &gui->dev;

	{
		lodge_window_get_size(gui->win, &gui->width, &gui->height);
		//glfwGetFramebufferSize(win, &lodge_gui.display_width, &lodge_gui.display_height);
		gui->display_width = gui->width;
		gui->display_height = gui->height;

		gui->fb_scale.x = (float)gui->display_width / (float)gui->width;
		gui->fb_scale.y = (float)gui->display_height / (float)gui->height;
	}

	lodge_pipeline_push(dev->pipeline);

	lodge_gfx_bind_shader(shader);

	const mat4 ortho = mat4_ortho(0, gui->width, gui->height, 0, -1.0f, 1.0f);
	lodge_shader_set_constant_mat4(shader, strview_static("ProjMtx"), ortho);

	lodge_gfx_set_viewport(0, 0, gui->display_width, gui->display_height);

	{
		/* convert from command queue into draw list and draw to screen */
		{
			/* Wait until GPU is done with buffer */
			//lodge_gui_wait_for_buffer_unlock();

			{
				/* fill convert configuration */
				struct nk_convert_config config;
				static const struct nk_draw_vertex_layout_element vertex_layout[] = {
					{ NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct lodge_gui_vertex, position) },
					{ NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct lodge_gui_vertex, uv) },
					{ NK_VERTEX_COLOR, NK_FORMAT_R32G32B32A32_FLOAT, NK_OFFSETOF(struct lodge_gui_vertex, col) },
					{ NK_VERTEX_LAYOUT_END }
				};
				NK_MEMSET(&config, 0, sizeof(config));
				config.vertex_layout = vertex_layout;
				config.vertex_size = sizeof(struct lodge_gui_vertex);
				config.vertex_alignment = NK_ALIGNOF(struct lodge_gui_vertex);
				config.null = dev->null;
				config.circle_segment_count = 22;
				config.curve_segment_count = 22;
				config.arc_segment_count = 22;
				config.global_alpha = 1.0f;
				config.shape_AA = NK_ANTI_ALIASING_ON;
				config.line_AA = NK_ANTI_ALIASING_ON;

				/* setup buffers to load vertices and elements */
				struct nk_buffer vbuf, ebuf;
				nk_buffer_init_fixed(&vbuf, dev->vert_buffer, (size_t)dev->max_vertex_buffer);
				nk_buffer_init_fixed(&ebuf, dev->elem_buffer, (size_t)dev->max_element_buffer);
				nk_convert(&gui->ctx, &dev->cmds, &vbuf, &ebuf, &config);

				if(vbuf.allocated > 0) {
					lodge_buffer_object_set(dev->vertex_buffer, 0, dev->vert_buffer, vbuf.allocated);
				}
				if(ebuf.allocated > 0) {
					lodge_buffer_object_set(dev->index_buffer, 0, dev->elem_buffer, ebuf.allocated);
				}
			}
		}

		/* iterate over and execute each draw command */
		const struct nk_draw_command *cmd;
		const nk_draw_index *offset = NULL;
		nk_draw_foreach(cmd, &gui->ctx, &dev->cmds) {
			if(!cmd->elem_count) {
				continue;
			}

			lodge_texture_t tex = cmd->texture.ptr;
			lodge_gfx_bind_texture_unit_2d(0, tex, dev->sampler);

			lodge_gfx_set_scissor(
				(int32_t)(cmd->clip_rect.x * gui->fb_scale.x),
				(int32_t)((gui->height - (int32_t)(cmd->clip_rect.y + cmd->clip_rect.h)) * gui->fb_scale.y),
				(size_t)(cmd->clip_rect.w * gui->fb_scale.x),
				(size_t)(cmd->clip_rect.h * gui->fb_scale.y));

			//glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
			lodge_drawable_render_indexed(dev->drawable, cmd->elem_count, (size_t)offset);

			offset += cmd->elem_count;
		}
		nk_clear(&gui->ctx);
		nk_buffer_clear(&dev->cmds);
	}

	lodge_pipeline_pop();
}

void lodge_gui_char_callback(lodge_window_t win, unsigned int codepoint, int mods, lodge_gui_t gui)
{
	ASSERT(gui);
	if(gui->text_len < NK_GLFW_TEXT_MAX) {
		gui->text[gui->text_len++] = codepoint;
	}
}

void lodge_gui_scroll_callback(lodge_window_t win, double xoff, double yoff, lodge_gui_t gui)
{
	ASSERT(gui);
	gui->scroll.x += (float)xoff;
	gui->scroll.y += (float)yoff;
}

void lodge_gui_mouse_button_callback(lodge_window_t window, int button, int action, int mods, lodge_gui_t gui)
{
	if(button != LODGE_MOUSE_BUTTON_LEFT) {
		return;
	}

	float x, y;
	lodge_window_get_cursor(window, &x, &y);

	const float double_click_pos_tolerance = 5.0f;

	if(action == LODGE_PRESS) {
		const double click_delta_time = lodge_timestamp_elapsed_ms(gui->last_button_click);
		if(click_delta_time >= NK_GLFW_DOUBLE_CLICK_LO && click_delta_time <= NK_GLFW_DOUBLE_CLICK_HI) {
			if(abs(gui->last_click_pos.x - x) < double_click_pos_tolerance && abs(gui->last_click_pos.y - y) < double_click_pos_tolerance) {
				gui->is_double_click_down = nk_true;
				gui->double_click_pos = nk_vec2(x, y);
			}
		}
		gui->last_button_click = lodge_timestamp_get();
	} else {
		gui->is_double_click_down = nk_false;
	}

	gui->last_click_pos = nk_vec2(x, y);
}

static void lodge_gui_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
	lodge_gui_t gui = usr.ptr;

	strview_t str = lodge_window_get_clipboard(gui->win);
	if(str.length) {
		nk_textedit_paste(edit, str.s, str.length);
	}
}

static void lodge_gui_clipboard_copy(nk_handle usr, const char *text, int len)
{
	lodge_gui_t gui = usr.ptr;

	lodge_window_set_clipboard(gui->win, strview_make(text, len));
}

static void lodge_gui_set_style(lodge_gui_t gui)
{
    struct nk_color table[NK_COLOR_COUNT];

	struct nk_color fg = nk_rgba_hex("#ebdbb2ff");
	struct nk_color bg_0 = nk_rgba_hex("#282828ff");
	struct nk_color bg_1 = nk_rgba_hex("#3c3836ff");
	struct nk_color bg_2 = nk_rgba_hex("#504945ff");
	struct nk_color highlight_0 = nk_rgba_hex("#458588ff");
	struct nk_color highlight_1 = nk_rgba_hex("#83a598ff");

	table[NK_COLOR_TEXT]					= fg;
	table[NK_COLOR_WINDOW]					= bg_0;
	table[NK_COLOR_HEADER]					= bg_1;
	table[NK_COLOR_BORDER]					= bg_1;
	table[NK_COLOR_BUTTON]					= bg_2;
	table[NK_COLOR_BUTTON_HOVER]			= highlight_0;
	table[NK_COLOR_BUTTON_ACTIVE]			= highlight_1;
	table[NK_COLOR_TOGGLE]					= highlight_1;
	table[NK_COLOR_TOGGLE_HOVER]			= highlight_0;
	table[NK_COLOR_TOGGLE_CURSOR]			= bg_1;
	table[NK_COLOR_SELECT]					= nk_rgba(57, 67, 61, 255);
	table[NK_COLOR_SELECT_ACTIVE]			= bg_1;
	table[NK_COLOR_SLIDER]					= bg_2;
	table[NK_COLOR_SLIDER_CURSOR]			= nk_rgba(48, 83, 111, 245);
	table[NK_COLOR_SLIDER_CURSOR_HOVER]		= nk_rgba(53, 88, 116, 255);
	table[NK_COLOR_SLIDER_CURSOR_ACTIVE]	= nk_rgba(58, 93, 121, 255);
	table[NK_COLOR_PROPERTY]				= bg_2;
	table[NK_COLOR_EDIT]					= bg_2;
	table[NK_COLOR_EDIT_CURSOR]				= nk_rgba(210, 210, 210, 255);
	table[NK_COLOR_COMBO]					= bg_2;
	table[NK_COLOR_CHART]					= bg_2;
	table[NK_COLOR_CHART_COLOR]				= bg_1;
	table[NK_COLOR_CHART_COLOR_HIGHLIGHT]	= nk_rgba(255, 0, 0, 255);
	table[NK_COLOR_SCROLLBAR]				= bg_2;
	table[NK_COLOR_SCROLLBAR_CURSOR]		= bg_1;
	table[NK_COLOR_SCROLLBAR_CURSOR_HOVER]	= highlight_0;
	table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE]	= highlight_1;
	table[NK_COLOR_TAB_HEADER]				= bg_1;
	nk_style_from_table(&gui->ctx, table);

	gui->ctx.style.window.header.active.data.color = highlight_0;
}

//
// TODO(TS): should actually take an Input abstraction, since that is the only part of the window we are interested in.
//
void lodge_gui_new_inplace(lodge_gui_t gui, lodge_window_t win, int max_vertex_buffer, int max_element_buffer)
{
	gui->win = win;

	nk_init_default(&gui->ctx, 0);
	gui->ctx.clip.copy = lodge_gui_clipboard_copy;
	gui->ctx.clip.paste = lodge_gui_clipboard_paste;
	gui->ctx.clip.userdata = nk_handle_ptr(gui);
	gui->last_button_click = 0;

	{
		struct lodge_gui_device *dev = &gui->dev;
		dev->max_vertex_buffer = max_vertex_buffer;
		dev->max_element_buffer = max_element_buffer;
		lodge_gui_device_create(gui);
	}

	gui->is_double_click_down = nk_false;
	gui->double_click_pos = nk_vec2(0, 0);

	// TODO(TS): support loading fonts from assets
	struct nk_font_atlas *atlas;
	lodge_gui_font_stash_begin(gui, &atlas);
	lodge_gui_font_stash_end(gui);

	lodge_gui_set_style(gui);
}

void lodge_gui_free_inplace(lodge_gui_t gui)
{
	nk_font_atlas_clear(&gui->atlas);
	nk_free(&gui->ctx);
	lodge_gui_device_destroy(gui);
	memset(gui, 0, sizeof(struct lodge_gui));
}

size_t lodge_gui_sizeof()
{
	return sizeof(struct lodge_gui);
}

lodge_gui_t lodge_gui_new(lodge_window_t win, int max_vertex_buffer, int max_element_buffer)
{
	lodge_gui_t gui = calloc(1, lodge_gui_sizeof());
	lodge_gui_new_inplace(gui, win, max_vertex_buffer, max_element_buffer);
	return gui;
}

void lodge_gui_free(lodge_gui_t gui)
{
	lodge_gui_free_inplace(gui);
	free(gui);
}

void lodge_gui_font_stash_begin(lodge_gui_t gui, struct nk_font_atlas **atlas)
{
	nk_font_atlas_init_default(&gui->atlas);
	nk_font_atlas_begin(&gui->atlas);
	*atlas = &gui->atlas;
}

void lodge_gui_font_stash_end(lodge_gui_t gui)
{
	const void *image; int w, h;
	image = nk_font_atlas_bake(&gui->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
	lodge_gui_device_upload_atlas(gui, image, w, h);
	nk_font_atlas_end(&gui->atlas, nk_handle_ptr(gui->dev.font_texture), &gui->dev.null);
	if(gui->atlas.default_font) {
		nk_style_set_font(&gui->ctx, &gui->atlas.default_font->handle);
	}
}

void lodge_gui_update(lodge_gui_t gui, float dt)
{
	struct nk_context *ctx = &gui->ctx;
	lodge_window_t win = gui->win;

	nk_input_begin(ctx);
	for(int i = 0; i < gui->text_len; ++i) {
		nk_input_unicode(ctx, gui->text[i]);
	}

	nk_input_key(ctx, NK_KEY_DEL, lodge_window_key_down(win, LODGE_KEY_DELETE));
	nk_input_key(ctx, NK_KEY_ENTER, lodge_window_key_down(win, LODGE_KEY_ENTER));
	nk_input_key(ctx, NK_KEY_TAB, lodge_window_key_down(win, LODGE_KEY_TAB));
	nk_input_key(ctx, NK_KEY_BACKSPACE, lodge_window_key_down(win, LODGE_KEY_BACKSPACE));
	nk_input_key(ctx, NK_KEY_UP, lodge_window_key_down(win, LODGE_KEY_UP));
	nk_input_key(ctx, NK_KEY_DOWN, lodge_window_key_down(win, LODGE_KEY_DOWN));
	nk_input_key(ctx, NK_KEY_TEXT_START, lodge_window_key_down(win, LODGE_KEY_HOME));
	nk_input_key(ctx, NK_KEY_TEXT_END, lodge_window_key_down(win, LODGE_KEY_END));
	nk_input_key(ctx, NK_KEY_SCROLL_START, lodge_window_key_down(win, LODGE_KEY_HOME));
	nk_input_key(ctx, NK_KEY_SCROLL_END, lodge_window_key_down(win, LODGE_KEY_END));
	nk_input_key(ctx, NK_KEY_SCROLL_DOWN, lodge_window_key_down(win, LODGE_KEY_PAGE_DOWN));
	nk_input_key(ctx, NK_KEY_SCROLL_UP, lodge_window_key_down(win, LODGE_KEY_PAGE_UP));
	nk_input_key(ctx, NK_KEY_SHIFT, lodge_window_key_down(win, LODGE_KEY_LEFT_SHIFT) || lodge_window_key_down(win, LODGE_KEY_RIGHT_SHIFT));

	if(lodge_window_key_down(win, LODGE_KEY_LEFT_CONTROL) || lodge_window_key_down(win, LODGE_KEY_RIGHT_CONTROL)) {
		nk_input_key(ctx, NK_KEY_COPY, lodge_window_key_down(win, LODGE_KEY_C));
		nk_input_key(ctx, NK_KEY_PASTE, lodge_window_key_down(win, LODGE_KEY_V));
		nk_input_key(ctx, NK_KEY_CUT, lodge_window_key_down(win, LODGE_KEY_X));
		nk_input_key(ctx, NK_KEY_TEXT_UNDO, lodge_window_key_down(win, LODGE_KEY_Z));
		nk_input_key(ctx, NK_KEY_TEXT_REDO, lodge_window_key_down(win, LODGE_KEY_R));
		nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, lodge_window_key_down(win, LODGE_KEY_LEFT));
		nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, lodge_window_key_down(win, LODGE_KEY_RIGHT));
		nk_input_key(ctx, NK_KEY_TEXT_LINE_START, lodge_window_key_down(win, LODGE_KEY_B));
		nk_input_key(ctx, NK_KEY_TEXT_LINE_END, lodge_window_key_down(win, LODGE_KEY_E));
	} else {
		nk_input_key(ctx, NK_KEY_LEFT, lodge_window_key_down(win, LODGE_KEY_LEFT));
		nk_input_key(ctx, NK_KEY_RIGHT, lodge_window_key_down(win, LODGE_KEY_RIGHT));
		nk_input_key(ctx, NK_KEY_COPY, 0);
		nk_input_key(ctx, NK_KEY_PASTE, 0);
		nk_input_key(ctx, NK_KEY_CUT, 0);
		nk_input_key(ctx, NK_KEY_SHIFT, 0);
	}

	float x, y;
	lodge_window_get_cursor(win, &x, &y);
	nk_input_motion(ctx, (int)x, (int)y);

	nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, lodge_window_get_mouse_button(win, LODGE_MOUSE_BUTTON_LEFT));
	nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, lodge_window_get_mouse_button(win, LODGE_MOUSE_BUTTON_MIDDLE));
	nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, lodge_window_get_mouse_button(win, LODGE_MOUSE_BUTTON_RIGHT));
	nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)gui->double_click_pos.x, (int)gui->double_click_pos.y, gui->is_double_click_down);
	nk_input_scroll(ctx, gui->scroll);
	nk_input_end(&gui->ctx);
	gui->text_len = 0;
	gui->scroll = nk_vec2(0, 0);

#if 0
	lodge_gui_test(gui);
#endif
}

struct nk_context* lodge_gui_to_ctx(lodge_gui_t gui)
{
	return &gui->ctx;
}

struct nk_color nk_color_from_vec4(const vec4 v)
{
	return (struct nk_color) {
		.r = v.r * 255.0f,
		.g = v.g * 255.0f,
		.b = v.b * 255.0f,
		.a = v.a * 255.0f
	};
}