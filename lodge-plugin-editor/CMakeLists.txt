# lodge-plugin-editor

add_library(lodge-plugin-editor STATIC "")

target_sources(lodge-plugin-editor
	PRIVATE
		"lodge_plugin_editor.c"
		"lodge_editor_controller.c"
		"lodge_editor_selection_system.c"
		"lodge_editor_plugins_panel.c"
		"lodge_editor_viewport_panel.c"
		"lodge_editor_scenes_panel.c"
		"lodge_editor_entities_panel.c"
		"lodge_editor_systems_panel.c"
		"lodge_editor_properties_panel.c"
		"lodge_scene_editor.h"
		"lodge_scene_editor.c"
	PUBLIC
		"lodge_plugin_editor.h"
		"lodge_editor_controller.h"
		"lodge_editor_selection_system.h"
		"lodge_editor_plugins_panel.h"
		"lodge_editor_viewport_panel.h"
		"lodge_editor_scenes_panel.h"
		"lodge_editor_entities_panel.h"
		"lodge_editor_systems_panel.h"
		"lodge_editor_properties_panel.h"
		"lodge_undo.h"
)

target_include_directories(lodge-plugin-editor
	PUBLIC
		"./"
)

target_link_libraries(lodge-plugin-editor
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-entity
		lodge-assets
		lodge-plugins
		lodge-plugin-shaders
		lodge-plugin-scenes
)

lodge_add_module_config(lodge-plugin-editor "assets")

lodge_target_make_plugin(lodge-plugin-editor "lodge_plugin_editor.h" lodge_plugin_editor)
