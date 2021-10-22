function(lodge_target_make_plugin target header_file plugin_func)
	get_target_property(TARGET_BINARY_DIR ${target} BINARY_DIR)

	configure_file(
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lodge-plugin-register.c.in"
		"${TARGET_BINARY_DIR}/${plugin_func}_register.c"
	)

	target_sources(${target}
		INTERFACE
			"${TARGET_BINARY_DIR}/${plugin_func}_register.c"
	)
endfunction()