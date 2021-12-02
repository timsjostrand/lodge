# lodge-image

add_library(lodge-image STATIC "")

target_sources(lodge-image
	PRIVATE
		"src/lodge_image.c"
		"src/lodge_image_raw.c"
	PUBLIC
		"include/lodge_image.h"
		"include/lodge_image_raw.h"
)

target_include_directories(lodge-image
	PUBLIC
		"include/"
)

target_link_libraries(lodge-image
	PRIVATE
		lodge-build-flags
		lodge-stb
	PUBLIC
		lodge-lib
		lodge-serialize-json
)
