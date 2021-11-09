# lodge-entity

add_library(lodge-entity STATIC "")

target_sources(lodge-entity
	PRIVATE
		"src/lodge_entity_type.c"
		"src/lodge_component_type.c"
		"src/lodge_system_type.c"
		"src/lodge_scene.c"
		"src/lodge_scene_serialize.c"
		"src/lodge_transform_component.c"
		"src/lodge_gui_property_widget_factory.c"
		"src/lodge_perspective.c"
		"src/lodge_camera_component.c"
	PUBLIC
		"include/lodge_entity_type.h"
		"include/lodge_component_type.h"
		"include/lodge_system_type.h"
		"include/lodge_scene.h"
		"include/lodge_scene_serialize.h"
		"include/lodge_transform_component.h"
		"include/lodge_gui_property_widget_factory.h"
		"include/lodge_perspective.h"
		"include/lodge_camera_params.h"
		"include/lodge_camera_component.h"
)

target_include_directories(lodge-entity
	PUBLIC
		"include/"
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
