#ifndef _LODGE_RES_FILES_H
#define _LODGE_RES_FILES_H

#include "strview.h"
#include "lodge_plugin.h"

struct lodge_res_file
{
	strview_t	name;
	char		*data;
	size_t		size;
};

struct lodge_plugin_desc lodge_plugin_files();

#endif