#include "lodge_plugin_renderdoc.h"

#include "renderdoc_app.h"

#include "lodge_platform.h"

struct lodge_plugin_renderdoc
{
	void					*lib;
	RENDERDOC_API_1_0_0		*api;
};

void lodge_plugin_renderdoc_free_inplace(struct lodge_plugin_renderdoc *plugin, struct lodge_plugins *plugins)
{
	if(plugin->lib) {
		int ret = lodge_lib_free(plugin->lib);
		ASSERT(ret);
	}
}

struct lodge_ret lodge_plugin_renderdoc_new_inplace(struct lodge_plugin_renderdoc *plugin, struct lodge_plugins *plugins)
{
	strview_t renderdoc_lib_path = strview_static("C:/Program Files/RenderDoc/renderdoc.dll");

	void *lib = lodge_lib_load(renderdoc_lib_path.s);
	ASSERT_OR(lib) {
		return lodge_error("Failed to load renderdoc.dll");
	}
	
	pRENDERDOC_GetAPI RENDERDOC_GetAPI = lodge_lib_get_symbol(lib, "RENDERDOC_GetAPI");
	ASSERT_OR(RENDERDOC_GetAPI) {
		lodge_lib_free(lib);
		return lodge_error("Failed to load RENDERDOC_GetAPI symbol");
	}

	bool ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0, (void**)&plugin->api);
	ASSERT_OR(ret) {
		lodge_lib_free(lib);
		return lodge_error("Failed to load eRENDERDOC_API_Version_1_0_0");
	}

	plugin->lib = lib;

	{
		int major = 0;
		int minor = 0;
		int patch = 0;
		plugin->api->GetAPIVersion(&major, &minor, &patch);
		debugf("Renderdoc", "Loaded v%d.%d.%d\n", major, minor, patch);
	}

	return lodge_success();
}

struct lodge_plugin_desc lodge_plugin_renderdoc()
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_renderdoc),
		.name = strview_static("renderdoc"),
		.new_inplace = &lodge_plugin_renderdoc_new_inplace,
		.free_inplace = &lodge_plugin_renderdoc_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}

void lodge_plugin_renderdoc_start_frame_capture(struct lodge_plugin_renderdoc *plugin)
{
	plugin->api->StartFrameCapture(NULL, NULL);
}

bool lodge_plugin_renderdoc_stop_frame_capture(struct lodge_plugin_renderdoc *plugin)
{
	return plugin->api->EndFrameCapture(NULL, NULL);
}
