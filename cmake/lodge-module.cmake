
function(lodge_add_module_config target assets_dir)
	get_target_property(TARGET_SOURCE_DIR ${target} SOURCE_DIR)
	get_target_property(TARGET_BINARY_DIR ${target} BINARY_DIR)

	configure_file(
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lodge-module-config.h.in"
		"${TARGET_BINARY_DIR}/config.h"
	)

	target_include_directories(${target}
		PRIVATE
			"${TARGET_BINARY_DIR}"
	)
endfunction()