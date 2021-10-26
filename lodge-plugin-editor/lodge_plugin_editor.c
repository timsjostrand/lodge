#include "lodge_plugin_editor.h"

#include "dynbuf.h"
#include "membuf.h"

#include "lodge_plugins.h"
#include "lodge_gfx.h"
#include "lodge_gui.h"
#include "lodge_argv.h"
#include "lodge_gui_property_widget_factory.h"
#include "lodge_window.h"
#include "lodge_editor_controller.h"
#include "lodge_editor_selection_system.h"
#include "lodge_assets2.h"
#include "lodge_serialize_json.h"
#include "lodge_system_type.h"
#include "lodge_scene.h"
#include "lodge_framebuffer.h"
#include "color.h"
#include "lodge_input.h"
#include "lodge_keys.h"

#include "lodge_editor_plugins_panel.h"
#include "lodge_editor_viewport_panel.h"
#include "lodge_editor_scenes_panel.h"
#include "lodge_editor_entities_panel.h"
#include "lodge_editor_systems_panel.h"
#include "lodge_editor_properties_panel.h"
#include "lodge_scene_editor.h"

#include "config.h"

struct lodge_editor_panels
{
	size_t										count;
	size_t										capacity;
	struct lodge_editor_panel_desc				*elements;
};

//
// TODO(TS): separate editor-state and editor-app into separate plugins; we should be able to
// run a game and open an editor overlay easily.
//
struct lodge_editor
{
	bool										gui_enabled;
	int											ignore_input_frames;

	struct lodge_plugins						*plugins;
	struct lodge_windows						*windows;
	//struct lodge_scene_renderer_plugin			*scene_renderer;
	struct lodge_assets2						*shaders;
	struct lodge_assets2						*scenes;

	lodge_window_t								window;
	struct lodge_gfx							*gfx;
	lodge_gui_t									gui;

	struct lodge_editor_types					types;
	struct lodge_editor_panels					panels;

	lodge_asset_t								gui_shader_asset;
	lodge_asset_t								scene;

	struct lodge_editor_controller_component	*controller;

	struct lodge_plugins_panel					*plugins_panel;
	struct lodge_viewport_panel					*viewport_panel;
	struct lodge_scene_editor					*scene_editor;
};

static void lodge_editor_render(struct lodge_editor *editor)
{
	//
	// TODO(TS): separate "RUN" mode where scene is rendered directly to default framebuffer w/o any editor GUI
	//
	int window_width = 0;
	int window_height = 0;
	lodge_window_get_size(editor->window, &window_width, &window_height);

	if(window_width <= 0 || window_height <= 0) {
		nk_clear(lodge_gui_to_ctx(editor->gui));
		return;
	}

	lodge_framebuffer_bind(lodge_framebuffer_default());
	lodge_gfx_set_scissor(0, 0, window_width, window_height);

	const int bgs[] = {
		0x928374,
		0x7c6f64,
		0x665c54,
		0x504945,
		0x3c3836,
		0x282828,
	};
	vec4 bg = HEX_TO_RGBA(bgs[3], 1.0f);
	lodge_framebuffer_clear_color(lodge_framebuffer_default(), 0, bg);

	if(editor->gui_enabled) {
		lodge_shader_t gui_shader = lodge_assets2_get(editor->shaders, editor->gui_shader_asset);
		ASSERT_OR(gui_shader) { return; }

		lodge_gfx_annotate_begin(strview("editor_gui"));
		lodge_gui_render(editor->gui, gui_shader);
		lodge_gfx_annotate_end();
	}
}

static vec2 lodge_editor_calc_panels_rect(struct lodge_editor *editor)
{
	vec2 tmp = { 0, 0 };
	for(size_t i = 0, count = editor->panels.count; i < count; i++) {
		struct lodge_editor_panel_desc *panel_desc = &editor->panels.elements[i];
		tmp.x = max(tmp.x, panel_desc->pos.x + panel_desc->size.x);
		tmp.y = max(tmp.y, panel_desc->pos.y + panel_desc->size.y);
	}
	return tmp;
}

static vec2 lodge_editor_get_window_size(struct lodge_editor *editor)
{
	int window_width = 0;
	int window_height = 0;
	lodge_window_get_size(editor->window, &window_width, &window_height);
	return vec2_make((float)window_width, (float)window_height);
}

static void lodge_editor_on_resize(lodge_window_t window, int width, int height, struct lodge_editor *editor)
{
	if(width <= 0 || height <= 0) {
		return;
	}

	const vec2 window_size = vec2_make(width, height);
	const vec2 panels_size = lodge_editor_calc_panels_rect(editor);
	const vec2 panel_factor = vec2_div(window_size, panels_size);

	struct nk_context *ctx = lodge_gui_to_ctx(editor->gui);

	for(size_t i = 0, count = editor->panels.count; i < count; i++) {
		struct lodge_editor_panel_desc *panel_desc = &editor->panels.elements[i];

		struct nk_rect panel_rect = nk_rect(
			panel_desc->pos.x * panel_factor.x,
			panel_desc->pos.y * panel_factor.y,
			panel_desc->size.x * panel_factor.x, 
			panel_desc->size.y * panel_factor.y
		);
	
		nk_window_set_bounds(ctx, panel_desc->name.s, panel_rect);
	}
}

static struct lodge_ret lodge_editor_new_inplace(struct lodge_editor *editor, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	editor->gui_enabled = true;
	editor->scene = NULL;
	editor->controller = NULL;

	//
	// FIXME(TS): static init stuff, this should happen elsewhere
	//
	lodge_gui_property_widget_factory_init();
	//lodge_json_register_func_indices();

	// Plugins
	{
		editor->plugins = plugins;

		void *types_plugin = lodge_plugins_depend(plugins, editor, strview("types"));
		if(!types_plugin) {
			return lodge_error("Failed to find `types` plugin");
		}

		editor->windows = lodge_plugins_depend(plugins, editor, strview("windows"));
		if(!editor->windows) {
			return lodge_error("Failed to find `windows` plugin");
		}

#if 0
		editor->scene_renderer = lodge_plugins_depend(plugins, editor, strview("scene_renderer"));
		if(!editor->scene_renderer) {
			return lodge_error("Failed to find `scene_renderer` plugin");
		}
#endif

		editor->shaders = lodge_plugins_depend(plugins, editor, strview("shaders"));
		if(!editor->shaders) {
			return lodge_error("Failed to find `shaders` plugin");
		}

		editor->scenes = lodge_plugins_depend(plugins, editor, strview("scenes"));
		if(!editor->scenes) {
			return lodge_error("Failed to find `scenes` plugin");
		}
	}

	editor->gui_shader_asset = lodge_assets2_register(editor->shaders, strview("editor/gui"));

	dynbuf_new_inplace(dynbuf(editor->panels), 32);

	// Create window
	{
		const uint32_t width = lodge_argv_get_u32(args, strview("width"), 1920);
		const uint32_t height = lodge_argv_get_u32(args, strview("height"), 1080);
		editor->window = lodge_window_new(editor->windows, "lodge-editor", width, height, LODGE_WINDOW_MODE_WINDOWED);
		if(!editor->window) {
			return lodge_error("Failed to create window");
		}
		lodge_window_set_vsync_enabled(editor->window, 0);
		lodge_window_set_resize_callback(editor->window, lodge_editor_on_resize, editor);
	}

	// GFX -- is this really necessary?
	{
		editor->gfx = lodge_gfx_new();
		struct lodge_ret renderer_ret = lodge_window_set_renderer(editor->window, editor->gfx);
		if(!renderer_ret.success) {
			return renderer_ret;
		}
	}

	// GUI
	{
		editor->gui = lodge_gui_new(editor->window, 32 * 1024 * 1024, 32 * 1024 * 1024);
	}

	editor->types = (struct lodge_editor_types) {
		.selection_system_type = lodge_editor_selection_system_type_register(editor),
		.controller_component_type = lodge_editor_controller_component_type_register(),
		.controller_system_type = lodge_editor_controller_system_type_register(editor),
	};

	//
	// FIXME(TS): register panels through plugins instead
	//
	{
		{
			editor->plugins_panel = calloc(1, lodge_plugins_panel_sizeof());
			lodge_plugins_panel_new_inplace(editor->plugins_panel, plugins);
			
			lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
				.name = strview("Plugins"),
				.panel = editor->plugins_panel,
				.update = &lodge_plugins_panel_update,
				.pos = vec2_make(0, 1080 - 300),
				.size = vec2_make(300, 300),
			});

			lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
				.name = strview("Stats"),
				.panel = editor->plugins_panel,
				.update = &lodge_stats_panel_update,
				.pos = vec2_make(0, 1080 - 300 - 175),
				.size = vec2_make(300, 175),
			});
		}

		{
			editor->viewport_panel = calloc(1, lodge_editor_viewport_panel_sizeof());
			lodge_editor_viewport_panel_new_inplace(editor->viewport_panel, plugins);

			lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
				.name = strview("Viewport"),
				.panel = editor->viewport_panel,
				.update = &lodge_editor_viewport_panel_update,
				.pos = vec2_make(300, 0),
				.size = vec2_make(1920-300-500, 1080),
				.hide_scrollbar = true,
			});
		}

		// FIXME(TS): register through plugins
		{
			editor->scene_editor = calloc(1, lodge_scene_editor_sizeof());
			lodge_scene_editor_new_inplace(editor->scene_editor, editor);
			
			lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
				.name = strview("Scenes"),
				.panel = editor->scene_editor,
				.update = &lodge_editor_scenes_panel_update,
				.pos = vec2_make(0, 0),
				.size = vec2_make(300, 100),
			});

			lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
				.name = strview("Entities"),
				.panel = editor->scene_editor,
				.update = &lodge_editor_entities_panel_update,
				.pos = vec2_make(0, 100),
				.size = vec2_make(300, 1080 - 300 - 100 - 175),
			});

			lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
				.name = strview("Properties"),
				.panel = editor->scene_editor,
				.update = &lodge_editor_properties_panel_update,
				.pos = vec2_make(1920-500, 0),
				.size = vec2_make(500, 540),
			});

			lodge_editor_add_panel(editor, &(struct lodge_editor_panel_desc) {
				.name = strview("Systems"),
				.panel = editor->scene_editor,
				.update = &lodge_editor_systems_panel_update,
				.pos = vec2_make(1920-500, 540),
				.size = vec2_make(500, 540),
			});
		}
	}

	lodge_editor_set_controller(editor, NULL);

	return lodge_success();
}

static void lodge_editor_free_inplace(struct lodge_editor *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	ASSERT_OR(plugin) { return; }

	lodge_gui_free_inplace(plugin->gui);
	lodge_gfx_free(plugin->gfx);
	lodge_window_free(plugin->window);
	dynbuf_free_inplace(dynbuf(plugin->panels));
}

struct lodge_editor_types lodge_editor_get_types(struct lodge_editor *editor)
{
	return editor->types;
}

static void lodge_editor_gui_update(struct lodge_editor *editor, float dt)
{
	lodge_gui_update(editor->gui, dt);

	struct nk_context *ctx = lodge_gui_to_ctx(editor->gui);
	for(size_t i = 0, count = editor->panels.count; i < count; i++) {
		struct lodge_editor_panel_desc *panel_desc = &editor->panels.elements[i];

		ASSERT_NULL_TERMINATED(panel_desc->name);
		struct nk_rect panel_rect = nk_rect(
			panel_desc->pos.x,
			panel_desc->pos.y,
			panel_desc->size.x,
			panel_desc->size.y
		);

		enum nk_panel_flags panel_flags = NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE;
		if(panel_desc->hide_scrollbar) {
			panel_flags |= NK_WINDOW_NO_SCROLLBAR;
		}

		//char node_gui_id[2 + sizeof(lodge_graph_t) + 1] = { 0 };
		//node_gui_id[0] = 'g';
		//node_gui_id[1] = '_';
		//memcpy(node_gui_id+2, (const char*)&graph, sizeof(lodge_graph_t));

		if(nk_begin(ctx, panel_desc->name.s, panel_rect, panel_flags)) {
			//
			// Get dragged & resized bound and sync with desc
			//
			struct nk_rect current_panel_rect = nk_window_get_bounds(ctx);
			panel_desc->pos = vec2_make((float)current_panel_rect.x, (float)current_panel_rect.y);
			panel_desc->size = vec2_make((float)current_panel_rect.w, (float)current_panel_rect.h);

			if(panel_desc->update) {
				panel_desc->update(panel_desc->panel, editor->gui, editor, dt);
			}
		}
		nk_end(ctx);
	}
}

static void lodge_editor_update(struct lodge_editor *editor, float dt)
{
	if(!lodge_window_is_open(editor->window)) {
		lodge_plugins_set_running(editor->plugins, false);
	}

	if(editor->ignore_input_frames > 0) {
		editor->ignore_input_frames--;
		lodge_input_invalidate_mouse_position(lodge_window_get_input(editor->window));
	}

	lodge_editor_gui_update(editor, dt);

	if(editor->scene) {
		lodge_scene_t scene = lodge_assets2_get(editor->scenes, editor->scene);
		if(scene) {
			lodge_scene_update(scene, dt);
		}
	}
}

LODGE_PLUGIN_IMPL(lodge_plugin_editor)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_editor),
		.name = strview("editor"),
		.new_inplace = &lodge_editor_new_inplace,
		.free_inplace = NULL, // &lodge_editor_free_inplace,
		.update = &lodge_editor_update,
		.render = &lodge_editor_render,
		.static_mounts = {
			.count = 1,
			.elements = {
				{
					.src_dir = strview(ASSETS_DIR),
					.dst_point = strview("/editor"),
				}
			}
		},
	};
}

void lodge_editor_add_panel(struct lodge_editor *editor, struct lodge_editor_panel_desc *panel_desc)
{
	struct lodge_editor_panel *tmp = dynbuf_append(dynbuf(editor->panels), panel_desc, sizeof(struct lodge_editor_panel_desc));
	ASSERT(tmp);
}

void lodge_editor_remove_panel(struct lodge_editor *editor, strview_t name)
{
	ASSERT_NOT_IMPLEMENTED();
}

struct lodge_scene* lodge_editor_get_current_scene(struct lodge_editor *editor)
{
	return (editor && editor->scene) ? lodge_assets2_get(editor->scenes, editor->scene) : NULL;
}

lodge_asset_t lodge_editor_get_current_scene_asset(struct lodge_editor *editor)
{
	return editor ? editor->scene : NULL;
}

void lodge_editor_set_current_scene(struct lodge_editor *editor, lodge_asset_t scene_asset)
{
	editor->scene = scene_asset;
}

struct lodge_assets2* lodge_editor_get_scenes(struct lodge_editor *editor)
{
	return editor->scenes;
}

lodge_window_t lodge_editor_get_window(struct lodge_editor *editor)
{
	return editor->window;
}

static void editor_controller_key_callback(lodge_window_t window, int key, int scancode, int action, int mods, struct lodge_editor *editor)
{
	if(key == LODGE_KEY_ESCAPE) {
		lodge_editor_set_controller(editor, NULL);
	}
}

static void editor_controller_mouse_button_callback(lodge_window_t win, int button, int action, int mods, lodge_gui_t gui)
{
}

static void editor_controller_scroll_callback(lodge_window_t win, double xoff, double yoff, lodge_gui_t gui)
{
}

static void lodge_editor_ignore_input_frames(struct lodge_editor *editor, int frames)
{
	editor->ignore_input_frames = frames;
	lodge_input_invalidate_mouse_position(lodge_window_get_input(editor->window));
}

void lodge_editor_set_controller(struct lodge_editor *editor, struct lodge_editor_controller_component *controller)
{
	if(controller != editor->controller) {
		lodge_editor_ignore_input_frames(editor, 5);
	}

	if(!controller) {
		if(editor->controller) {
			editor->controller->input = NULL;
		}

		lodge_window_set_mousebutton_callback(editor->window, &lodge_gui_mouse_button_callback, editor->gui);
		lodge_window_set_scroll_callback(editor->window, &lodge_gui_scroll_callback, editor->gui);
		lodge_window_set_input_char_callback(editor->window, &lodge_gui_char_callback, editor->gui);
		//lodge_window_set_input_callback(game->window, NULL, NULL);

		lodge_window_set_cursor_mode(editor->window, LODGE_CURSOR_MODE_NORMAL);
	} else {
		if(editor->controller) {
			editor->controller->input = NULL;
		}

		editor->controller = controller;
		editor->controller->input = lodge_window_get_input(editor->window);

		lodge_window_set_mousebutton_callback(editor->window, &editor_controller_mouse_button_callback, editor);
		lodge_window_set_scroll_callback(editor->window, &editor_controller_scroll_callback, editor);
		lodge_window_set_input_char_callback(editor->window, NULL, NULL);
		lodge_window_set_input_callback(editor->window, &editor_controller_key_callback, editor);
	
		lodge_window_set_cursor_mode(editor->window, LODGE_CURSOR_MODE_DISABLED);
	}
}