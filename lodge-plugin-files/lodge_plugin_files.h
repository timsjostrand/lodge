#ifndef _LODGE_PLUGIN_FILES_H
#define _LODGE_PLUGIN_FILES_H

#include "strview.h"
#include "lodge_plugin.h"

struct lodge_asset_file
{
	strview_t				name;
	char					*data;
	size_t					size;
	bool					vfs_callback;
};
typedef bool				(*lodge_file_filter_func_t)(strview_t filename);

struct lodge_plugin_desc	lodge_plugin_files();

void						lodge_plugin_files_add_file_discovery(struct lodge_assets2 *files, struct lodge_assets2 *populate, lodge_file_filter_func_t filter);

#endif