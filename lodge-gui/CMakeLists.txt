# lodge-gui

add_library(lodge-gui STATIC "")

target_sources(lodge-gui
	PRIVATE
		"src/lodge_gui.c"
		"src/lodge_gui_property_widget_factory.c"
	PUBLIC
		"include/lodge_gui.h"
		"include/lodge_gui_property_widget_factory.h"
)

target_include_directories(lodge-gui
	PRIVATE
		"src/"
	PUBLIC
		"include/"
		"lib/nuklear/"
)

target_link_libraries(lodge-gui
	PRIVATE
		lodge-build-flags
		lodge-gfx
		lodge-assets # for lodge_asset_ref widget
	PUBLIC
		lodge-lib
		lodge-window
)

lodge_set_source_silent_build_flags("lib/nuklear.h")
