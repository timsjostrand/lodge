# lodge-entity

add_library(lodge-entity STATIC "")

target_sources(lodge-entity
	PRIVATE
		"lodge_entity_type.c"
		"lodge_component_type.c"
		"lodge_system_type.c"
		"lodge_scene.c"
		"lodge_scene_serialize.c"
		"lodge_transform_component.c"
		"lodge_gui_property_widget_factory.c"
		"lodge_perspective.c"
		"lodge_camera_component.c"
	PUBLIC
		"lodge_entity_type.h"
		"lodge_component_type.h"
		"lodge_system_type.h"
		"lodge_scene.h"
		"lodge_scene_serialize.h"
		"lodge_transform_component.h"
		"lodge_gui_property_widget_factory.h"
		"lodge_perspective.h"
		"lodge_camera_params.h"
		"lodge_camera_component.h"
)

target_include_directories(lodge-entity
	PUBLIC
		"./"
)

target_link_libraries(lodge-entity
	PRIVATE
		lodge-build-flags
		lodge-assets # FIXME(TS): to provide editor widget for lodge_asset_t -- this should probably live in editor plugin
	PUBLIC
		lodge-lib
		lodge-collections
		lodge-serialize-json
		lodge-gui # FIXME(TS): only for `lodge_gui_property_widget_factory` => should probably be separate module
)
