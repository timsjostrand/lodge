#ifndef _LODGE_PLUGIN_FILES_H
#define _LODGE_PLUGIN_FILES_H

#include "strview.h"
#include "lodge_plugin.h"

struct lodge_asset_file
{
	strview_t	name;
	char		*data;
	size_t		size;
	bool		vfs_callback;
};

struct lodge_plugin_desc lodge_plugin_files();

#endif