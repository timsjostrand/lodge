# lodge-entity

add_library(lodge-entity STATIC "")

target_sources(lodge-entity
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/lodge_entity_type.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_component_type.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_system_type.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_entity.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_scene.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_scene_serialize.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_transform_component.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_gui_property_widget_factory.c"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_perspective.c"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/lodge_entity_type.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_component_type.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_system_type.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_entity.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_scene.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_scene_serialize.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_transform_component.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_gui_property_widget_factory.h"
		"${CMAKE_CURRENT_LIST_DIR}/lodge_perspective.h"
)

target_include_directories(lodge-entity
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(lodge-entity
	PRIVATE
		lodge-build-flags
	PUBLIC
		lodge-lib
		lodge-collections
		lodge-serialize-json
		lodge-gui # FIXME(TS): only for `lodge_gui_property_widget_factory` => should probably be separate module
)
