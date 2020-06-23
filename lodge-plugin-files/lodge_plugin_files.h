#ifndef _LODGE_RES_FILES_H
#define _LODGE_RES_FILES_H

#include "strview.h"
#include "lodge_plugin.h"

struct lodge_res_file
{
	strview_t	name;
	const void	*data;
	size_t		size;
};

struct lodge_plugin lodge_plugin_files();

#endif