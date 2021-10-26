# lodge-plugin-editor

add_library(lodge-plugin-editor STATIC "")

target_sources(lodge-plugin-editor
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_editor.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_controller.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_selection_system.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_plugins_panel.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_viewport_panel.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_scenes_panel.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_entities_panel.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_systems_panel.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_properties_panel.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_scene_editor.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_scene_editor.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_plugin_editor.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_controller.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_selection_system.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_plugins_panel.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_viewport_panel.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_scenes_panel.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_entities_panel.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_systems_panel.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_editor_properties_panel.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_undo.h"
)

target_include_directories(lodge-plugin-editor
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-plugin-editor
	PRIVATE
		lodge-build-flags
		lodge-plugins
	PUBLIC
		lodge-entity
		lodge-plugin-scene-renderer
		lodge-plugin-scenes
)

lodge_add_module_config(lodge-plugin-editor "assets")