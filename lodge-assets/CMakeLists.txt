# lodge-assets
#
# Framework for hotswappable resource manager with dependency graph.
#

add_library(lodge-assets STATIC "")

target_sources(lodge-assets
	PRIVATE
		"src/lodge_assets2.c"
		"src/lodge_type_asset.c"
	PUBLIC
		"include/lodge_assets2.h"
		"include/lodge_type_asset.h"
)

target_include_directories(lodge-assets
	PUBLIC
		"include/"
)

target_link_libraries(lodge-assets
	PRIVATE
		lodge-build-flags
		lodge-lib
		lodge-collections
		lodge-plugins
)
